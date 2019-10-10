#include <stdio.h>
#include <string>
#include <iostream>

#ifdef _LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <opencv/cv.h>
#include <opencv/highgui.h>



#include "FRLibrary.h"
#include "FRLibraryPC.h"


#define PCTEST (1)
#define USEWEBCAM (0)

#ifdef __linux__
#include <sys/resource.h>
#endif

using namespace cv;

/* Number of maximum faces to be detected in the scene */
#define MAX_FACES (1)

int testEnrollFromVideo(string imagename);
int testEnrollFromVideoNV21(string imagename);
int testEnrollFromWecam();
int testEnrollFromSaveImage();
int testEnrollFromJpegBuffer();

extern int frEngineMode;
int process_image_FR(const void * p, int width, int height);

int realFaceVerified = 0;


/**
 * Returns the current resident set size (physical memory use) measured
 * in bytes, or zero if the value cannot be determined on this OS.
 */
size_t getCurrentRSS( )
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


void checkMemory()
{
#ifdef __linux__
	struct rusage r_usage;
  	getrusage(RUSAGE_SELF,&r_usage);
  	printf("Memory usage Max: %ld kb, curr %ld kb\n",r_usage.ru_maxrss, getCurrentRSS( ));
#endif
}



int displayResults(IplImage* testImage, FRIDList pResult, int framecounter, int faceverified)
{
	
#if PCTEST	 // display results

	CvFont font;
	cvInitFont(&font, 0, 1.2, 1.2, 0, 3);
	for(int i=0;i<pResult.nResults;i++)
	{
		int w = pResult.pFace[i].width;
		int h = pResult.pFace[i].height;
		int x = pResult.pFace[i].x;
		int y = pResult.pFace[i].y;
		CvScalar color = cvScalar(0, 0, 255);
		char pBuff[256];
		pBuff[0] = '\0';

		if(faceverified == 1)
		{
			cvRectangle(testImage, cvPoint(x, y), cvPoint(x + w, y + h), cvScalar(255, 0, 0), 6);
		}

		if(pResult.pConfidence[i] > 60)
		{
			char outname[255];
			//sprintf(outname, "results/image_%d.jpg", framecounter);
			//cvSaveImage(outname, testImage);
		}
		if(pResult.pConfidence[i] > 0)
		{
			color = cvScalar(255, 0, 0);
			sprintf(pBuff, "%s %s: %.1f, ID %d", pResult.pFName[i], pResult.pLName[i], pResult.pConfidence[i], pResult.pTrackID[i]);
			cvPutText(testImage, pBuff, cvPoint(x, y - 40), &font, color);
		}
		else
		{
			color = cvScalar(255, 0, 0);
			sprintf(pBuff, "ID %d",  pResult.pTrackID[i]);
			cvPutText(testImage, pBuff, cvPoint(x, y - 40), &font, color);
		}
		cvRectangle(testImage, cvPoint(x, y), cvPoint(x + w, y + h), color, 2);
	}
	cvShowImage("Display", testImage);
	//if(pResult.nResults > 0 && pResult.pConfidence[0] > 0)
		//cvWaitKey(10);
	char c = cvWaitKey(10);
	
	if(c == 'q')
	{
		return 0;
	}
#endif

	return 1;
}

