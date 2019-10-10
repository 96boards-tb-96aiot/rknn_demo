#include <stdio.h>
#include <string>
#include <iostream>
#include <time.h>



#include <opencv/cv.h>
#include <opencv/highgui.h>


#include "FRLibrary.h"
#include "FRLibraryID.h"


int testEnrollFromVideo();


/* Number of maximum faces to be detected in the scene */
#define MAX_FACES (1)

/* Time in seconds for which ID check should be run. It can be in range of 2 to 20 seconds*/
#define IDCHECK_TIME (5) 

//Set base path accordingly for your application. Base path should have files like p0.bin, p1.bin etc and a folder named wffrdbpc.
//const char *pBasePath = "../frdata/";	
const char *pBasePathID = "";	

/* Sample code to check ID of a person. Initialize engine once using wfFRID_Init. 
Then for every ID first Enroll ID image using wfFRID_EnrollID.
Then call wfFRID_RecognizeFaces iteratively to match person with Enrolled ID.
*/
int testIDCheck()
{

	///////////// Read image from the file /////////////
	int width = 640;
	int height = 480;
	unsigned char* pixeldata = new unsigned char[width*height];
	FILE* fp = fopen("frtestimage.dat", "rb");
	if(fp == NULL)
	{
		printf("Error in opening file testimages/jen_aniston_003.dat\n");
		return -1;
	}
	fread(pixeldata, 1, width* height, fp);
	fclose(fp);

	void* hFR = NULL;

	

	/* Initialize the engine for ID check*/
	int rval = wfFRID_Init(&hFR, pBasePathID, 0);

	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}
	printf("PC Init done ID check\n");fflush(stdout);

	int enrollSuccess = 0;
	//rval = wfFRID_EnrollID(hFR,  (unsigned char*)(pixeldata), width,  height, width,	&enrollSuccess);
	char* imageNameID = "testimagesID/35080219920811251X_ID.jpg";
	rval = wfFRID_EnrollIDFromSavedImage(hFR,  imageNameID,	&enrollSuccess);
	if(rval != FR_OK || enrollSuccess == 0)
	{
		printf("Error in FR EnrollID, rval %d, enrollSuccess %d\n", rval, enrollSuccess);
		return 0;
	}
	printf("Enroll enrollSuccess %d\n", enrollSuccess);
	
	double startTime = clock(); 
	int frameCounter = 0;

	while(1)
	{
		FRIDList pResult;
		int frstatus = 0;

		/* Send image from stream for enrollment. */
		rval =  wfFRID_RecognizeFaces(hFR, (unsigned char*)(pixeldata), width,  height, width, &pResult, &frstatus);
		if(rval != FR_OK)
		{
			printf("Error in FR ID check %d\n", rval);
			//return 0;
		}
		printf("Num results %d\n", pResult.nResults);	// Number of detected Faces
		for(int i=0;i<pResult.nResults;i++)
		{
			printf("Frame %d: size %d, confidence %.1f, spoof %d\n",frameCounter , pResult.pFace[i].width,pResult.pConfidence[0], frstatus);
		}

		frameCounter++;
		double elaspedTime = (clock() - startTime) / (CLOCKS_PER_SEC );
		//double elaspedTime = (clock() - startTime) / CLOCKS_PER_SEC;
		if(elaspedTime > IDCHECK_TIME || frameCounter>20)	// end of enrolling is 10 seconds have passed.
			break;
		printf("Elapsed time %f\n\n", elaspedTime);

		if(frameCounter==15)
		{
			wfFRID_Release(&hFR);
			wfFRID_Init(&hFR, pBasePathID, 0);
		}
		if(frameCounter>10)
			memset(pixeldata, 0, width*height);
	}

	// Release FR engine after enrolling is done.
	wfFRID_Release(&hFR);

	delete [] pixeldata;
	return 1;
}



