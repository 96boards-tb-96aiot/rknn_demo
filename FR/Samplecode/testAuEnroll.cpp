#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <time.h>

#ifdef _LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>	
#endif

#define USE_OPENCV (1)

#if USE_OPENCV
#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace cv;
#endif

#include "FRLibrary.h"

using namespace std;

/* Number of maximum faces to be detected in the scene */
#define MAX_FACES (1)

/* Time in seconds for which enrolling should be run. It can be in range of 5 to 20 seconds*/
#define ENROLLING_TIME (10000) 

#define USEWEBCAM (1)


void checkMemory();

/* To enroll from video, recognition must be stopped and FR handle must be release.
Then create new FR handle for enrolling. Once enrolling is done, release FR handle for enrolling.
Recongnition and Enrolling handle should NOT be active at any same moment. Only either one of them should be initialized.
Keep only one face in front while enrolling. Usually it takes about 10 second to enroll person.
Person can make small changes face poses like looking left, right, up, down by 15-20 degrees.
*/
int testEnrollFromVideo(string imagename)
{
	printf("Enroll start\n");
	checkMemory();
	
	//wfFR_setDeleteExistingNameInEnrolling(0);

	int width = 1280;
	int height = 720;
	int widthStep = 1280;
	unsigned char* pixval = NULL;
#if USE_OPENCV
	printf("Enrolling %s\n", imagename.c_str());
	IplImage* testImage = cvLoadImage(imagename.c_str(), 0);

	if(testImage == NULL)
	{
		printf("Error in loading image\n");
		return 0;
	}
	//cvZero(testImage);


	checkMemory();
	pixval = (unsigned char*)testImage->imageData;
	width = testImage->width;
	height = testImage->height;
	widthStep = testImage->widthStep;
#else
	pixval = new unsigned char[width*height];
	FILE* fp = fopen("testimages/jen_aniston_003.yuv", "rb");
	if(fp==NULL)
	{
		printf("Error in reading YUV file\n");
		delete [] pixval;
		return 0;
	}
	int readval = fread(pixval, width*height, 1, fp);
	fclose(fp);
		if(readval != 1)
		printf("Error in reading YUV file jen_aniston_003.yuv\n");
	else
		printf("YUV file jen_aniston_003.yuv read successfully\n");
#endif

	printf("Image W %d, H %d, S %d\n", width, height, widthStep);
	//wfFR_setDatabaseFolderPath("basepathdb/");

	//wfFR_setEnrollQualityCheckFlag(1);
	//wfFR_setVerbose("", 1);

	//FREnrollQualityParams enrollparams;
	//wfFR_getEnrollQualityParameters(&enrollparams);
	//enrollparams.maxBlur = 70;
	//wfFR_setEnrollQualityParameters(enrollparams);

	void* hFR = NULL;
	char pBasePath[255] = {0};

	wfFR_setMinFaceDetectionSizePercent(10);

	/* Initialize the engine for recognition. Give input width, height and widthstep for Y channel image.
	   pBasePath is the full path of the data files like p0.bin, p1tc.bin etc */
	double t0 = cvGetTickCount();
	int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_ENROLL, 0);
	t0 = (cvGetTickCount() - t0)/(1000.0*cvGetTickFrequency());
	checkMemory();

	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init for enrolling %d\n", rval);
		return 0;
	}
	printf("Init done Enrolling %.1f\n", t0);fflush(stdout);

	/* Add record in database to enroll new person. Use this record ID in Enroll API */
	unsigned long recordID = ULONG_MAX;

	rval =  wfFR_AddRecord(hFR, &recordID, "Jen", "Aniston");
	if(rval != FR_OK)
	{
		printf("Error in FR add record %d, recordID %d\n", rval, (int)recordID);
		return 0;
	}

	int iForce = 0;

	//wfFR_saveEnrollImages(hFR, 0);

	//double startTime = clock(); //
	//double startTime =cvGetTickCount();
	int frameCounter = 0;

	while(1)
	{
		FRIDList pResult;
		pResult.nResults = 0;
		int frstatus = 0;
		//checkMemory();
		double t1 = cvGetTickCount();

		/* Send image from stream for enrollment. Use recordID from AddRecord API. */
		rval =  wfFR_EnrollFace(hFR, pixval, width, height, widthStep, recordID, iForce, &pResult, &frstatus);
		t1 = (cvGetTickCount() - t1)/(1000.0*cvGetTickFrequency());
		if(rval != FR_OK)
		{
			printf("Error in FR enroll face\n");
			break;
		}
		
		printf("Num results %d\n", pResult.nResults);	// Number of detected Faces
		for(int i=0;i<pResult.nResults;i++)
		{
			printf("Frame %d: size %d, conf %.1f\n",frameCounter++ , pResult.pFace[i].width, pResult.pConfidence[i]);
		}
		//checkMemory();

#ifdef _LINUX
		//if(t1 < 100)
		//	usleep(100000 - t1*1000); // sleep is only for testing on PC, no sleep should be used on device
#endif

		//double elaspedTime = (cvGetTickCount() - startTime) / (1000000.0*cvGetTickFrequency());
		//double elaspedTime = (clock() - startTime) / CLOCKS_PER_SEC;
		//if(elaspedTime > ENROLLING_TIME)	// end of enrolling is 10 seconds have passed.
		if(frameCounter++ > 5)
			break;
		//printf("Elapsed time %f\n", elaspedTime);
	}
	
	double t3 = cvGetTickCount();
	wfFR_Release(&hFR);
	t3 = (cvGetTickCount() - t3)/(1000.0*cvGetTickFrequency());
