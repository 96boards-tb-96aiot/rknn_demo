#include <stdio.h>
#include <vector>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "FRLibrary.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif
/* Number of maximum faces to be detected in the scene */
#define MAX_FACES (1)

/* Time in seconds for which enrolling should be run. It can be in range of 5 to 20 seconds*/
#define ENROLLING_TIME (10)

/* Time in seconds for which recognition should be run. It can be in range of 2 to 20 seconds*/
#define RECOGNITION_TIME (10)

void* hFrhandle = NULL;		// Face Recognition (FR) handle
int frhandleInitialized = 0;	// Flag to store initialize status of FR handle
int frEngineMode = 0;		// FR engine mode: 0 for doing nothing, 1 for recognition and 2 for enrolling
int internalfrMode = 0;		// FR engine mode only for internal use: 0 for doing nothing, 1 for recognition and 2 for enrolling
unsigned char* ybuffer = NULL;	// Buffer to store Y frame
const char* pBasePath = "./";	// Path where FR data files are kept. Change for your device
unsigned long recordID = ULONG_MAX;	// Record ID used during enrollment
FRIDList FrResult;			// Detection output from FR
int frFrameCounter = 0;			// For counting number of frames processed
#ifdef _WIN32	
long int startTime;
#else
timeval startTime;			// Start timer for duration of enrollment and recognition
#endif

int inputDataFormat = 0;		// Format of input frame buffer, 0 for YUV420/NV12/NV21/GRAY, 1 for YUV2

int enableEnrollSpoofing = 0;		// Enable spoof detection for enrolling
int enableRecognitionSpoofing = 0;	// Enable spoof detection for recognition