int testIDCheckFromImageFile()
{

	const char* imageNameID = "testimagesID/35080219920811251X_ID.jpg";
	const char* imageName = "testimagesID/35082319831011341X.jpg";

	void* hFR = NULL;

	/* Initialize the engine for ID check*/
	int rval = wfFRID_Init(&hFR, pBasePathID);

	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}
	printf("PC Init done ID check\n");fflush(stdout);

	wfFRID_setRecognitionThreshold(10);
	//wfFRID_setHighResolutionMode(1);

	int enrollSuccess = 0;
	rval = wfFRID_EnrollIDFromSavedImage(hFR,  imageNameID,	&enrollSuccess);
	if(rval != FR_OK || enrollSuccess == 0)
	{
		printf("Error in FR EnrollID, rval %d, enrollSuccess %d\n", rval, enrollSuccess);
		return 0;
	}
	printf("enrollSuccess %d\n", enrollSuccess);
	
	double startTime = clock(); 
	int frameCounter = 0;

	while(1)
	{
		FRIDList pResult;
		int frstatus = 0;

		/* Send image from stream for enrollment. */
		rval =  wfFRID_RecognizeFacesFromSavedImage(hFR, imageName, &pResult, &frstatus);
		if(rval != FR_OK)
		{
			printf("Error in FR ID check\n");
			//return 0;
		}
		printf("Num results %d\n", pResult.nResults);	// Number of detected Faces
		for(int i=0;i<pResult.nResults;i++)
		{
			printf("Frame %d: size %d, confidence %.1f\n",frameCounter++ , pResult.pFace[i].width,pResult.pConfidence[0]);
		}


		frameCounter++;
		if(frameCounter > 10)
			break;
		
	}
	
	// Release FR engine
	wfFRID_Release(&hFR);

	return 1;
}


int testIDCheckFromJpegBuffer()
{
	const char* imageNameID = "testimagesID/35080219920811251X_ID.jpg";
	const char* imageName = "testimagesID/35080219920811251X.jpg";

	FILE* fpid = fopen(imageNameID,"rb");
	if(fpid==NULL)
	{
		printf("Error in reading file\n");
		return 0;
	}
	fseek (fpid , 0 , SEEK_END);
	int szid = ftell (fpid);
	rewind (fpid);
	char* jpegBufferID = new char[szid];
	fread (jpegBufferID, 1, szid, fpid);
	fclose (fpid);


	FILE* fp = fopen(imageName,"rb");
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

	void* hFR = NULL;

	/* Initialize the engine for ID check*/
	int rval = wfFRID_Init(&hFR, pBasePathID);

	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}
	printf("PC Init done ID check\n");fflush(stdout);

	//wfFRID_setHighResolutionMode(1);
	//wfFRID_setRecognitionThreshold(45);

	int enrollSuccess = 0;
	rval = wfFRID_EnrollIDFromJpegBuffer(hFR,  jpegBufferID, szid,	&enrollSuccess);
	if(rval != FR_OK || enrollSuccess == 0)
	{
		printf("Error in FR EnrollID, rval %d, enrollSuccess %d\n", rval, enrollSuccess);
		return 0;
	}
	printf("ID enrollSuccess %d\n", enrollSuccess);

	wfFRID_VerifyImageForEnrollJpegBuffer(hFR, jpegBufferID, szid, 0, &enrollSuccess);
	printf("ID Verify %d\n", enrollSuccess);
	
	double startTime = clock(); 
	int frameCounter = 0;

	while(1)
	{
		FRIDList pResult;
		int frstatus = 0;

		//wfFRID_VerifyImageForEnrollJpegBuffer(hFR, jpegBuffer, sz, &enrollSuccess);
		//printf("enrollSuccess %d\n", enrollSuccess);

		
		/* Send image from stream for enrollment. */
		rval =  wfFRID_RecognizeFacesFromJpegBuffer(hFR, jpegBuffer, sz, &pResult, &frstatus);
		if(rval != FR_OK)
		{
			printf("Error in FR ID check\n");
			//return 0;
		}
		printf("Num results %d\n", pResult.nResults);	// Number of detected Faces
		for(int i=0;i<pResult.nResults;i++)
		{
			printf("Frame %d: size %d, confidence %.1f\n",frameCounter++ , pResult.pFace[i].width,pResult.pConfidence[0]);
		}

		double elaspedTime = (clock() - startTime) / (CLOCKS_PER_SEC );
		//double elaspedTime = (clock() - startTime) / CLOCKS_PER_SEC;
		if(elaspedTime > IDCHECK_TIME)	// end of enrolling is 10 seconds have passed.
			break;
		printf("Elapsed time %f\n", elaspedTime);
	}


	// Release FR engine
	wfFRID_Release(&hFR);

	return 1;
}



int main7()
{
	//testIDCheck();
	testIDCheckFromImageFile();
	//testIDCheckFromJpegBuffer();

	//testEnrollFromVideo();

}
