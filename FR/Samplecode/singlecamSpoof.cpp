
#include <stdio.h>
#include <string>
#include <iostream>
#ifdef _LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#include <Windows.h>
#endif

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "FRLibrary.h"
#include "FRLibraryID.h"

using namespace cv;


void checkMemory();
void ConvertBGR_TO_YUV420SP(const IplImage* pBGR, IplImage* yuv420, int outw, int outh);

void drawSinglecamspoof(Mat imgMat,  FRIDList pResult, int frstatus, int mode) //mode: 0 rec, 1 enroll
{
	Scalar color = Scalar(0,0,255);
	CvFont font;
	cvInitFont(&font, 0, 1.2, 1.2, 0, 3);
	if(frstatus ==1)
	{
		if(imgMat.channels() == 1)
			color = Scalar(255);
		else
		{
			if(mode == 0)
				color = Scalar(0,255,255);
			else
				color = Scalar(255,0,0);
		}
		
	}

	if(pResult.nResults>0)
	{
		if(pResult.pConfidence[0] > 0)
		{
			printf("Matched conf %.1f, name %s %s\n", pResult.pConfidence[0],  pResult.pFName[0], pResult.pLName[0]);
			color = Scalar(0,255,0);
		}

		int w = pResult.pFace[0].width;
		int h = pResult.pFace[0].height;
		int x = pResult.pFace[0].x;
		int y = pResult.pFace[0].y;
		rectangle(imgMat,  cvPoint(x, y), cvPoint(x + w, y + h), color, 3);

		char pBuff[255];
		sprintf(pBuff, "%.3f, %d",pResult.pValues[0][0], pResult.pTrackID[0]);
		IplImage imgMatIpl = imgMat;
		cvPutText(&imgMatIpl, pBuff, cvPoint(x, y+h+40), &font, color);
		
	}
}


int testsinglecamfrEnrollCamera(int camid)
{		
	cv::VideoCapture vcap;
	vcap.open(camid);
	if (vcap.isOpened())
	{
		//vcap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
		//vcap.set(CV_CAP_PROP_FRAME_HEIGHT, 960);
	}
	else
	{
		printf("Error in capturing from camera ID %d\n", camid);
		return 0;
	}

	
	Mat imgMat;
	vcap.read(imgMat);
	if (imgMat.empty())
	{
		printf("Failed to capture image %d\n", camid);
		return 0;
	}
	else
		printf("Capture %d started %d %d\n", camid, imgMat.cols, imgMat.rows);
#ifdef _WIN32
	Sleep(1000);
#else
	sleep(1);
#endif

	wfFR_setMinFaceDetectionSizePercent(30);
	void* hFR = NULL;
	char pBasePath[255] = { 0 };
	int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_ENROLL, 0);
	if (rval != FR_OK || hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}

	/* Add record in database to enroll new person. Use this record ID in Enroll API */
	unsigned long recordID = ULONG_MAX;

	rval =  wfFR_AddRecord(hFR, &recordID, "Jen", "Aniston");
	if(rval != FR_OK)
	{
		printf("Error in FR add record %d, recordID %d\n", rval, (int)recordID);
		return 0;
	}

	
	cv::Mat imgMatGray;
	cvtColor(imgMat, imgMatGray, CV_BGR2GRAY);
	
	IplImage framegray1 = imgMatGray;
	
	int frameCounter = 0;
	int saveCounter = 0;
	while(1)
	{

		vcap.read(imgMat);
		if (imgMat.empty())
		{
			printf("Capture end 1\n");
			break;
		}

		cvtColor(imgMat, imgMatGray, CV_BGR2GRAY);

		FRIDList pResult1;
		int frstatus1;

		wfFR_EnrollFaceSingleCamSpoof(hFR, (unsigned char*)(framegray1.imageData), framegray1.width, framegray1.height, framegray1.widthStep, 
						recordID, &pResult1, &frstatus1);
		

		printf("Detection %d,  frstatus1 %d\n\n", pResult1.nResults, frstatus1);
		drawSinglecamspoof(imgMat, pResult1, frstatus1, 1);
		imshow("imageColor", imgMat);
	
		char c = waitKey(200);
		//if(pResult1.nResults > 0 && frstatus1 == 1)
		//	waitKey(0);
		if(c == 'q')
			break;
		else if(c == 's')
		{
			char oname[255];
			sprintf(oname, "images//image_%d_0.jpg",saveCounter);
			imwrite(oname, imgMat);

			printf("Saved %d\n", saveCounter);
			saveCounter++;
		}
		frameCounter++;

		if(frameCounter > 50)
			break;
	}

	wfFR_Release(&hFR);
	cvDestroyAllWindows();
	vcap.release();

	return 1;
}