#if USE_OPENCV
	cvReleaseImage(&testImage);
#else
	delete [] pixval;
#endif
	checkMemory();

	printf("Enroll ends video, t3 %.1f\n", t3);
	return 1;
}


void ConvertBGR_TO_YUV420SP_Rec(const IplImage* pBGR, IplImage* yuv420, int outw, int outh);
int testEnrollFromVideoNV21(string imagename)
{
	printf("Enroll start\n");
	checkMemory();
	
	//wfFR_setDeleteExistingNameInEnrolling(0);

	int width = 1280;
	int height = 720;
	int widthStep = 1280;
	unsigned char* pixval = NULL;

	printf("Enrolling %s\n", imagename.c_str());
	IplImage* testImageColor = cvLoadImage(imagename.c_str());

	if(testImageColor == NULL)
	{
		printf("Error in loading image\n");
		return 0;
	}

	void* hFR = NULL;
	char pBasePath[255] = {0};

	wfFR_setMinFaceDetectionSizePercent(10);

	/* Initialize the engine for recognition. Give input width, height and widthstep for Y channel image.
	   pBasePath is the full path of the data files like p0.bin, p1tc.bin etc */
	double t0 = cvGetTickCount();
	int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_ENROLL, 0);
	t0 = (cvGetTickCount() - t0)/(1000.0*cvGetTickFrequency());
	checkMemory();

	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init for enrolling %d\n", rval);
		return 0;
	}
	printf("Init done Enrolling %.1f\n", t0);fflush(stdout);

	/* Add record in database to enroll new person. Use this record ID in Enroll API */
	unsigned long recordID = ULONG_MAX;

	rval =  wfFR_AddRecord(hFR, &recordID, "Jen", "Aniston");
	if(rval != FR_OK)
	{
		printf("Error in FR add record %d, recordID %d\n", rval, (int)recordID);
		return 0;
	}

	int iForce = 0;
	int frameCounter = 0;

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

	
	while(1)
	{
		FRIDList pResult;
		pResult.nResults = 0;
		int frstatus = 0;
		//checkMemory();
		double t1 = cvGetTickCount();


		/* Send image from stream for enrollment. Use recordID from AddRecord API. */
		rval =  wfFR_EnrollFace(hFR, (unsigned char*)(yuv420->imageData), imgRoi.width, imgRoi.height, imgRoi.width, recordID, iForce, &pResult, &frstatus);
		t1 = (cvGetTickCount() - t1)/(1000.0*cvGetTickFrequency());
		
		if(rval != FR_OK)
		{
			printf("Error in FR enroll face\n");
			break;
		}
		
		printf("Num results %d\n", pResult.nResults);	// Number of detected Faces
		for(int i=0;i<pResult.nResults;i++)
		{
			printf("Frame %d: size %d, conf %.1f\n",frameCounter++ , pResult.pFace[i].width, pResult.pConfidence[i]);
		}
		//checkMemory();

#ifdef _LINUX
		//if(t1 < 100)
		//	usleep(100000 - t1*1000); // sleep is only for testing on PC, no sleep should be used on device
#endif

		//double elaspedTime = (cvGetTickCount() - startTime) / (1000000.0*cvGetTickFrequency());
		//double elaspedTime = (clock() - startTime) / CLOCKS_PER_SEC;
		//if(elaspedTime > ENROLLING_TIME)	// end of enrolling is 10 seconds have passed.
		if(frameCounter++ > 5)
			break;
		//printf("Elapsed time %f\n", elaspedTime);
	}
	
	double t3 = cvGetTickCount();
	wfFR_Release(&hFR);
	t3 = (cvGetTickCount() - t3)/(1000.0*cvGetTickFrequency());
