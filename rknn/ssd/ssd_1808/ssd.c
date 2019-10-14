#include <stdio.h>
#include <rga/RgaApi.h>
#include <linux/videodev2.h>
#include "buffer.h"
#include "rknn_msg.h"
#include "yuv.h"
#include "ssd.h"
// #include "ssd_post.h"
#include "v4l2camera.h"
#include "device_name.h"

#ifdef __linux__
#include <sys/resource.h>
#include <sys/time.h>
#endif

#include "FRLibrary.h"
#include "FRLibraryTypes.h"

#define MODEL_NAME    "/usr/share/rknn_demo/ssd_inception_v2.rknn"

#define SRC_W         640
#define SRC_H         480
#define SRC_FPS       30
#define SRC_BPP       24
//#define DST_W         240
//#define DST_H         480
#define DST_W         640
#define DST_H         480
#define DST_BPP       24

#define SRC_V4L2_FMT  V4L2_PIX_FMT_YUV420
//#define SRC_V4L2_FMT  V4L2_PIX_FMT_NV21
#define SRC_RKRGA_FMT RK_FORMAT_YCbCr_420_P
//#define SRC_RKRGA_FMT RK_FORMAT_YCbCr_420_SP

#define DST_RKRGA_FMT RK_FORMAT_RGB_888

void* hFR = NULL;

float g_fps;
bo_t g_rga_buf_bo;
int g_rga_buf_fd;
char *g_mem_buf;
struct ssd_group g_ssd_group[2];
volatile int send_count = 0;
volatile int recv_count = 0;
char *dev_name;

unsigned long recordID = -1;

extern int yuv_draw(char *src_ptr, int src_fd, int format,
                    int src_w, int src_h);
extern int yuv_draw_beiqi(char *src_ptr, int src_fd, int format,
                    int src_w, int src_h);

extern void ssd_paint_object_msg();
extern void ssd_paint_fps_msg();

pthread_mutex_t group_mutex;

// start time
struct timeval start;

int continue_camera_capture = 1;


/**
 * Returns the current resident set size (physical memory use) measured
 * in bytes, or zero if the value cannot be determined on this OS.
 */
size_t getCurrentRSSc( )
{
#if defined(_WIN32)
    /* Windows -------------------------------------------------- */
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
    return (size_t)info.WorkingSetSize;

#elif defined(__APPLE__) && defined(__MACH__)
    /* OSX ------------------------------------------------------ */
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if ( task_info( mach_task_self( ), MACH_TASK_BASIC_INFO,
                    (task_info_t)&info, &infoCount ) != KERN_SUCCESS )
        return (size_t)0L;        /* Can't access? */
    return (size_t)info.resident_size;

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    /* Linux ---------------------------------------------------- */
    long rss = 0L;
    FILE* fp = NULL;
    if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
        return (size_t)0L;        /* Can't open? */
    if ( fscanf( fp, "%*s%ld", &rss ) != 1 )
    {
        fclose( fp );
        return (size_t)0L;        /* Can't read? */
    }
    fclose( fp );
    return ((size_t)rss * (size_t)sysconf( _SC_PAGESIZE)) / 1024;

#else
    /* AIX, BSD, Solaris, and Unknown OS ------------------------ */
    return (size_t)0L;            /* Unsupported. */
#endif
}

void checkMemoryc()
{
#ifdef __linux__
    struct rusage r_usage;
    getrusage(RUSAGE_SELF,&r_usage);
    printf("Memory usage Max: %ld kb, curr %ld kb\n",r_usage.ru_maxrss, getCurrentRSSc( ));
#endif
}

static int cur_group = 0;
inline struct ssd_group* ssd_get_ssd_group()
{

    return &g_ssd_group[cur_group];
}

int ssd_group_mutex_init()
{
    pthread_mutex_init(&group_mutex, NULL);
}

int ssd_group_mutex_deinit()
{
    pthread_mutex_destroy(&group_mutex);
}

int ssd_group_mutex_lock()
{
    pthread_mutex_lock(&group_mutex);
}

int ssd_group_mutex_unlock()
{
    pthread_mutex_unlock(&group_mutex);
}