int testsinglecamfrCamera(int camid)
{		
	printf("%s\n", wfFR_GetVersionInfo());
	wfFR_setVerbose("", 0);	

	cv::VideoCapture vcap;
	vcap.open(camid);
	if (vcap.isOpened())
	{
		vcap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
		vcap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
	}
	else
	{
		printf("Error in capturing from camera ID %d\n", camid);
		return 0;
	}

	
	Mat imgMat;
	vcap.read(imgMat);
	if (imgMat.empty())
	{
		printf("Failed to capture image %d\n", camid);
		return 0;
	}
	else
		printf("Capture %d started %d %d\n", camid, imgMat.cols, imgMat.rows);
#ifdef _WIN32
	Sleep(1000);
#else
	sleep(1);
#endif

	//wfFR_setSingleCamSpoofThreshold(-15.0f);
	wfFR_setMinFaceDetectionSizePercent(10);
	//wfFR_enableImageSaveForDebugging(1);
	//wfFR_setSpoofingSensitivity(NULL, 3);
	//wfFR_setDetectionAlgoType(1);

	//FRMiscParams mparams;
	//wfFR_getMiscParameters(&mparams);
	//mparams.enableSinglecamAntiSpoofBlock = 1;
	//wfFR_setMiscParameters(mparams);

	//wfFR_setRecogQualityCheckFlag(1);
	//wfFR_setDetectionOnlyMode(1);
	//wfFR_setSaveDetectedFaceFlag(2, NULL);

	wfFR_setInputImageBufferFormat(IMAGE_YUV420);

	void* hFR = NULL;
	char pBasePath[255] = { 0 };
	int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_RECOGNIZE, 0);
	if (rval != FR_OK || hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}
	
	cv::Mat imgMatGray;
	cvtColor(imgMat, imgMatGray, CV_BGR2GRAY);

	IplImage framegray1 = imgMatGray;

	int frameCounter = 0;
	int saveCounter = 0;
	while(1)
	{
		vcap.read(imgMat);
		if (imgMat.empty())
		{
			printf("Capture end 1\n");
			break;
		}
		//imwrite("singlecamtest2.jpg", imgMat);
		//imgMat = imread("singlecamtest.jpg");

		cvtColor(imgMat, imgMatGray, CV_BGR2GRAY);
	
		IplImage frameColor = imgMat;
		IplImage *yuv420 = cvCreateImage(cvSize(frameColor.width, frameColor.height*1.5f), 8,1);
		ConvertBGR_TO_YUV420SP(&frameColor, yuv420, frameColor.width, frameColor.height);
		//cvSaveImage("input.jpg", &frameColor);

		FRIDList pResult1;
		int frstatus1;

		
		//cvSaveImage("debugimages/spoof13.png", &framegray1);
		rval = wfFR_RecognizeFacesSingleCamSpoof(hFR, (unsigned char*)(yuv420->imageData), yuv420->width, frameColor.height, yuv420->widthStep, &pResult1, &frstatus1);

		//rval = wfFR_RecognizeFaces(hFR, (unsigned char*)(yuv420->imageData), yuv420->width, frameColor.height, yuv420->widthStep, &pResult1, &frstatus1);
		//rval = wfFR_DetectRecognizeSingleCamSpoofMultiThread(hFR, (unsigned char*)(yuv420->imageData), yuv420->width, frameColor.height, yuv420->widthStep, &pResult1, &frstatus1);
		
		if(pResult1.nResults > 0) 
			printf("Frame %d: spoof %d, conf %.1f, value %.3f, width %d\n", frameCounter, frstatus1, pResult1.pConfidence[0], pResult1.pValues[0][0], pResult1.pFace[0].width);

		if (rval != FR_OK)
		{
			printf("Error in wfFR_RecognizeFacesSingleCamSpoof %d\n", rval);
			break;
		}

		if(0) //(pResult1.nResults > 0 && frstatus1 == 1)
		{
			char oname[255];
			sprintf(oname, "singlecamimages_videotest/image_%d_0.jpg",saveCounter);
			imwrite(oname, imgMat);
			printf("Saved %d\n", saveCounter);
			saveCounter++;
		}

		//printf("after rec API: "); checkMemory();
		//printf("Detection %d, %d\n\n", pResult1.nResults, pResult2.nResults);
		drawSinglecamspoof(imgMat,  pResult1, frstatus1, 0);
		namedWindow("imageColor", 0);
		imshow("imageColor", imgMat);
		
		char c = waitKey(10);
		if(pResult1.nResults > 0 && frstatus1 == 1)
		//if(pResult1.nResults > 0 && pResult1.pConfidence[0] > 0)
		{
			waitKey(10);
		}

		if(c == 'q')
			break;
		else if(c == 's')
		{
			char oname[255];
			sprintf(oname, "images//image_%d_0.jpg",saveCounter);
			imwrite(oname, imgMat);


			printf("Saved %d\n", saveCounter);
			saveCounter++;
		}
		frameCounter++;

		cvReleaseImage(&yuv420);
		//if(pResult1.nResults > 0) printf("\n\n");

		//char pImageName[2048];
		//wfFR_getSaveDetectedImageName(pImageName);
		//printf("Saved image %s\n", pImageName);
	}

	wfFR_Release(&hFR);
	cvDestroyAllWindows();
	vcap.release();

	return 1;
}