#if USE_OPENCV
	cvReleaseImage(&testImageColor);
#else
	delete [] pixval;
#endif
	checkMemory();

	printf("Enroll ends video, t3 %.1f\n", t3);
	return 1;
}


int testEnrollFromWecam()
{
	//wfFR_setDeleteExistingNameInEnrolling(1);
#if USEWEBCAM
	printf("Enroll start\n");

	//checkMemory();

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

	wfFR_setVerbose("", 1);
	wfFR_setEnrollQualityCheckFlag(1);

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

	//checkMemory();

	void* hFR = NULL;
	char pBasePath[255] = {0};

	/* Initialize the engine for recognition. Give input width, height and widthstep for Y channel image.
	   pBasePath is the full path of the data files like cp.bin, mpE.dat etc */
	int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_ENROLL, 0);
	//checkMemory();

	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}
	printf("Init done\n");

	FREnrollQualityParams mparams;
	wfFR_getEnrollQualityParameters(&mparams);
	mparams.minFaceAvg = 40;
	wfFR_setEnrollQualityParameters(mparams);
	
	/* Add record in database to enroll new person. Use this record ID in Enroll API */
	unsigned long recordID = ULONG_MAX;
	rval =  wfFR_AddRecord(hFR, &recordID, "Jen", "Aniston");
	if(rval != FR_OK)
	{
		printf("Error in FR add record %d, recordID %d\n", rval, (int)recordID);
		//return 0;
	}

	int iForce = 0;

	//double startTime = clock(); //
	double startTime =cvGetTickCount();
	int frameCounter = 0;

	while(1)
	{
		vcap.read(imgMatColor);
		if (imgMatColor.empty())
		{
			printf("Capture end\n");
			break;
		}
		cvtColor(imgMatColor, imgMat, CV_BGR2GRAY);

		FRIDList pResult;
		int frstatus = 0;

		double t1 = cvGetTickCount();
		/* Send image from stream for enrollment. Use recordID from AddRecord API. */
		rval =  wfFR_EnrollFace(hFR, (unsigned char*)(testImage->imageData), testImage->width, testImage->height, testImage->widthStep, recordID, iForce, &pResult, &frstatus);
		//rval =  wfFR_EnrollFaceSingleCamSpoof(hFR, (unsigned char*)(testImage->imageData), testImage->width, testImage->height, testImage->widthStep, recordID, &pResult, &frstatus);

		t1 = (cvGetTickCount() - t1)/(1000.0*cvGetTickFrequency());
		if(rval != FR_OK)
		{
			printf("Error in FR enroll face\n");
			//return 0;
		}
		
		int enrollFailcode = 0;
		wfFR_getEnrollFailureCode(&enrollFailcode);
		printf("enrollFailcode %d\n", enrollFailcode);
		
		//checkMemory();
		printf("Num results %d, frstatus %d, Time %.1f\n", pResult.nResults, frstatus, t1);	// Number of detected Faces
		for(int i=0;i<pResult.nResults;i++)
		{
			printf("Frame %d: size %d\n",frameCounter++ , pResult.pFace[i].width);
		}

		CvFont font;
		cvInitFont(&font, 0, 1.2, 1.2, 0, 3);
		for(int i=0;i<pResult.nResults;i++)
		{
			int w = pResult.pFace[i].width;
			int h = pResult.pFace[i].height;
			int x = pResult.pFace[i].x;
			int y = pResult.pFace[i].y;
			CvScalar color = cvScalar(255, 0, 0);
			printf("confidence %.1f\n", pResult.pConfidence[0]);
			if(frstatus == 0 || pResult.pConfidence[0] < 0)
				color = cvScalar(0, 255, 255);
			cvRectangle(testImage, cvPoint(x, y), cvPoint(x + w, y + h), color, 2);
		}
		cvShowImage("Display", testImage);
		char c = cvWaitKey(0);
		if(c == 'q')
		{
			break;
		}


		
		//if(t1 < 100)
		//	usleep(100000 - t1*1000); // sleep is only for testing on PC, no sleep should be used on device

		double elaspedTime = (cvGetTickCount() - startTime) / (1000000.0*cvGetTickFrequency());
		//double elaspedTime = (clock() - startTime) / CLOCKS_PER_SEC;
		if(elaspedTime > ENROLLING_TIME)	// end of enrolling is 10 seconds have passed.
			break;
		printf("Elapsed time %f\n\n", elaspedTime);


	}
	
	vcap.release();
	wfFR_Release(&hFR);
	//checkMemory();
	printf("Enroll ends cam\n");