/* To run recognize, initialize FR handle in recognition mode. Any handle in enroll should be released prior to this.
Send images frame by frame to API wfFR_RecognizeFaces to run face detection and recognition.
*/
int testRecogniseFromVideo()
{		
	printf("Recognize Start : "); checkMemory();

#if USEWEBCAM
	
	cv::VideoCapture vcap;
	vcap.open(0);
	if (vcap.isOpened())
	{
		vcap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
		vcap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	}
	else
	{
		printf("Error in capturing from camera\n");
		return 0;
	}

	Mat imgMatColor;
	vcap.read(imgMatColor);
	if (imgMatColor.empty())
	{
		printf("Failed to capture image \n");
		return 0;
	}
	else
		printf("Capture started %d %d\n", imgMatColor.cols, imgMatColor.rows);
	
	Mat imgMat;
	cvtColor(imgMatColor, imgMat, CV_BGR2GRAY);
	IplImage imager = imgMat;
	IplImage* testImage = &imager;
#else
	//printf("Before image load : "); checkMemory();
	char* imagename = "testimages/jen_aniston_004.jpg";

	IplImage* testImage = cvLoadImage(imagename, 0);
	//printf("After image load : "); checkMemory();
	if(testImage == NULL)
	{
		printf("Error in loading image %s\n", imagename);
		return 0;
	}
#endif

	void* hFR = NULL;
	char pBasePath[255] = {0};

	const char* versionStr = wfFR_GetVersionInfo();
	printf("Version - <%s>\n", versionStr);

	//wfFR_setOnlineLicensing(1);

#if 0
	int licval = wfFR_VerifyLic(pBasePath);
	if(licval != FR_OK)
	{
		printf("License check failed %d\n", licval);
		return 0;
	}
#endif
	
	wfFR_setVerbose("", 1);
	wfFR_setMinFaceDetectionSizePercent(10);
	//wfFR_setSaveDetectedFaceFlag(1);
	//wfFR_setDetectionOnlyMode(1);
	//wfFR_setDeleteExistingNamePCDBUpdate(0);

	//wfFR_setDatabaseFolderPath("basepathdb/");
	//wfFR_setRecognitionThreshold(55);
	wfFR_setRecogQualityCheckFlag(1);
	//wfFR_setDetectionAlgoType(0);

	printf("Before wfFR_Init : "); checkMemory();
	double t0 = cvGetTickCount();
	/* Initialize the engine for recognition. Give input width, height and widthstep for Y channel image.
	   pBasePath is the full path of the data files like cp.bin, p1.bin etc */
	int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_RECOGNIZE, 0);
	
	t0 = (cvGetTickCount() - t0)/(1000.0*cvGetTickFrequency());
	printf("Init time %.1f\n", t0);
	printf("After wfFR_Init : "); checkMemory();

	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init rval %d, hFR %d\n",rval, hFR);
		return 0;
	}
	printf("Init done Recognition\n");fflush(stdout);

	//wfFR_setSpoofingSensitivity(hFR, 3);
	//float conf = 0;
	//wfFR_getRecognitionThreshold(&conf);
	//printf("***************** conf %f\n", conf);

	int frameCounter = 1;
	while(1)
	{
#if USEWEBCAM
		vcap.read(imgMatColor);
		if (imgMatColor.empty())
		{
			printf("Capture end\n");
			break;
		}
		cvtColor(imgMatColor, imgMat, CV_BGR2GRAY);

#else
		cvReleaseImage(&testImage);	
		testImage = cvLoadImage(imagename,0);
		if(testImage == NULL)
		{
			printf("Error in loading %s\n", imagename);
			break;
		}
		if(frameCounter > 20) break;
#endif

		FRIDList pResult;
		int frstatus = 0;
		
		printf("before rec API: "); checkMemory();
		double t1 = cvGetTickCount();
		/* Call to recognize face, to be called repeatedly on image stream coming from camera.*/
		//int rval =  wfFR_RecognizeFaces(hFR, (unsigned char*)(testImage->imageData), testImage->width, testImage->height, testImage->widthStep, &pResult, &frstatus);

		//int rval =  wfFR_DetectRecognizeFacesMultiThread(hFR, (unsigned char*)(testImage->imageData), testImage->width, testImage->height, testImage->widthStep, &pResult, &frstatus);

		int rval =  wfFR_DetectRecognizeFacesQueue(hFR, (unsigned char*)(testImage->imageData), testImage->width, testImage->height, testImage->widthStep, &pResult, &frstatus);	

		if(rval != FR_OK)
		{
			printf("Error in wfFR_RecognizeFaces\n");
			break;
		}
		t1 = (cvGetTickCount() - t1)/(1000.0*cvGetTickFrequency());

		realFaceVerified = frstatus;

		printf("after rec API: "); checkMemory();

		
	
		char detImageName[2048];
		wfFR_getDetectedWfgName(detImageName);
		printf("Status %d, Num results %d, Time %.3f milliseconds, frstatus %d, res %dx%d, detImageName <%s>\n", frstatus, pResult.nResults, t1, frstatus, testImage->width, testImage->height, detImageName);	// prints number of faces recognized
		for(int i=0;i<pResult.nResults;i++)
		{
			//if(pResult.pConfidence[i] > 0)
			/* If pConfidence > 0 then face is recognized */
			printf("Frame %d (V1): face %d, Conf %.2f, size %d, ID %d\n", frameCounter, i, pResult.pConfidence[i], pResult.pFace[i].width, pResult.pRecordID[i]);
	
		}
		printf("\n\n");
#if USEWEBCAM
		int disprval = displayResults(testImage, pResult, frameCounter, realFaceVerified);
		if(disprval == 0)
			break;

#endif

		frameCounter++;
	}
	
	printf("Release FR handle\n");	
	/* Release FR handle */
	wfFR_Release(&hFR);

	printf("After FR release : "); checkMemory();
	