int process_image_FR(const void * p, int width, int height){


	unsigned char* in = (unsigned char*)p;
	int istride = 2*width;
	int rval = 0;
	
	
	printf("FR %d:  frEngineMode %d\n", frFrameCounter, frEngineMode);

	if((internalfrMode == 1) && (frEngineMode == 2)) // state changed from recognition to enroll
	{
		if ((hFrhandle != NULL) && (frhandleInitialized == 1))
		{
			wfFR_Release(&hFrhandle);
			hFrhandle = NULL;
			frhandleInitialized = 0;
		}
	}

	if((internalfrMode == 2) && (frEngineMode == 1)) // state changed from recognition to enroll
	{
		if ((hFrhandle != NULL) && (frhandleInitialized == 1))
		{
			wfFR_Release(&hFrhandle);
			hFrhandle = NULL;
			frhandleInitialized = 0;
		}
	}


	if (frEngineMode == 0)	// nothing to be done. Release handle if it is not null
	{
		internalfrMode = frEngineMode;
		if ((hFrhandle != NULL) && (frhandleInitialized == 1))
		{
				wfFR_Release(&hFrhandle);
				hFrhandle = NULL;
				frhandleInitialized = 0;
		}
		return 0;
	}

	// initialize Y buffer if it is null
	if (ybuffer == NULL)
	{
		ybuffer = (unsigned char*)malloc(width*height);
	}

	if(inputDataFormat == 0)	// Format is YUV420/NV12/NV21/GRAY
	{
		memcpy(ybuffer, in, width*height);
	}
	else if(inputDataFormat == 1)	// Format is YUV2
	{	
		/// Convert YUV2 format to Y
		for (int y = 0; y < height; ++y)
		{
			unsigned char* srcpix = in + y*istride;
			unsigned char* dstpix = ybuffer + y*width;
			int location = 0;
			for (int j = 0; j < width * 2; j += 4, location += 2)
			{
				int y0 = srcpix[j];
				int y1 = srcpix[j + 2];
				dstpix[location] = y0;
				dstpix[location + 1] = y1;
			}
		}
	}

	if (frEngineMode == 2)		// Run Enroll mode
	{
		/// Initialize FR for enrolling
		if (hFrhandle == NULL)
		{
			
			int rval = wfFR_Init(&hFrhandle, pBasePath, width, height, width, FRMODE_ENROLL, enableEnrollSpoofing);
			if (rval != 0)
			{
				printf("Error in initializing FR Enroll\n");
				return rval;
			}

			/// Add nre record to database
			rval = wfFR_AddRecord(hFrhandle, &recordID, "Jen", "Aniston");
			if (rval != 0)
			{
				printf("Error in wfFR_AddRecord\n");
				return rval;
			}
			frFrameCounter = 0;
			frhandleInitialized = 1;
#ifdef _WIN32
			startTime = GetTickCount();
#else
			gettimeofday(&startTime, 0);
#endif
			printf("FR Enroll Initialization done\n");
		}

		if (frhandleInitialized == 1)
		{
			//// Enroll from image stream
			FrResult.nResults = 0;
			int frstatus = 0;
			rval = wfFR_EnrollFace(hFrhandle, ybuffer, width, height, width, recordID, 0, &FrResult, &frstatus);
			if (rval != 0)
			{
				printf("Error in wfFR_EnrollFace\n");
				return rval;
			}

			printf("Face spoof detection %d, Enroll num faces detected %d\n", frstatus, FrResult.nResults);
			for (int i = 0; i<FrResult.nResults; i++)
			{
				printf("Frame %d: size %d\n", frFrameCounter, FrResult.pFace[i].width);
			}

			frFrameCounter++;

#ifdef _WIN32
			long int curTime = GetTickCount();
			double elapsed = (curTime - startTime) / 1000;
#else
			timeval curTime;
			gettimeofday(&curTime, 0);
			double elapsed = (curTime.tv_sec - startTime.tv_sec);
#endif

			printf("Enroll elapsed time %.1f\n", elapsed);
			/// Finish enrolling and release FR after 10 seconds
			if (elapsed > ENROLLING_TIME)
			{
				printf("Enrolling done. Release FR\n");
				wfFR_Release(&hFrhandle);
				hFrhandle = NULL;
				frhandleInitialized = 0;
				frEngineMode = 0;

				free(ybuffer);
				ybuffer = NULL;

			}
		}
	}
	else if (frEngineMode == 1)			// Run Recognition mode
	{
		/// Initialize FR for recognition
		if (hFrhandle == NULL)
		{
			int rval = wfFR_Init(&hFrhandle, pBasePath, width, height, width, 
							FRMODE_RECOGNIZE, enableRecognitionSpoofing);
			if (rval != 0)
			{
				printf("Error in initializing FR Recognize\n");
				return rval;
			}
			frFrameCounter = 0;
			frhandleInitialized = 1;
#ifdef _WIN32
			startTime = GetTickCount();
#else
			gettimeofday(&startTime, 0);
#endif
			printf("FR Recognize Initialization done\n");
		}

		if (frhandleInitialized == 1)
		{

			//// Recognize from image stream
			FrResult.nResults = 0;
			int frstatus = 0;
			rval = wfFR_RecognizeFaces(hFrhandle, ybuffer, width, height, width, &FrResult, &frstatus);
			if (rval != 0)
			{
				printf("Error in wfFR_RecognizeFaces\n");
				return rval;
			}

			printf("Face spoof detection %d, Num faces detected %d\n", frstatus, FrResult.nResults);

			for (int i = 0; i<FrResult.nResults; i++)
			{
				printf("Frame %d: face %d, Conf %.1f, size %d, ID %d\n", frFrameCounter, i,
					FrResult.pConfidence[i], FrResult.pFace[i].width, FrResult.pRecordID[i]);
			}

			frFrameCounter++;

#ifdef _WIN32
			long int curTime = GetTickCount();
			double elapsed = (curTime - startTime) / 1000;
#else
			timeval curTime;
			gettimeofday(&curTime, 0);
			double elapsed = (curTime.tv_sec - startTime.tv_sec);
#endif

			/// Finish recognition and release FR 
			if (elapsed > RECOGNITION_TIME)
			{
				printf("Recognition done. Release FR\n");
				wfFR_Release(&hFrhandle);
				hFrhandle = NULL;
				frhandleInitialized = 0;
				frEngineMode = 0;

				free(ybuffer);
				ybuffer = NULL;


			}
		}
	}
	
	internalfrMode = frEngineMode;
	return 0;
}