#endif
}


/* To run enroll from saved image, initialize FR handle in enrolling mode.
Send images name to API wfFR_EnrollFaceFromSavedImage to run enrolling.
*/
int testEnrollFromSaveImage()
{		
	printf("Enroll from Saved image start\n");
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

	/* Add record in database to enroll new person. Use this record ID in Enroll API */
	unsigned long recordID = ULONG_MAX;
	rval =  wfFR_AddRecord(hFR, &recordID, "Jen", "Aniston");
	if(rval != FR_OK)
	{
		printf("Error in FR add record %d, recordID %d\n", rval, (int)recordID);
		//return 0;
	}


	//const char* pImageFilename = "testimages/jen_aniston_003.jpg";
const char* pImageFilename = "sfirm/2019.08.14_enroll/sfirm_100335_1 (3).jpg";

	FRIDList pResult;
	int frstatus = 0;
	double t1 = 0;
	//t1 = cvGetTickCount();
	rval = wfFR_EnrollFaceFromSavedImage(hFR, pImageFilename, recordID, &pResult, &frstatus);
	
	//t1 = (cvGetTickCount() - t1)/(1000.0*cvGetTickFrequency());

	if(rval != FR_OK)
	{
		printf("Error in FR enroll face\n");
		//return 0;
	}
	//checkMemory();
	printf("Num results %d, Time %.1f\n", pResult.nResults, t1);	// Number of detected Faces
	for(int i=0;i<pResult.nResults;i++)
	{
		printf("Face %d: size %d, conf %.1f\n",i, pResult.pFace[i].width,  pResult.pConfidence[i]);
	}

	/* Release FR handle */
	wfFR_Release(&hFR);
}


/* To run enroll from jpeg buffer, initialize FR handle in enrolling mode.
Send images name to API wfFR_EnrollFaceFromSavedImage to run enrolling.
*/
int testEnrollFromJpegBuffer()
{		
	printf("Recognize from Saved image start\n");
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

	/* Add record in database to enroll new person. Use this record ID in Enroll API */
	unsigned long recordID = ULONG_MAX;
	rval =  wfFR_AddRecord(hFR, &recordID, "Jen", "Aniston");
	if(rval != FR_OK)
	{
		printf("Error in FR add record %d, recordID %d\n", rval, (int)recordID);
		//return 0;
	}


	const char* pImageFilename = "testimages/jen_aniston_003.jpg";

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

	FRIDList pResult;
	int frstatus = 0;
	double t1 = 0;
	//t1 = cvGetTickCount();
	rval = wfFR_EnrollFaceFromJpegBuffer(hFR, jpegBuffer, sz, recordID, &pResult, &frstatus);
	
	//t1 = (cvGetTickCount() - t1)/(1000.0*cvGetTickFrequency());

	if(rval != FR_OK)
	{
		printf("Error in FR enroll face\n");
		//return 0;
	}
	//checkMemory();
	printf("Num results %d, Time %.1f\n", pResult.nResults, t1);	// Number of detected Faces
	for(int i=0;i<pResult.nResults;i++)
	{
		printf("Face %d: size %d\n",i, pResult.pFace[i].width);
	}

	delete [] jpegBuffer;	

	/* Release FR handle */
	wfFR_Release(&hFR);
}

int main4()
{
	//testEnrollFromVideo();
	return 0;
}