#if USEWEBCAM
	vcap.release();
#else
	cvReleaseImage(&testImage);
#endif
	printf("TEST End : ");checkMemory();
	return 1;
}



// NV21 is YVU420 which is YCrCb420 as in below function. NV12 is is YUV420
void ConvertBGR_TO_YUV420SP_Rec(const IplImage* pBGR, IplImage* yuv420, int outw, int outh)
{
    //IplImage* pYUV = 0;
    int i, j;
    CvSize size;
    
    size = cvGetSize(pBGR);
    //pYUV = cvCreateImage(size, 8, 3);
    //cvCvtColor(pBGR, pYUV, CV_BGR2YCrCb);

    for (i = 0; i < outh; i++)
    {
        const uchar* pBGRData = (const uchar*) (pBGR->imageData + i*pBGR->widthStep);
        uchar* pYData = (uchar*) (yuv420->imageData + i*(yuv420->widthStep));
        uchar* pVUData = (uchar*) (yuv420->imageData + outh*(yuv420->widthStep) + (i/2)*(yuv420->widthStep));
        for (j = 0; j < outw; j++, pBGRData+=3)
        {
		int B = pBGRData[0];
		int G = pBGRData[1];
		int R = pBGRData[2];
		int Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
            	
		pYData[j] = MAX(MIN(Y, 255), 0);		
		//pYData[j] = pYUVData[3*j+0];
		if (((i&1) == 0) && ((j&1) == 0))
		{
			int U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
        	        int V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

			//pVUData[j+0] = pYUVData[3*j+1];
			//pVUData[j+1] = pYUVData[3*j+2];
			pVUData[j+0] = V;
			pVUData[j+1] = U;
		}
        }
    }

    //cvReleaseImage(&pYUV);
}


