#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "FRLibrary.h"
//#include "opencv/cv.h"

#define MAX_FACES (1)


#ifdef __linux__
#include <sys/resource.h>
#include <sys/time.h>
#endif

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




/* To run recognize, initialize FR handle in recognition mode. Any handle in enroll should be released prior to this.
Send images frame by frame to API wfFR_RecognizeFaces to run face detection and recognition.
*/
int testRecognise()
{

	printf("Start: "); checkMemoryc();
	void* hFR = NULL;
	char pBasePath[255] = {0};

	
	printf("Before Init: "); checkMemoryc();

	/* Initialize the engine for recognition. Give input width, height and widthstep for Y channel image.
	   pBasePath is the full path of the data files like cp.bin, mpE.dat etc */
	int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_RECOGNIZE, 0);

	printf("After Init: "); checkMemoryc();

	int width = 640;
	int height = 480;
	int step = 640;
	unsigned char* imageData = new unsigned char[step*height];
	memset(imageData, 0, step*height);
	
	FILE* fp = fopen("frtestimage.dat", "rb");
	if(fp == NULL)
	{
		printf("Error in loading frtestimage.dat\n");
		return 0;
	}
	int readval = fread(imageData, step*height, 1, fp);
	if(readval != 1)
	{
		printf("Error in reading frtestimage.dat\n");
		return 0;
	}
	fclose(fp);

	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}
	printf("Init done\n");
 	
	int algotype = 0;
	wfFR_getDetectionAlgoType(&algotype);
	if(algotype == 0)
	{
		for(int m=0; m<10; m++)
		{
			FRIDList pResult;
			int frstatus = 0;

			printf("Before recog: "); checkMemoryc();
			struct timeval start, end;
			gettimeofday(&start, NULL);

			/* Call to recognize face, to be called repeatedly on image stream coming from camera.*/
			int rval =  wfFR_RecognizeFaces(hFR, imageData, width, height, step, &pResult, &frstatus);

			gettimeofday(&end, NULL);
			float totalTime = (end.tv_sec  - start.tv_sec)*1000.0f + (end.tv_usec - start.tv_usec)/1000.0f;

			printf("After recog: "); checkMemoryc();

			printf("Num results %d, Total time %.1f\n", pResult.nResults, totalTime);	// prints number of faces recognized
			for(int i=0;i<pResult.nResults;i++)
			{
				/* If pConfidence > 0 then face is recognized */
				printf("Frame %d: face %d, Conf %.1f, size %d, ID %d\n",m, i, 
						pResult.pConfidence[i], pResult.pFace[i].width, (int)pResult.pRecordID[i]);
			}
			printf("\n\n");

		}
	}
	else
	{
		printf("Only NV21 input is supported. Input was gray scale image\n");
	}
	
	printf("Release FR handle\n");	
	/* Release FR handle */
	wfFR_Release(&hFR);
	printf("After release: "); checkMemoryc();

}


/* To run enroll, initialize FR handle in enrolling mode. Any handle in recognition mode should be released before to this.
Send images frame by frame to API wfFR_EnrollFace to run face detection and enrollment.
*/
int testEnrolling()
{

	printf("Start: "); checkMemoryc();
	void* hFR = NULL;
	char pBasePath[255] = {0};

	int width = 640;
	int height = 480;
	int step = 640;
	unsigned char* imageData = new unsigned char[step*height];
	memset(imageData, 0, step*height);
	
	FILE* fp = fopen("frtestimage.dat", "rb");
	if(fp == NULL)
	{
		printf("Error in loading frtestimage.dat\n");
		return 0;
	}
	int readval = fread(imageData, step*height, 1, fp);
	if(readval != 1)
	{
		printf("Error in reading frtestimage.dat\n");
		return 0;
	}
	fclose(fp);

	printf("Before Init: "); checkMemoryc();

	/* Initialize the engine for enrolling. Give input width, height and widthstep for Y channel image.
	   pBasePath is the full path of the data files like cp.bin etc */
	int rval = wfFR_Init(&hFR, pBasePath, width, height, step, FRMODE_ENROLL, 0);

	printf("After Init: "); checkMemoryc();
	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}
	printf("Init done\n");
 	
	
	/*Add new record for the person with firstname and lastname*/
	unsigned long recordID = -1;
	rval = wfFR_AddRecord(hFR, &recordID, "Jen", "Aniston");
	if(rval != FR_OK)
	{
		printf("Error in Adding record\n");
		return 0;
	}
	
	int algotype = 0;
	wfFR_getDetectionAlgoType(&algotype);
	if(algotype == 0)
	{
		for(int m=0; m<10; m++)
		{
			FRIDList pResult;
			int frstatus = 0;

			printf("Before enroll: "); checkMemoryc();
			/* Call to enroll face, to be called repeatedly on image stream coming from camera.*/
			int rval =  wfFR_EnrollFace(hFR, imageData, width, height, step, recordID, 0, &pResult, &frstatus);
			printf("After enroll: "); checkMemoryc();

			printf("Num results %d\n", pResult.nResults);	// prints number of faces
			for(int i=0;i<pResult.nResults;i++)
			{
				/* If pConfidence = 0 then the current face image is enrolled */
				printf("Frame %d: face %d, Conf %.1f, size %d\n",m, i, 
						pResult.pConfidence[i], pResult.pFace[i].width);
			}
			printf("\n\n");

		}
	}
	else
	{
		printf("Only NV21 input is supported. Input was gray scale image\n");
	}
	
	printf("Release FR handle\n");	
	/* Release FR handle */
	wfFR_Release(&hFR);
	printf("After release: "); checkMemoryc();

}

int main()
{

	testRecognise();
	//testEnrolling();	
	
}