void testsinglecamfrImage()
{
	//wfFR_setDetectionAlgoType(0);
	wfFR_setMinFaceDetectionSizePercent(10);
	wfFR_setVerbose("", 0);
	wfFR_setSingleCamSpoofThreshold(-0.5f);
	for(int m=0;m<1;m++)
	{
		char name1[1024];
		
		sprintf(name1, "testimages/jen_aniston_004_small.jpg");
		//sprintf(name1, "testimages/group6.jpg");
		
		void* hFR = NULL;
		char pBasePath[255] = { 0 };
		//char* pBasePath = "newbasepath/";
		int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_RECOGNIZE, 0);
		if (rval != FR_OK || hFR == NULL)
		{
			printf("Error in FR init\n");
			return;
		}
		FRIDList pResult1;
		int frstatus1;
		for(int i=0;i<20;i++)
		{
			IplImage* framecolor = cvLoadImage(name1);
			
			if(framecolor == NULL)
			{
				printf("Error in loading images\n");
				return;
			}
			IplImage *yuv420 = cvCreateImage(cvSize(framecolor->width, framecolor->height*1.5f), 8,1);
			ConvertBGR_TO_YUV420SP(framecolor, yuv420, framecolor->width, framecolor->height);

			pResult1.nResults = 0;
			printf("before recognize H %d, %d\n", framecolor->height, yuv420->height);
			frstatus1 = 0;
			double t0 = cvGetTickCount();
			wfFR_RecognizeFacesSingleCamSpoof(hFR, (unsigned char*)(yuv420->imageData), framecolor->width, framecolor->height,  framecolor->width, &pResult1, &frstatus1);
			t0 = (cvGetTickCount() - t0) / (cvGetTickFrequency()*1000.0f);

			printf("Num faces single cam %d  spoof %d, Time %.1f\n", pResult1.nResults,  frstatus1, t0);
			if(pResult1.nResults >0)
				printf("Match conf %.1f\n", pResult1.pConfidence[0]);
		
			Mat imgMat = cvarrToMat(framecolor);
			
			//drawSinglecamspoof(imgMat,  pResult1, frstatus1,  0);
			//cvShowImage("color", framecolor);
			//cvWaitKey(0);	

			cvReleaseImage(&framecolor);
			cvReleaseImage(&yuv420);
			printf("\n\n");
		}

		wfFR_Release(&hFR);
	}

	
}