/* To run recognize, initialize FR handle in recognition mode. Any handle in enroll should be released prior to this.
Send images frame by frame to API wfFR_RecognizeFaces to run face detection and recognition.
*/
int testRecogniseFromVideoNV21()
{		
	printf("Recognize Start : "); checkMemory();

#if USEWEBCAM
	
	cv::VideoCapture vcap;
	vcap.open(0);
	if (vcap.isOpened())
	{
		vcap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
		vcap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	}
	else
	{
		printf("Error in capturing from camera\n");
		return 0;
	}

	Mat imgMatColor;
	vcap.read(imgMatColor);
	if (imgMatColor.empty())
	{
		printf("Failed to capture image \n");
		return 0;
	}
	else
		printf("Capture started %d %d\n", imgMatColor.cols, imgMatColor.rows);
	
	IplImage imager = imgMatColor;
	IplImage* testImageColor = &imager;
#else
	//printf("Before image load : "); checkMemory();
	char* imagename = "testimages/jen_aniston_004.jpg";
	//char* imagename = "group1080p.jpg";

	IplImage* testImageColor = cvLoadImage(imagename, 1);
	printf("After image load : %s\n", imagename); 
	if(testImageColor == NULL)
	{
		printf("Error in loading image %s\n", imagename);
		return 0;
	}
#endif

	void* hFR = NULL;
	char pBasePath[255] = {0};

	const char* versionStr = wfFR_GetVersionInfo();
	printf("Version - <%s>\n", versionStr);

	//wfFR_setOnlineLicensing(1);

#if 0
	int licval = wfFR_VerifyLic(pBasePath);
	if(licval != FR_OK)
	{
		printf("License check failed %d\n", licval);
		return 0;
	}
#endif
	
	wfFR_setVerbose("", 1);
	wfFR_setMinFaceDetectionSizePercent(10);
	//wfFR_setDetectionOnlyMode(1);
	wfFR_setRecogQualityCheckFlag(2);
	//wfFR_setDetectionAlgoType(0);

	//FRRecogQualityParams qparams;
	//wfFR_getRecogQualityParameters(&qparams);
	//qparams.maxBlur = 60;
	//wfFR_setRecogQualityParameters(qparams);

	printf("Before wfFR_Init : "); checkMemory();
	double t0 = cvGetTickCount();
	/* Initialize the engine for recognition. Give input width, height and widthstep for Y channel image.
	   pBasePath is the full path of the data files like cp.bin, p1.bin etc */
	int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_RECOGNIZE, 0);
	
	t0 = (cvGetTickCount() - t0)/(1000.0*cvGetTickFrequency());
	printf("Init time %.1f\n", t0);
	printf("After wfFR_Init : "); checkMemory();

	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init rval %d, hFR %d\n",rval, hFR);
		return 0;
	}
	printf("Init done Recognition\n");fflush(stdout);


	int frameCounter = 1;
	while(1)
	{
#if USEWEBCAM
		vcap.read(imgMatColor);
		if (imgMatColor.empty())
		{
			printf("Capture end\n");
			break;
		}
#else
		cvReleaseImage(&testImageColor);	
		testImageColor = cvLoadImage(imagename);
		if(testImageColor == NULL)
		{
			printf("Error in loading %s\n", imagename);
			break;
		}
		if(frameCounter > 20) break;
#endif
		CvRect imgRoi = cvRect(0,0,testImageColor->width, testImageColor->height);
		if(imgRoi.width%4 != 0)
			imgRoi.width = imgRoi.width - imgRoi.width%4;
		if(imgRoi.height%2 != 0)
			imgRoi.height = imgRoi.height - imgRoi.height%2;
		printf("testImageColor->width %d, testImageColor->height %d, ROI %d %d\n", testImageColor->width, testImageColor->height, imgRoi.width, imgRoi.height);
		cvSetImageROI(testImageColor, imgRoi);
		IplImage* yuv420 = cvCreateImage(cvSize(imgRoi.width, (int)(imgRoi.height*1.5f+0.5f)), 8,1);
		ConvertBGR_TO_YUV420SP_Rec(testImageColor, yuv420, imgRoi.width, imgRoi.height);
		cvResetImageROI(testImageColor);


		FRIDList pResult;
		int frstatus = 0;
		
		printf("before rec API: "); checkMemory();
		double t1 = cvGetTickCount();
		/* Call to recognize face, to be called repeatedly on image stream coming from camera.*/
		int rval =  wfFR_RecognizeFaces(hFR, (unsigned char*)(yuv420->imageData), imgRoi.width, imgRoi.height, imgRoi.width, &pResult, &frstatus);

		if(rval != FR_OK)
		{
			printf("Error in wfFR_RecognizeFaces\n");
			break;
		}
		t1 = (cvGetTickCount() - t1)/(1000.0*cvGetTickFrequency());

		realFaceVerified = frstatus;

		printf("after rec API: "); checkMemory();

		char detImageName[2048];
		wfFR_getDetectedWfgName(detImageName);
		printf("Status %d, Num results %d, Time %.3f milliseconds, frstatus %d, res %dx%d, detImageName <%s>\n", frstatus, pResult.nResults, t1, frstatus, testImageColor->width, testImageColor->height, detImageName);	// prints number of faces recognized
		for(int i=0;i<pResult.nResults;i++)
		{
			//if(pResult.pConfidence[i] > 0)
			/* If pConfidence > 0 then face is recognized */
			printf("Frame %d (V1): face %d, Conf %.2f, size %d, ID %d\n", frameCounter, i, pResult.pConfidence[i], pResult.pFace[i].width, pResult.pRecordID[i]);
	
		}
		printf("\n\n");
#if 0 //USEWEBCAM
		int disprval = displayResults(testImageColor, pResult, frameCounter, realFaceVerified);
		if(disprval == 0)
			break;

#endif

		frameCounter++;
		cvReleaseImage(&yuv420);
	}
	
	printf("Release FR handle\n");	
	/* Release FR handle */
	wfFR_Release(&hFR);

	printf("After FR release : "); checkMemory();
	