inline float ssd_get_fps()
{
    return g_fps;
}

#define CHECK_STATUS(func) do { \
    status = func; \
    if (status < 0)  { \
    goto final; \
    }   \
    } while(0)

long cal_fps(float *cal_fps)
{
    static int count = 0;
    static float fps = 0;
    static long begin_time = 0;
    static long end_time = 0;

    count++;
    if (begin_time == 0)
        begin_time = getCurrentTime();
    if (count >= 10) {
        end_time = getCurrentTime();
        fps = (float)10000 / (float)(end_time - begin_time);
        begin_time = end_time;
        count = 0;
        *cal_fps = fps;
    }
    ssd_paint_fps_msg();
}

int ssd_rknn_process(char* in_data, int w, int h)
{
    int width = w;
    int height = h;
    int step = w;

    for(int m=0; m<1; m++)
    {
        FRIDList pResult;
        int frstatus = 0;

        printf("Before recog: "); checkMemoryc();
        struct timeval start, end;
        gettimeofday(&start, NULL);

	printf("FR input image %dx%d\n", width, height);
        /* Call to recognize face, to be called repeatedly on image stream coming from camera.*/
        int rval;
        if (spoof_check) {
            rval = wfFR_RecognizeFacesSingleCamSpoof(hFR, in_data, width, height, step, &pResult, &frstatus);
        }
        else {
            rval =  wfFR_RecognizeFaces(hFR, in_data, width, height, step, &pResult, &frstatus);
            frstatus = 1;
        }

        if (rval != FR_OK)
        {
            printf("Error in wfFR_Recognizing Faces %d\n", rval);
            break;
        }

        gettimeofday(&end, NULL);
        float totalTime = (end.tv_sec  - start.tv_sec)*1000.0f + (end.tv_usec - start.tv_usec)/1000.0f;

        printf("After recog: "); checkMemoryc();

        printf("Num results %d, Total time %.1f\n", pResult.nResults, totalTime);
        if (spoof_check)
            printf("Spoof Status :  %d\n", frstatus);

        int ret = rknn_msg_send(&pResult, frstatus);
        if (ret)
            return -1;
        while(send_count - recv_count >= 5) {
            printf("sleep now \n");
            usleep(2000);
        }
        send_count++;

        // prints number of faces recognized
        for(int i=0;i<pResult.nResults;i++)
        {
            //            exit(0);
            /* If pConfidence > 0 then face is recognized */
            printf("Frame %d: face %d, Conf %.6f, size %d, ID %d %d\n",m, i,
                   pResult.pConfidence[i], pResult.pFace[i].width, (int)pResult.pRecordID[i],pResult.pTrackID[i]);
        }
        printf("\n\n");

    }


    // long postprocessTime2 = getCurrentTime();
    // printf("post process time:%0.2ldms\n", postprocessTime2 - postprocessTime1);
}