void testsinglecamfrjpegbuf()
{
	wfFR_setDetectionAlgoType(0);
	wfFR_setMinFaceDetectionSizePercent(10);
	//FRMiscParams mparams;
	//wfFR_getMiscParameters(&mparams);
	//mparams.enableSinglecamBGReject = 1;
	//wfFR_setMiscParameters(mparams);

	//wfFR_enableImageSaveForDebugging(1);
	wfFR_setVerbose("", 1);
	
	for(int m=0;m<1;m++)
	{
		char name1[1024];
		
		//sprintf(name1, "testimages/jen_aniston_004.jpg");
		sprintf(name1, "testimages/group6.jpg");
		
		void* hFR = NULL;
		char pBasePath[255] = { 0 };
		int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_RECOGNIZE, 0);
		if (rval != FR_OK || hFR == NULL)
		{
			printf("Error in FR init\n");
			return;
		}

		FILE* fp = fopen(name1,"rb");
		if(fp==NULL)
		{
			printf("Error in reading file\n");
			return;
		}
		fseek (fp , 0 , SEEK_END);
		int sz = ftell (fp);
		rewind (fp);
		char* jpegBuffer = new char[sz];
		fread (jpegBuffer, 1, sz, fp);
		fclose (fp);

		FRIDList pResult;
		int frstatus1;
		for(int i=0;i<10;i++)
		{
			IplImage* frame1 = cvLoadImage(name1,1);
			
			if(frame1 == NULL)
			{
				printf("Error in loading images\n");
				return;
			}

			pResult.nResults = 0;
		
			frstatus1 = 0;
			double t1 = cvGetTickCount();
			wfFR_RecognizeFacesSingleCamSpoofFromJpegBuffer(hFR, jpegBuffer, sz, &pResult, &frstatus1);
			t1 = (cvGetTickCount() - t1) / (cvGetTickFrequency()*1000.0f);

			printf("Num faces single cam %d  spoof %d Time %.1f\n\n", pResult.nResults,  frstatus1, t1);
			
			Mat imgMat = cvarrToMat(frame1);
			
			//drawSinglecamspoof(imgMat,  pResult, frstatus1,  0);
			//cvShowImage("color", framegray1);
			//cvWaitKey(0);	

			cvReleaseImage(&frame1);

		}

		wfFR_Release(&hFR);
	}

	
}


void testsinglecamenrolljpegbuf()
{

	wfFR_setMinFaceDetectionSizePercent(5);

	
	for(int m=0;m<1;m++)
	{
		char name1[1024];
		
		sprintf(name1, "debugimages/170.jpg");
		
		void* hFR = NULL;
		char pBasePath[255] = { 0 };
		int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_ENROLL, 0);
		if (rval != FR_OK || hFR == NULL)
		{
			printf("Error in FR init\n");
			return;
		}

		/* Add record in database to enroll new person. Use this record ID in Enroll API */
		unsigned long recordID = ULONG_MAX;

		rval =  wfFR_AddRecord(hFR, &recordID, "Jen", "Aniston");
		if(rval != FR_OK)
		{
			printf("Error in FR add record %d, recordID %d\n", rval, (int)recordID);
			return;
		}


		FILE* fp = fopen(name1,"rb");
		if(fp==NULL)
		{
			printf("Error in reading file\n");
			return;
		}
		fseek (fp , 0 , SEEK_END);
		int sz = ftell (fp);
		rewind (fp);
		char* jpegBuffer = new char[sz];
		fread (jpegBuffer, 1, sz, fp);
		fclose (fp);

		FRIDList pResult1;
		int frstatus1;
		for(int i=0;i<10;i++)
		{
			IplImage* framegray1 = cvLoadImage(name1,0);
			
			if(framegray1 == NULL)
			{
				printf("Error in loading images\n");
				return;
			}

			pResult1.nResults = 0;
		
			frstatus1 = 0;
			//wfFR_EnrollFaceSingleCamSpoof(hFR, (unsigned char*)(framegray1->imageData), framegray1->width, framegray1->height, framegray1->widthStep, recordID, &pResult1, &frstatus1);
			wfFR_EnrollFaceSingleCamSpoofFromJpegBuffer(hFR, jpegBuffer, sz, recordID, &pResult1, &frstatus1);

			printf("Num faces single cam %d  spoof %d\n\n\n", pResult1.nResults,  frstatus1);
		
			Mat imgMat = cvarrToMat(framegray1);
			
			drawSinglecamspoof(imgMat,  pResult1, frstatus1,  0);
			cvShowImage("color", framegray1);
			
			cvWaitKey(10);	

			cvReleaseImage(&framegray1);

		}

		wfFR_Release(&hFR);
	}

	
}

int main12(int argc, char* argv[])
{
	int camid = 0;
		
	testsinglecamfrCamera(camid);
	//testsinglecamfrEnrollCamera(camid);

	//testsinglecamfrImage();

	//testsinglecamfrjpegbuf();
	//testsinglecamenrolljpegbuf();

	return 0;

}