#if USEWEBCAM
	vcap.release();
#else
	cvReleaseImage(&testImageColor);
#endif
	printf("TEST End : ");checkMemory();
	return 1;
}

int testStateMachine()
{
	IplImage* testImage = cvLoadImage("testimages/jen_aniston_004.jpg", 0);
	if(testImage == NULL)
	{
		printf("Error in loading image\n");
		return 0;
	}
	IplImage* yuv2 = cvCreateImage(cvGetSize(testImage),8,2);
	cvZero(yuv2);
	for(int h=0;h<testImage->height;h++)
	{
		unsigned char* srcpix = cvPtr2D(testImage,h,0);
		unsigned char* dstpix = cvPtr2D(yuv2,h,0);
		for(int w=0;w<testImage->width;w+=2)
		{
			dstpix[2*w] = srcpix[w];
			dstpix[2*w+2] = srcpix[w+1];
		}
	}
	printf("W %d, H %d\n", testImage->width, testImage->height);

	frEngineMode = 1;
	while(1)
	{
		process_image_FR((const void*)(yuv2->imageData), testImage->width, testImage->height);
		if(frEngineMode == 0)
			break;
#ifdef _LINUX
		usleep(100000);
#endif
	}
}

void testDbManagement()
{
	void* hFR = NULL;
	char pBasePath[255] = {0};
	int rval = 0;
	double t0 = 0;

	//wfFRPC_mergePCDB(pBasePath);
	//wfFRPC_mergePCDB(pBasePath);
	//return;

	checkMemory();
	//for(int i=0;i<10;i++)
	//{
	t0 = cvGetTickCount();
	/* Initialize the engine for recognition. Give input width, height and widthstep for Y channel image.
	pBasePath is the full path of the data files like cp.bin, mpE.dat etc */
	rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_RECOGNIZE, 0);

	t0 = (cvGetTickCount() - t0)/(1000.0*cvGetTickFrequency());
	printf("Init time %.1f\n", t0);
	//wfFR_Release(&hFR);
	//}
	//return ;
	checkMemory();

	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init\n");
		return;
	}
	printf("Init done DB manage\n");fflush(stdout);


	/* Remove all record for a person with given ID */
	//rval =  wfFR_RemovePersonByRecordID(hFR,0);
	//printf("rval %d\n", rval);

	/* Remove all record for a person with given first and second name */
	//t0 = cvGetTickCount();
	//rval = wfFR_RemovePersonByName(hFR, "3303116", "");	//
	//t0 = (cvGetTickCount() - t0)/(1000.0*cvGetTickFrequency());
	//printf("RemovePersonByName time %.1f\n", t0);
	//printf("RemovePersonByName rval %d\n", rval);

	/* Remove all record for a person with given first name only */
	//rval = wfFR_RemovePersonByFirstName(hFR, "Jen");
	//printf("rval %d\n", rval);

	/* Remove all record for all people in database */
	rval = wfFR_RemoveAllRecord(hFR);
	printf("rval %d\n", rval);

	/* Rename record in database */ 
	//rval = wfFR_RenameRecordID(hFR, 1, "Albert", "Jones");	
	//printf("rval %d\n", rval);

	/* Rename by name in database */ 
	//rval = wfFR_RenamePersonByName(hFR, "Jen9", "Aniston9", "Jen8", "Aniston8");	
	//printf("rval %d\n", rval);

	/* Extract and save person from DB */	
	//rval = wfFR_ExtractPersonByName(hFR, "Jen5", "Aniston");
	//printf("rval %d\n", rval);
	
	/////////// Get list of names in database //////////
	std::vector<std::string> outFirstNameList, outSecondNameList; 
	std::vector<int> outIDList;
	outFirstNameList.reserve(10000);
	outSecondNameList.reserve(10000);
	outIDList.reserve(10000);
	//wfFR_test(&outIDList);

	t0 = cvGetTickCount();
	rval =  wfFR_GetNamelist(hFR, outFirstNameList, outSecondNameList, outIDList);
	t0 = (cvGetTickCount() - t0)/(1000.0*cvGetTickFrequency());
	printf("GetNamelist time %.3f, num %d\n", t0, outFirstNameList.size());