int wfFR_recognise(char* in_data, int w, int h) {
    int width = w;
    int height = h;
    int step = w;

    for(int m=0; m<1; m++)
    {


        FRIDList pResult;
        int frstatus = 0;

        printf("Before enroll: "); checkMemoryc();
        /* Call to enroll face, to be called repeatedly on image stream coming from camera.*/

        int rval =  wfFR_EnrollFace(hFR, in_data, width, height, step, recordID, 0, &pResult, &frstatus);

        printf("After enroll: "); checkMemoryc();

        printf("Num results %d\n", pResult.nResults);	// prints number of faces

        int ret = rknn_msg_send(&pResult, 1); // wfFr currently doesnot support spoof check on enroll. so passing 1 as spoof_status to ipc_msg
        if (ret)
            return -1;
        while(send_count - recv_count >= 5) {
            printf("sleep now \n");
            usleep(2000);
        }
        send_count++;

        for(int i=0;i<pResult.nResults;i++)
        {
            /* If pConfidence = 0 then the current face image is enrolled */
            printf("Frame %d: face %d, Conf %.1f, size %d\n",m, i,
                   pResult.pConfidence[i], pResult.pFace[i].width);
        }
        printf("\n\n");
    }
    // End time
    struct timeval end;
    gettimeofday(&end, NULL);

    float totalTime = (end.tv_sec  - start.tv_sec)*1000.0f + (end.tv_usec - start.tv_usec)/1000.0f;
    printf("Time : %.6f \n", totalTime);

    if (totalTime > 10000.0f) {
        continue_camera_capture = 0;
        printf("Release FR handle\n");
        /* Release FR handle */
        wfFR_Release(&hFR);
        printf("After release: "); checkMemoryc();
    }
}
// int frameeeee = 0;
void ssd_camera_callback(void *p, int w, int h)
{
    unsigned char* srcbuf = (unsigned char *)p;

//    YUV420toRGB24_RGA2(SRC_RKRGA_FMT, srcbuf, w, h,
//                      SRC_RKRGA_FMT, g_rga_buf_fd, DST_W, DST_H);
//    memcpy(g_mem_buf, g_rga_buf_bo.ptr, 240 * 480 * DST_BPP / 8);
    // Send camera data to minigui layer
    yuv_draw_beiqi(srcbuf, 0, SRC_RKRGA_FMT, w ,h);

    struct timeval start, end;
    gettimeofday(&start, NULL);

//    YUV420toRGB24_RGA(SRC_RKRGA_FMT, srcbuf, w, h,
//                      DST_RKRGA_FMT, g_rga_buf_fd, DST_W, DST_H);

//     char txt[256];
// 
//     sprintf(txt, "./data/image%d.bin", frameeeee);
//     frameeeee++;
//    FILE* fp = fopen(txt,"wb");

//    if(fp!=NULL) {
//        printf("Wrinting to file");
//        fwrite(srcbuf, 1,w*h*1.5,fp);
//        fclose(fp);
//    }

//    memcpy(g_mem_buf, g_rga_buf_bo.ptr, DST_W * DST_H * DST_BPP / 8);


//    FILE* fp1 = fopen("imgrgb.dat","wb");

//    if(fp1!=NULL) {
//        printf("Wrinting to file");
//        fwrite(g_mem_buf, 1,w*h*3,fp1);
//        fclose(fp1);
//    }

    gettimeofday(&end, NULL);
    float totalTime = (end.tv_sec  - start.tv_sec)*1000.0f + (end.tv_usec - start.tv_usec)/1000.0f;
    printf("Conversion Time : %.6f \n", totalTime);
    //    memcpy(g_mem_buf, g_rga_buf_bo.ptr, DST_W * DST_H * DST_BPP / 8);
    //    ssd_rknn_process(g_mem_buf, DST_W, DST_H, DST_BPP);
    cal_fps(&g_fps);
    if (enroll)
        wfFR_recognise(p,w,h);
    else
        ssd_rknn_process(p,w,h);
}

int ssd_post(void *flag)
{
    FRIDList msg;
    int spoof_status;

//    int width = SRC_W;
//    int heigh = SRC_H;
    float *predictions;
    unsigned long *output_classes;
    struct ssd_group *group;

    while(*(int *)flag) {
        if (!rknn_msg_recv(&msg,&spoof_status))
            recv_count++;

        predictions = (float*) malloc(sizeof (float) * msg.nResults);
        output_classes = (unsigned long*) malloc(sizeof (unsigned long) * msg.nResults);
        group = &g_ssd_group[!cur_group];

        group->count = msg.nResults;

        for(int i =0; i<msg.nResults;i++) {
            predictions[i] = msg.pConfidence[i];
            output_classes[i] = msg.pRecordID[i];

            char conf[7];
            gcvt(predictions[i],6, conf);

            if (predictions[i] > 0.0f) {
                strcpy(group->objects[i].name, "Name : ");
                strcat(group->objects[i].name, msg.pFName[i]);
                strcat(group->objects[i].name, " ");
                strcat(group->objects[i].name, msg.pLName[i]);
                strcat(group->objects[i].name, " Confidence: ");
                strcat(group->objects[i].name, conf);
            } else {
                strcpy(group->objects[i].name, "");
            }

            group->objects[i].select.left =  msg.pFace[i].x;
            group->objects[i].select.top = msg.pFace[i].y;
            group->objects[i].select.right = msg.pFace[i].x + msg.pFace[i].width;
            group->objects[i].select.bottom =msg.pFace[i].y + msg.pFace[i].height;
            group->objects[i].spoof_status = spoof_status;

            printf("points: %.2f %lu \n",predictions[i], output_classes[i]);

        }

        // if (group->count > 0 && group->posted > 0)
        // {
        //     if (predictions)
        //         free(predictions);
        //     if (output_classes)
        //         free(output_classes);
        //     printf("throw data\n");
        //     cur_group = !cur_group;
        //     continue;
        // }
        // postProcessSSD(predictions, output_classes, width, heigh, group, spoof_status);
        cur_group = !cur_group;
        if (predictions)
            free(predictions);
        if (output_classes)
            free(output_classes);
        ssd_paint_object_msg();
    }
}