#if 1
	if(outFirstNameList.size() > 0)
	{
		double t1 = cvGetTickCount();
		//wfFR_RemovePersonByName(hFR, outFirstNameList[0].c_str(), outSecondNameList[0].c_str());
		//wfFR_RemoveAllRecord(hFR);
		t1 = (cvGetTickCount() - t1)/(1000.0*cvGetTickFrequency());
		//printf("RemovePersonByName time %.3f\n", t1);
	}

	int numzeros= 0;
	for(int i=0;i<outFirstNameList.size();i++)
	{
		
	
		printf("%d: ID %d, Firstname <%s>, secondname <%s> \n", i, outIDList[i], outFirstNameList[i].c_str(), outSecondNameList[i].c_str());
	
		
		std::vector<int> outRecordIDList, outNumImagesList;
		outRecordIDList.reserve(100);
		outNumImagesList.reserve(100);
		/////////// Get list of record ID's for a given name in database //////////
		double t1 = cvGetTickCount();
		rval =  wfFR_GetRecordIDListByName(hFR, outFirstNameList[i].c_str(), outSecondNameList[i].c_str(), outRecordIDList, outNumImagesList);
		t1 = (cvGetTickCount() - t1)/(1000.0*cvGetTickFrequency());
		//printf("GetRecordIDListByName time %.3f\n", t1);
		//for(int j=0;j<outRecordIDList.size();j++)
		//	printf("(%d, %d) ",outRecordIDList[j], outNumImagesList[j]);
		//printf("\n");

		//if(outRecordIDList.size() > 2)
		//{
			/////////// Remove single record ID in database //////////
		//	rval =  wfFR_RemoveSingleRecord(hFR, outRecordIDList[0]);
		//}
	}
	printf("numzeros %d\n", numzeros);
#endif
	


	printf("Release FR handle\n");	
	/* Release FR handle */
		wfFR_Release(&hFR);

}

/* To run recognize from saved image, initialize FR handle in recognition mode.
Send images name to API wfFR_RecognizeFacesFromSavedImage to run face detection and recognition.
*/
int testRecogniseFromSaveImage()
{		
	printf("Recognize from Saved image start\n");
	void* hFR = NULL;
	char pBasePath[255] = {0};
	int rval = 0;

	wfFR_setDetectionAlgoType(1);
	wfFR_setInputImageBufferFormat(IMAGE_YV12_INV);

	wfFR_setMinFaceDetectionSizePercent(10);
	rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_RECOGNIZE, 0);
	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}
	printf("Init done Recognition\n");fflush(stdout);

	float threshold = 0;

	//wfFR_setRecognitionThreshold(10);
	wfFR_setVerbose("", 1);

	wfFR_getRecognitionThreshold(&threshold);
#if 0
	std::vector<std::string> DbFirstNameList;	// list of first names in DB
	std::vector<std::string> DbSecNameList;	// list of second names in DB
	std::vector<int> DbIDList;		// list of recordID's in DB
	DbFirstNameList.clear();
	DbSecNameList.clear();
	DbIDList.clear();
	rval = wfFR_GetNamelist(hFR, DbFirstNameList,  DbSecNameList, DbIDList);
	if(rval != 0)
	{
		printf("Error in wfFR_GetDbRecords %d\n", rval);
	 	
	}

	printf("Number of people in DB %d, %d\n", (int)DbFirstNameList.size(), (int)DbSecNameList.size());
    	
	for (int i = 0; i < (int)DbFirstNameList.size(); ++i) 
	{
		char curname[512];
		//printf("%d: <%s %s>\n", i, DbFirstNameList[i].c_str(),DbSecNameList[i].c_str());
	}
#endif
	const char* pImageFilename = "testimages/jen_aniston_004.jpg";
	//const char* pImageFilename = "testimages/group.jpg";
	for(int m=0;m<10;m++)
	{
		FRIDList pResult;
		int frstatus = 0;
		double t1 = cvGetTickCount();
		rval = wfFR_RecognizeFacesFromSavedImage(hFR, pImageFilename, &pResult, &frstatus);
	
		t1 = (cvGetTickCount() - t1)/(1000.0*cvGetTickFrequency());
		printf("Status %d, Num results %d, Time %.3f\n", frstatus, pResult.nResults, t1);	// prints number of faces recognized
		for(int i=0;i<pResult.nResults;i++)
		{
			/* If pConfidence > 0 then face is recognized */
			printf("Face %d, Conf %.3f (%.1f), size %d, ID %d\n", i, pResult.pConfidence[i], threshold, pResult.pFace[i].width, pResult.pRecordID[i]);
			printf("X %d, Y %d, %d %d\n", pResult.pFace[i].x, pResult.pFace[i].y, pResult.pFace[i].width, pResult.pFace[i].height);
		}
		printf("\n");
	}
	/* Release FR handle */
	wfFR_Release(&hFR);
}

int testRecogniseFromJpegBuffer()
{		
	printf("Recognize from Jpeg buffer start\n");
	void* hFR = NULL;
	char pBasePath[255] = {0};
	int rval = 0;

	wfFR_setRecognitionThreshold(10);
	wfFR_setVerbose("", 1);

	rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_RECOGNIZE, 0);
	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}
	printf("Init done Recognition\n");fflush(stdout);

	const char* pImageFilename = "testimages/jen_aniston_004.jpg";
	FILE* fp = fopen(pImageFilename,"rb");
	if(fp==NULL)
	{
		printf("Error in reading file\n");
		return 0;
	}
	fseek (fp , 0 , SEEK_END);
	int sz = ftell (fp);
	rewind (fp);
	char* jpegBuffer = new char[sz];
	fread (jpegBuffer, 1, sz, fp);
	fclose (fp);

	//const char* pImageFilename = "usbtest.jpg";
	FRIDList pResult;
	int frstatus = 0;
	double t1 = cvGetTickCount();
	rval = wfFR_RecognizeFacesFromJpegBuffer(hFR, jpegBuffer, sz, &pResult, &frstatus);
	
	t1 = (cvGetTickCount() - t1)/(1000.0*cvGetTickFrequency());
	printf("Status %d, Num results %d, Time %.3f\n", frstatus, pResult.nResults, t1);	// prints number of faces recognized
	for(int i=0;i<pResult.nResults;i++)
	{
		/* If pConfidence > 0 then face is recognized */
		printf("Face %d, Conf %.1f, size %d, ID %d\n", i, pResult.pConfidence[i], pResult.pFace[i].width, pResult.pRecordID[i]);
	}

	delete [] jpegBuffer;

	/* Release FR handle */
	wfFR_Release(&hFR);
}