void jpegRun(char* image_path, int* flag ){
    FRIDList pResult;
    int spoof_status = 0;

    wfFR_setInputImageBufferFormat(IMAGE_YUV420);
    wfFR_EnrollFaceFromSavedImage(hFR, image_path,recordID,&pResult,&spoof_status);

    printf("Num results %s %d\n", image_path,pResult.nResults);
    printf("spoof status: %d\n",spoof_status); 
    printf("Release FR handle\n");
    /* Release FR handle */
    wfFR_Release(&hFR);
}



int ssd_run(void *flag)
{
    flag = (int*) &continue_camera_capture;

    int status = 0;
    printf("Start: "); checkMemoryc();

    //char pBasePath[255] = {"/userdata/frsdk_demo/"};
    char pBasePath[255] = {"/usr/share/rknn_demo/frsdk_demo/"};
    //                /*"./b1.bin"; */ {0};

    printf("Before Init: "); checkMemoryc();

    /* Initialize the engine for recognition. Give input width, height and widthstep for Y channel image.
       pBasePath is the full path of the data files like cp.bin, mpE.dat etc */

    int rval;
    if (enroll) {
        rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_ENROLL, 0);
        // Set start time   d
        gettimeofday(&start, NULL);
        int rval = wfFR_AddRecord(hFR, &recordID, first_name, last_name);
        if(rval != FR_OK)
        {
            printf("Error in Adding record\n");
            return 0;
        }
    }
    else
        rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_RECOGNIZE, 0);

    printf("After Init: "); checkMemoryc();

    if(rval != FR_OK ||  hFR == NULL)
    {
        printf("Error in FR init\n");
        return 0;
    }
    printf("Init done\n");

    rval = wfFR_setVerbose("",1);
    wfFR_enableImageSaveForDebugging(0);
//    wfFR_setDetectionAlgoType(0);
    wfFR_setInputImageBufferFormat(IMAGE_YV12_INV);
    wfFR_setMinFaceDetectionSizePercent(10);

    if (rval != FR_OK) {
        printf("Error on verbose\n");
    }

    if (enroll) {
        /*Add new record for the person with firstname and lastname*/
        rval = wfFR_AddRecord(hFR, &recordID, first_name, last_name);
        if(rval != FR_OK)
        {
            printf("Error in Adding record\n");
            return 0;
        }
        gettimeofday(&start, NULL);
    }

    // Open Camera and Run

    if(jpeg && enroll) {
        jpegRun(jpegPath, flag);
    } else {
    	cameraRun(dev_name, SRC_W, SRC_H, SRC_FPS, SRC_V4L2_FMT,ssd_camera_callback, (int*)flag);
    }


    printf("exit cameraRun\n\n");
    if (enroll)
        printf("Press Ctrl + C kill the application.\n");

    return status;
}


int ssd_init(char *name)
{
    dev_name = name;
    rknn_msg_init();
    buffer_init(DST_W, DST_H, DST_BPP, &g_rga_buf_bo,
                &g_rga_buf_fd);

    if (!g_mem_buf)
        g_mem_buf = (char *)malloc(DST_W * DST_H * DST_BPP / 8);
}

int ssd_deinit()
{
    if (g_mem_buf)
        free(g_mem_buf);
    buffer_deinit(&g_rga_buf_bo, g_rga_buf_fd);
    rknn_msg_deinit();
}