int testVerifyFaceForEnroll()
{		
	printf("Check image for enrollment from Jpeg buffer start\n");
	void* hFR = NULL;
	char pBasePath[255] = {0};
	int rval = 0;

	rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_ENROLL, 0);
	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}
	printf("Init done Recognition\n");fflush(stdout);

	const char* pImageFilename = "testimages/jen_aniston_004.jpg";

	FRIDList pResult;
	int frstatus = 0;

	//wfFR_setEnrollQualityCheckFlag(2);

#if 0
	printf("Testing image %s\n", pImageFilename);
		
	double t1 = cvGetTickCount();
	rval = wfFR_VerifyImageForEnrollSavedImage(hFR, pImageFilename, &pResult, &frstatus);

#endif

#if 1
	printf("Testing image %s\n", pImageFilename);
	FILE* fp = fopen(pImageFilename,"rb");
	if(fp==NULL)
	{
		printf("Error in reading file\n");
		return 0;
	}
	fseek (fp , 0 , SEEK_END);
	int sz = ftell (fp);
	rewind (fp);
	char* jpegBuffer = new char[sz];
	fread (jpegBuffer, 1, sz, fp);
	fclose (fp);

	//const char* pImageFilename = "usbtest.jpg";
	
	double t1 = cvGetTickCount();
	rval = wfFR_VerifyImageForEnrollJpegBuffer(hFR, jpegBuffer, sz, &pResult, &frstatus);
	
	delete [] jpegBuffer;
#endif

#if 0
	IplImage* testImage = cvLoadImage(pImageFilename, 0);
	if(testImage == NULL)
	{
		printf("Error in loading image %s\n", pImageFilename);
		return 0;
	}

	double t1 = cvGetTickCount();
	rval = wfFR_VerifyFrameForEnroll(hFR, (unsigned char*)(testImage->imageData), testImage->width, testImage->height, testImage->widthStep, &pResult, &frstatus);

	
	cvShowImage("testImage", testImage);
	cvWaitKey(0);
	cvReleaseImage(&testImage);
#endif
	
	t1 = (cvGetTickCount() - t1)/(1000.0*cvGetTickFrequency());
	printf("Status %d, Num results %d, Time %.3f\n", frstatus, pResult.nResults, t1);	// prints number of faces recognized
	for(int i=0;i<pResult.nResults;i++)
	{
		/* If pConfidence > 0 then face is recognized */
		printf("Face %d, Conf %.1f, size %d, ID %d\n", i, pResult.pConfidence[i], pResult.pFace[i].width, pResult.pRecordID[i]);
	}

	
	/* Release FR handle */
	wfFR_Release(&hFR);
}

int main()
{
	printf("Testing V31\n"); fflush(stdout);
	//wfFR_enableImageSaveForDebugging(1);
	
	//testEnrollFromWecam();

	//testEnrollFromVideo("testimages/jen_aniston_004.jpg");

	//testRecogniseFromVideo();

	//testEnrollFromSaveImage();
	//testRecogniseFromSaveImage();

	//testEnrollFromJpegBuffer();
	//testRecogniseFromJpegBuffer();

	testEnrollFromVideoNV21("testimages/jen_aniston_003.jpg");

	testRecogniseFromVideoNV21();

	//testDbManagement();

	//testVerifyFaceForEnroll();

}
	

