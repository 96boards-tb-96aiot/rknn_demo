
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

#define ENABLE_SECOND (1)

void checkMemory();

void ConvertBGR_TO_YUV420SP_Rec(const IplImage* pBGR, IplImage* yuv420, int outw, int outh);

void drawDualspoof(Mat imgMat, Mat imgMat2, FRIDList pResult1, int frstatus1, FRIDList pResult2, int mode) //mode: 0 rec, 1 enroll
{
	CvFont font;
	cvInitFont(&font, 0, 1.0, 1.0, 0, 2);
	Scalar color = Scalar(0,0,255);
	if(frstatus1 ==1)
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

	if(pResult1.nResults>0 && pResult2.nResults>0)
	{
		
		int w1 = pResult1.pFace[0].width;
		int h1 = pResult1.pFace[0].height;
		int x1 = pResult1.pFace[0].x;
		int y1 = pResult1.pFace[0].y;

		
		int w2 = pResult2.pFace[0].width;
		int h2 = pResult2.pFace[0].height;
		int x2 = pResult2.pFace[0].x;
		int y2 = pResult2.pFace[0].y;
		if(pResult1.pConfidence[0] > 0)
		{
			//printf("Matched conf %.1f\n", pResult1.pConfidence[0]);
			color = Scalar(0,255,0);

			char pBuff[256];
			sprintf(pBuff, "%s %s: %.1f", pResult1.pFName[0], pResult1.pLName[0], pResult1.pConfidence[0]);
			IplImage iplimage = imgMat;
			IplImage iplimage2 = imgMat2;
			cvPutText(&iplimage, pBuff, cvPoint(x1, y1 - 20), &font, color);
			cvPutText(&iplimage2, pBuff, cvPoint(x1, y1 - 20), &font, color);
		}

		rectangle(imgMat,  cvPoint(x1, y1), cvPoint(x1 + w1, y1 + h1), color, 3);
		rectangle(imgMat2,  cvPoint(x2, y2), cvPoint(x2 + w2, y2 + h2), color, 3);
	}
}


int testdualcamfrEnrollCamera(int camid, int camid2, string fname)
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


	cv::VideoCapture vcap2;
	vcap2.open(camid2);
	if (vcap2.isOpened())
	{
		//vcap2.set(CV_CAP_PROP_FRAME_WIDTH, 640);
		//vcap2.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	}
	else
	{
		printf("Error in capturing from camera 2 ID %d\n", camid2);
		return 0;
	}

	Mat imgMat2;
	vcap2.read(imgMat2);
	if (imgMat2.empty())
	{
		printf("Failed to capture image %d\n", camid2);
		return 0;
	}
	else
		printf("Capture %d started %d %d\n", camid2, imgMat.cols, imgMat.rows);


	wfFR_setMinFaceDetectionSizePercent(10);
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

	rval =  wfFR_AddRecord(hFR, &recordID, fname.c_str(), "");
	if(rval != FR_OK)
	{
		printf("Error in FR add record %d, recordID %d\n", rval, (int)recordID);
		return 0;
	}

	
	cv::Mat imgMatGray, imgMatGray2;
	cvtColor(imgMat, imgMatGray, CV_BGR2GRAY);
	cvtColor(imgMat2, imgMatGray2, CV_BGR2GRAY);

	IplImage framegray1 = imgMatGray;
	IplImage framegray2 = imgMatGray2;

	int frameCounter = 0;
	int saveCounter = 0;
	printf("Enrolling Started for %s\n", fname.c_str());
	while(1)
	{

		vcap.read(imgMat);
		if (imgMat.empty())
		{
			printf("Capture end 1\n");
			break;
		}

		vcap2.read(imgMat2);
		if (imgMat2.empty())
		{
			printf("Capture end 2\n");
			break;
		}
		

		cvtColor(imgMat, imgMatGray, CV_BGR2GRAY);
		cvtColor(imgMat2, imgMatGray2, CV_BGR2GRAY);

		FRIDList pResult1;
		int frstatus1;
		FRIDList pResult2;
		

		wfFR_EnrollFaceDualCam(hFR, (unsigned char*)(framegray1.imageData), (unsigned char*)(framegray2.imageData), framegray2.width, framegray2.height, framegray2.widthStep, 
						recordID, &pResult1, &pResult2, &frstatus1);
	
		//printf("Detection %d, %d, frstatus1 %d\n\n", pResult1.nResults, pResult2.nResults, frstatus1);
		drawDualspoof(imgMat, imgMat2, pResult1, frstatus1, pResult2, 1);
		imshow("Color", imgMat);
		imshow("IR", imgMat2);
		char c = waitKey(10);
		//if(pResult1.nResults > 0 && frstatus1 == 1)
		//	waitKey(0);
		if(c == 'q')
			break;
		else if(c == 's')
		{
			char oname[255];
			sprintf(oname, "images//image_%d_0.jpg",saveCounter);
			imwrite(oname, imgMat);
#if ENABLE_SECOND
			sprintf(oname, "images//image_%d_1.jpg",saveCounter);
			imwrite(oname, imgMat2);
#endif

			printf("Saved %d\n", saveCounter);
			saveCounter++;
		}
		frameCounter++;

		if(frameCounter > 100)
			break;
	}

	wfFR_Release(&hFR);
	cvDestroyAllWindows();
	vcap.release();
#if ENABLE_SECOND
	vcap2.release();
#endif
	printf("Enrolling Ends\n");
	return 1;
}

int testdualcamfrCamera(int camid, int camid2)
{		
	wfFR_setVerbose("", 1);
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


	cv::VideoCapture vcap2;
	vcap2.open(camid2);
	if (vcap2.isOpened())
	{
		//vcap2.set(CV_CAP_PROP_FRAME_WIDTH, 640);
		//vcap2.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	}
	else
	{
		printf("Error in capturing from camera 2 ID %d\n", camid2);
		return 0;
	}

	Mat imgMat2;
	vcap2.read(imgMat2);
	if (imgMat2.empty())
	{
		printf("Failed to capture image %d\n", camid2);
		return 0;
	}
	else
		printf("Capture %d started %d %d\n", camid2, imgMat.cols, imgMat.rows);


	wfFR_setMinFaceDetectionSizePercent(15);
	void* hFR = NULL;
	char pBasePath[255] = { 0 };
	int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_RECOGNIZE, 0);
	if (rval != FR_OK || hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}
	
	cv::Mat imgMatGray, imgMatGray2;
	cvtColor(imgMat, imgMatGray, CV_BGR2GRAY);
	cvtColor(imgMat2, imgMatGray2, CV_BGR2GRAY);

	IplImage framegray1 = imgMatGray;
	IplImage framegray2 = imgMatGray2;

	int frameCounter = 0;
	int saveCounter = 0;
	printf("Recognition Started\n");
	while(1)
	{

		vcap.read(imgMat);
		if (imgMat.empty())
		{
			printf("Capture end 1\n");
			break;
		}
		

		vcap2.read(imgMat2);
		if (imgMat2.empty())
		{
			printf("Capture end 2\n");
			break;
		}
		Mat imgMatt = imgMat(cv::Rect(0, 0, imgMat.cols, imgMat.rows-1)).clone();
		Mat imgMatt2 = imgMat2(cv::Rect(0, 0, imgMat.cols, imgMat.rows-1)).clone();
		
		cvtColor(imgMatt, imgMatGray, CV_BGR2GRAY);
		cvtColor(imgMatt2, imgMatGray2, CV_BGR2GRAY);
		framegray2 = imgMatGray2;
		IplImage framecolor1 = imgMatt;
		IplImage *yuv420 = NULL;
	
		yuv420 = cvCreateImage(cvSize(framecolor1.width, (int)(framecolor1.height*1.5f + 0.5f)), 8,1);
		ConvertBGR_TO_YUV420SP_Rec(&framecolor1, yuv420, framecolor1.width, framecolor1.height);

		FRIDList pResult1;
		int frstatus1;
		FRIDList pResult2;
		
		//printf("Size %d, %d, %d, col %d %d\n", framegray2.width, framegray2.height, framegray2.widthStep, framecolor1.width, framecolor1.height);
		wfFR_RecognizeFacesDualCam(hFR, (unsigned char*)(yuv420->imageData), (unsigned char*)(framegray2.imageData), framegray2.width, framegray2.height, framegray2.widthStep, &pResult1, &pResult2, &frstatus1);
		

		//printf("Detection %d, %d\n\n", pResult1.nResults, pResult2.nResults);
		drawDualspoof(imgMat, imgMat2, pResult1, frstatus1, pResult2, 0);
		imshow("Color", imgMat);
		imshow("IR", imgMat2);
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
#if ENABLE_SECOND
			sprintf(oname, "images//image_%d_1.jpg",saveCounter);
			imwrite(oname, imgMat2);
#endif

			printf("Saved %d\n", saveCounter);
			saveCounter++;
		}
		frameCounter++;

		if(yuv420!=NULL)
			cvReleaseImage(&yuv420);
		//printf("after rec API: "); checkMemory();
		if(pResult1.nResults > 0) printf("\n\n");
	}

	wfFR_Release(&hFR);
	cvDestroyAllWindows();
	vcap.release();
#if ENABLE_SECOND
	vcap2.release();
#endif
	printf("Recognition Ends\n");
	return 1;
}

void testdualcamRecImage()
{
	//char* imagePath = "/home/titan/Data/Spoofdata/hqvt/hqvt_dualcam_threshold_-8_realperson";	//realperson paper
	//char* imagePath = "qianxinde/dualcamimages1/";	//0-100
	//char* imagePath = "/tmp/image/dualcamimages";

	char* imagePath = "../hqvt_dualcam_threshold_-8_realperson"; // 70-120
	//char* imagePath = "hqvt/dualcamimages";

	wfFR_setMinFaceDetectionSizePercent(10);
	//wfFR_setSingleCamSpoofThreshold(0.0f);
	wfFR_setVerbose("", 1);

	void* hFR = NULL;
	char pBasePath[255] = { 0 };
	int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_RECOGNIZE, 0);
	if (rval != FR_OK || hFR == NULL)
	{
		printf("Error in FR init\n");
		return;
	}
	int totalcount = 0, passcount = 0;

	FRMiscParams mparams;
	wfFR_getMiscParameters(&mparams);
	mparams.enableDualcamBGReject = 0;
	wfFR_setMiscParameters(mparams);

	for(int m=70;m<120;m++)	//11000
	{
		char name1[1024];
		char name2[1024];
		sprintf(name1, "%s/color_%d.jpg", imagePath, m);	
		sprintf(name2, "%s/ir_%d.jpg", imagePath, m);

		FRIDList pResult1;
		int frstatus1;
		FRIDList pResult2;


		int spoofPassed = -1;
		int imageFound = 1;
		float imageScaleFac = 1.0f;
		for(int i=0;i<1;i++)
		{
			IplImage* inpcol = cvLoadImage(name1,1);
			IplImage* inpir = cvLoadImage(name2,0);
			if(inpcol == NULL || inpir == NULL)
			{
				imageFound = 0;
				//printf("Error in loading images <%s>\n", name1);
				continue;
			}	
			//cvFlip(inpcol, inpcol, 0);

			IplImage* frameInputcolor1 = inpcol;
			IplImage* frameInputgray2 = inpir;
			if(imageScaleFac != 1)
			{
				int newWidth = 	(int)(inpcol->width*imageScaleFac + 0.5f);
				int newHeight = (int)(inpcol->height*imageScaleFac + 0.5f);
				frameInputcolor1 = cvCreateImage(cvSize(newWidth, newHeight), 8, 3);
				frameInputgray2 = cvCreateImage(cvSize(newWidth, newHeight), 8, 1);
				cvResize(inpcol, frameInputcolor1);
				cvResize(inpir, frameInputgray2);
			}
#if 0 // rotate
			Mat imageMat1 = cvarrToMat(frameInputcolor1);
			//imshow("before1", imageMat1);
			cv::transpose(imageMat1, imageMat1);
			//imshow("after1", imageMat1);
			IplImage framecolor1tmp = imageMat1;
			IplImage* color1 = &framecolor1tmp;

			Mat imageMat2 = cvarrToMat(frameInputgray2);
			//imshow("before2", imageMat2);
			cv::transpose(imageMat2, imageMat2);
			//imshow("after2", imageMat2);
			IplImage framegray2tmp = imageMat2;
			IplImage* gray2 = &framegray2tmp;
#else
			IplImage* color1 = frameInputcolor1;
			IplImage* gray2 = frameInputgray2;
#endif
	
			
			if(color1 == NULL || gray2 == NULL)
			{
				imageFound = 0;
				printf("Error in loading images <%s>\n", name1);
				continue;
			}
			printf("Image %s\n", name1);

			IplImage* framecolor1, *framegray2;
			if(color1->height%4 != 0)
			{
				CvRect r = cvRect(0,0,color1->width, color1->height-color1->height%4);
				framecolor1 = cvCreateImage(cvSize(r.width, r.height), 8, 3);
				framegray2 = cvCreateImage(cvSize(r.width, r.height), 8, 1);
				cvSetImageROI(color1, r);
				cvCopy(color1, framecolor1);
				cvResetImageROI(color1);

				cvSetImageROI(gray2, r);
				cvCopy(gray2, framegray2);
				cvResetImageROI(gray2);
			}
			else
			{			
				framecolor1 = color1;
				framegray2 = gray2;
			}

			IplImage* framegray1 = cvCreateImage(cvGetSize(framecolor1), 8, 1);
			cvCvtColor(framecolor1, framegray1, CV_BGR2GRAY);
	
			//cvCvtColor(framegray1, framecolor1, CV_GRAY2BGR);
			IplImage *yuv420 = cvCreateImage(cvSize(framecolor1->width, framecolor1->height*1.5f), 8,1);
			ConvertBGR_TO_YUV420SP_Rec(framecolor1, yuv420, framecolor1->width, framecolor1->height);

			pResult1.nResults = 0;
			pResult2.nResults = 0;
			frstatus1 = 0;
			double t1 = cvGetTickCount();
			int rval = wfFR_RecognizeFacesDualCam(hFR, (unsigned char*)(yuv420->imageData), (unsigned char*)(framegray2->imageData),
											framegray2->width, framegray2->height, framegray2->widthStep, &pResult1, &pResult2, &frstatus1);
			t1 = (cvGetTickCount() - t1) / (cvGetTickFrequency()*1000.0f);

			if(rval != 0)
			{
				exit(0);
			}
			printf("dual cam %d %d, spoof %d, Total Time %.2f\n\n\n", pResult1.nResults, pResult2.nResults, frstatus1, t1);
		
			Mat imgMat = cvarrToMat(framecolor1);
			Mat imgMat2 = cvarrToMat(framegray2);
#if 0 // display
			drawDualspoof(imgMat, imgMat2, pResult1, frstatus1, pResult2, 0);
			cvShowImage("color", framecolor1);
			cvShowImage("ir", framegray2);	
			//if(pResult1.nResults > 0 && frstatus1 == 1)// || pResult1.nResults==0)
			//if(pResult1.nResults == 0)
				cvWaitKey(0);	
			cvWaitKey(10);	
#endif

			if(frstatus1 == 1)
			{
				spoofPassed = 1;
				break;
			}
			if(pResult1.nResults > 0 && frstatus1 == 0)
				spoofPassed = 0;

			//if(i == 9)
				//cvWaitKey(0);	
			
			cvReleaseImage(&frameInputgray2);
			cvReleaseImage(&frameInputcolor1);
			cvReleaseImage(&yuv420);
			cvReleaseImage(&framegray1);

			if(gray2 != framegray2)
			{
				cvReleaseImage(&framegray2);
			}
			if(color1 != framecolor1)
			{
				cvReleaseImage(&framecolor1);
			}

		}
		if(spoofPassed == 1)
			passcount++;
		if(spoofPassed >= 0)
			totalcount++;

		
		if(imageFound) printf("\n************ passcount %d, totalcount %d\n",passcount, totalcount);
	}

	wfFR_Release(&hFR);
}


void testdualcamEnrollImage()
{
	//char* imagePath = "/home/titan/Data/Spoofdata/hqvt/hqvt_dualcam_threshold_-8_realperson";	// 70-120
	char* imagePath = "hqvt/dualcamimages";		// 14 - 60
	wfFR_setMinFaceDetectionSizePercent(10);
	wfFR_setVerbose("", 1);

	void* hFR = NULL;
	char pBasePath[255] = { 0 };
	int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_ENROLL, 0);
	if (rval != FR_OK || hFR == NULL)
	{
		printf("Error in FR init\n");
		return;
	}
	
	unsigned long recordID = ULONG_MAX;
	char fname[10], lname[10];
	sprintf(fname, "jen");
	sprintf(lname, "anis");
	rval =  wfFR_AddRecord(hFR, &recordID, fname, lname);

	int totalcount = 0, passcount = 0;
	//long long int folders[] = {1542244857748};
	for(int m=14;m<60;m++)
	{
		char name1[1024];
		char name2[1024];

		sprintf(name1, "%s/color_%d.jpg", imagePath, m);
		sprintf(name2, "%s/ir_%d.jpg", imagePath, m);

		FRIDList pResult1;
		int frstatus1;
		FRIDList pResult2;



		int spoofPassed = -1;
		for(int i=0;i<1;i++)
		{
			IplImage* frameInputcolor1 = cvLoadImage(name1,1);
			IplImage* frameInputgray2 = cvLoadImage(name2,0);
#if 0 // rotate
			Mat imageMat1 = cvarrToMat(frameInputcolor1);
			//imshow("before1", imageMat1);
			cv::transpose(imageMat1, imageMat1);
			//imshow("after1", imageMat1);
			IplImage framecolor1tmp = imageMat1;
			IplImage* framecolor1 = &framecolor1tmp;

			Mat imageMat2 = cvarrToMat(frameInputgray2);
			//imshow("before2", imageMat2);
			cv::transpose(imageMat2, imageMat2);
			//imshow("after2", imageMat2);
			IplImage framegray2tmp = imageMat2;
			IplImage* framegray2 = &framegray2tmp;
#else
			IplImage* framecolor1 = frameInputcolor1;
			IplImage* framegray2 = frameInputgray2;
#endif

			printf("Image %s\n", name1);
			if(framecolor1 == NULL || framegray2 == NULL)
			{
				printf("Error in loading images <%s>\n", name1);
				continue;
			}

			IplImage* framegray1 = cvCreateImage(cvGetSize(framecolor1), 8, 1);
			cvCvtColor(framecolor1, framegray1, CV_BGR2GRAY);
	
		
			IplImage *yuv420 = cvCreateImage(cvSize(framecolor1->width, framecolor1->height*1.5f), 8,1);
			ConvertBGR_TO_YUV420SP_Rec(framecolor1, yuv420, framecolor1->width, framecolor1->height);

			pResult1.nResults = 0;
			pResult2.nResults = 0;
			frstatus1 = 0;
			int rval = wfFR_EnrollFaceDualCam(hFR, (unsigned char*)(yuv420->imageData), (unsigned char*)(framegray2->imageData),
											framegray2->width, framegray2->height, framegray2->widthStep, recordID, &pResult1, &pResult2, &frstatus1);

			if(rval != 0)
			{
				exit(0);
			}
			printf("dual cam %d %d, spoof %d\n\n\n", pResult1.nResults, pResult2.nResults, frstatus1);
		
			Mat imgMat = cvarrToMat(framecolor1);
			Mat imgMat2 = cvarrToMat(framegray2);
			drawDualspoof(imgMat, imgMat2, pResult1, frstatus1, pResult2, 0);
			cvShowImage("color", framecolor1);
			cvShowImage("ir", framegray2);	
			//if(pResult1.nResults > 0)// && frstatus1 == 0)
				cvWaitKey(0);	
			char c = cvWaitKey(10);	
			if(c == 'q')
			{
				wfFR_Release(&hFR);
				exit(0);
			}

			if(frstatus1 == 1)
			{
				spoofPassed = 1;
				break;
			}
			if(pResult1.nResults > 0)
				spoofPassed = 0;

			cvReleaseImage(&framegray1);
			cvReleaseImage(&frameInputgray2);
			cvReleaseImage(&frameInputcolor1);
			cvReleaseImage(&yuv420);

		}
		if(spoofPassed == 1)
			passcount++;
		if(spoofPassed >= 0)
			totalcount++;

		
		printf("\n************ passcount %d, totalcount %d\n",passcount, totalcount);
	}

	wfFR_Release(&hFR);
}



int testIDCheckCameraDualcam(int camid, int camid2)
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


	cv::VideoCapture vcap2;
	vcap2.open(camid2);
	if (vcap2.isOpened())
	{
		//vcap2.set(CV_CAP_PROP_FRAME_WIDTH, 640);
		//vcap2.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	}
	else
	{
		printf("Error in capturing from camera 2 ID %d\n", camid2);
		return 0;
	}

	Mat imgMat2;
	vcap2.read(imgMat2);
	if (imgMat2.empty())
	{
		printf("Failed to capture image %d\n", camid2);
		return 0;
	}
	else
		printf("Capture %d started %d %d\n", camid2, imgMat.cols, imgMat.rows);

	void* hFR = NULL;
	char pBasePath[255] = { 0 };
	int rval = wfFRID_Init(&hFR, pBasePath, 1);
	if (rval != FR_OK || hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}
	
	cv::Mat imgMatGray, imgMatGray2;
	cvtColor(imgMat, imgMatGray, CV_BGR2GRAY);
	cvtColor(imgMat2, imgMatGray2, CV_BGR2GRAY);

	IplImage framegray1 = imgMatGray;
	IplImage framegray2 = imgMatGray2;

	char* imageNameID = "testimages/jen_aniston_003.jpg";
	int enrollSuccess = 0;
	rval = wfFRID_EnrollIDFromSavedImage(hFR,  imageNameID,	&enrollSuccess);
	if(rval != FR_OK || enrollSuccess == 0)
	{
		printf("Error in FR EnrollID, rval %d, enrollSuccess %d\n", rval, enrollSuccess);
		return 0;
	}

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

		vcap2.read(imgMat2);
		if (imgMat2.empty())
		{
			printf("Capture end 2\n");
			break;
		}
		

		cvtColor(imgMat, imgMatGray, CV_BGR2GRAY);
		cvtColor(imgMat2, imgMatGray2, CV_BGR2GRAY);

		FRIDList pResult1;
		int frstatus1;
		FRIDList pResult2;
		

		rval = wfFRID_RecognizeFacesDualCam(hFR, (unsigned char*)(framegray1.imageData), (unsigned char*)(framegray2.imageData), framegray2.width, framegray2.height, framegray2.widthStep, &pResult1, &pResult2, &frstatus1);
		if(rval!= FR_OK)
		{
			printf("Error %d in wfFRID_RecognizeFacesDualCam\n", rval);
			break;
		}
		

		//printf("Detection %d, %d\n\n", pResult1.nResults, pResult2.nResults);
		drawDualspoof(imgMat, imgMat2, pResult1, frstatus1, pResult2, 0);
		imshow("imageColor", imgMat);
		imshow("imageIR", imgMat2);
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
#if ENABLE_SECOND
			sprintf(oname, "images//image_%d_1.jpg",saveCounter);
			imwrite(oname, imgMat2);
#endif

			printf("Saved %d\n", saveCounter);
			saveCounter++;
		}
		frameCounter++;
	}

	wfFRID_Release(&hFR);
	cvDestroyAllWindows();
	vcap.release();
#if ENABLE_SECOND
	vcap2.release();
#endif
	
	return 0;
}


int testdualcamRecImageID()
{

	void* hFR = NULL;
	char pBasePath[255] = { 0 };
	int rval = wfFRID_Init(&hFR, pBasePath, 0);
	if (rval != FR_OK || hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}

	char* nameid = "testimagesID/35080219920811251X_ID.jpg";
	char name1[1024];
	char name2[1024];
	sprintf(name1, "slec/dualcam/images/image_clr2.jpg");
	sprintf(name2, "slec/dualcam/images/image_ir2.jpg");

	for(int k=0;k<10;k++)
	{	
		int enrollSuccess = 0;
		//printf("wfFRID Enroll Start\n");
		rval = wfFRID_EnrollIDFromSavedImage(hFR,  nameid, &enrollSuccess);
		if(rval != FR_OK || enrollSuccess == 0)
		{
			printf("Error in FR EnrollID, rval %d, enrollSuccess %d, name %s\n", rval, enrollSuccess, nameid);
			return 0;
		}
		printf("Enroll success status %d\n\n", enrollSuccess);
		for(int i=0;i<10;i++)
		{

			IplImage* framegray1 = cvLoadImage(name1,0);
			IplImage* framegray2 = cvLoadImage(name2,0);

			if(framegray1 == NULL || framegray2 == NULL)
			{
				printf("Error in loading images\n");
				return 0;
			}

		
			FRIDList pResult1;
			int frstatus1;
			FRIDList pResult2;
			pResult1.nResults = 0;
			pResult2.nResults = 0;
			frstatus1 = 0;
			rval = wfFRID_RecognizeFacesDualCam(hFR, (unsigned char*)(framegray1->imageData), (unsigned char*)(framegray2->imageData), framegray2->width, framegray2->height, framegray2->widthStep, &pResult1, &pResult2, &frstatus1);
			//rval = wfFRID_RecognizeFaces(hFR, (unsigned char*)(framegray1->imageData), framegray1->width, framegray1->height, framegray1->widthStep, &pResult1, &frstatus1);
			if(rval!= FR_OK)
			{
				printf("Error %d in wfFRID_RecognizeFacesDualCam\n", rval);
				exit(0);
			}
			printf("dual cam %d %d, spoof %d\n\n\n", pResult1.nResults, pResult2.nResults, frstatus1);
		
			cvShowImage("color", framegray1);
			//cvShowImage("ir", framegray2);	
			cvWaitKey(10);	
		
			cvReleaseImage(&framegray1);
			cvReleaseImage(&framegray2);

		}
	}
	printf("Releasing ID check\n");
	wfFRID_Release(&hFR);
	
}

void testdualcamSignature()
{
	//char* imagePath = "/home/titan/Data/Spoofdata/hqvt/hqvt_dualcam_threshold_-8_realperson";	//realperson paper
	//char* imagePath = "qianxinde/dualcamimages1/";	//0-100
	//char* imagePath = "hqvt/dualcamimages";		// 14 - 60

	char* imagePath = "hqvt_dualcam_threshold_-8_realperson"; // 70-120

	wfFR_setMinFaceDetectionSizePercent(10);
	//wfFR_setSingleCamSpoofThreshold(0.0f);
	wfFR_setVerbose("", 1);

	void* hFR = NULL;
	char pBasePath[255] = { 0 };
	int rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_RECOGNIZE, 0);
	if (rval != FR_OK || hFR == NULL)
	{
		printf("Error in FR init\n");
		return;
	}
	int totalcount = 0, passcount = 0;

	FRMiscParams mparams;
	wfFR_getMiscParameters(&mparams);
	mparams.enableDualcamBGReject = 0;
	wfFR_setMiscParameters(mparams);

	int siglength = 0;
	wfFR_getFaceSignatureSize(hFR, &siglength);
	int enrolldone = 0;
	char* enrollsig = new char[siglength];
	char* recsig = new char[siglength];

	for(int m=14;m<60;m++)	//11000
	{
		char name1[1024];
		char name2[1024];
		sprintf(name1, "%s/color_%d.jpg", imagePath, m);	
		sprintf(name2, "%s/ir_%d.jpg", imagePath, m);

		FRIDList pResult1;
		int frstatus1;
		FRIDList pResult2;


		int spoofPassed = -1;
		int imageFound = 1;
		for(int i=0;i<1;i++)
		{
			IplImage* frameInputcolor1 = cvLoadImage(name1,1);
			IplImage* frameInputgray2 = cvLoadImage(name2,0);
#if 0 // rotate
			Mat imageMat1 = cvarrToMat(frameInputcolor1);
			//imshow("before1", imageMat1);
			cv::transpose(imageMat1, imageMat1);
			//imshow("after1", imageMat1);
			IplImage framecolor1tmp = imageMat1;
			IplImage* color1 = &framecolor1tmp;

			Mat imageMat2 = cvarrToMat(frameInputgray2);
			//imshow("before2", imageMat2);
			cv::transpose(imageMat2, imageMat2);
			//imshow("after2", imageMat2);
			IplImage framegray2tmp = imageMat2;
			IplImage* gray2 = &framegray2tmp;
#else
			IplImage* color1 = frameInputcolor1;
			IplImage* gray2 = frameInputgray2;
#endif
	
			
			if(color1 == NULL || gray2 == NULL)
			{
				imageFound = 0;
				printf("Error in loading images <%s>\n", name1);
				continue;
			}
			printf("Image %s\n", name1);

			IplImage* framecolor1, *framegray2;
			if(color1->height%4 != 0)
			{
				CvRect r = cvRect(0,0,color1->width, color1->height-color1->height%4);
				framecolor1 = cvCreateImage(cvSize(r.width, r.height), 8, 3);
				framegray2 = cvCreateImage(cvSize(r.width, r.height), 8, 1);
				cvSetImageROI(color1, r);
				cvCopy(color1, framecolor1);
				cvResetImageROI(color1);

				cvSetImageROI(gray2, r);
				cvCopy(gray2, framegray2);
				cvResetImageROI(gray2);
			}
			else
			{			
				framecolor1 = color1;
				framegray2 = gray2;
			}

			IplImage* framegray1 = cvCreateImage(cvGetSize(framecolor1), 8, 1);
			cvCvtColor(framecolor1, framegray1, CV_BGR2GRAY);
	
			//cvCvtColor(framegray1, framecolor1, CV_GRAY2BGR);
			IplImage *yuv420 = cvCreateImage(cvSize(framecolor1->width, framecolor1->height*1.5f), 8,1);
			ConvertBGR_TO_YUV420SP_Rec(framecolor1, yuv420, framecolor1->width, framecolor1->height);

			pResult1.nResults = 0;
			pResult2.nResults = 0;
			frstatus1 = 0;
			int sigsucc = 0;
			double t1 = cvGetTickCount();
			int rval = wfFR_calcFaceSignatureDualCam(hFR, (unsigned char*)(yuv420->imageData), (unsigned char*)(framegray2->imageData), framegray2->width, framegray2->height, 
									framegray2->widthStep, &pResult1, &pResult2, &frstatus1, recsig, &sigsucc);
			t1 = (cvGetTickCount() - t1) / (cvGetTickFrequency()*1000.0f);

			if(rval != 0)
			{
				exit(0);
			}

			if(sigsucc == 1)
			{
				if(enrolldone == 0)
				{
					memcpy(enrollsig, recsig, siglength);
					enrolldone = 1;
				}
				else
				{
					float Confidence = 0;
					wfFR_compareFaceSignatures(hFR, enrollsig, recsig, &Confidence);
					printf("Match confidence %.2f\n", Confidence);
				}
			}
			printf("dual cam %d %d, spoof %d, recsucc %d, Total Time %.2f\n\n\n", pResult1.nResults, pResult2.nResults, frstatus1, sigsucc, t1);
		
			Mat imgMat = cvarrToMat(framecolor1);
			Mat imgMat2 = cvarrToMat(framegray2);
#if 1 // display
			drawDualspoof(imgMat, imgMat2, pResult1, frstatus1, pResult2, 0);
			cvShowImage("color", framecolor1);
			cvShowImage("ir", framegray2);	
			if(pResult1.nResults > 0 && frstatus1 == 1)// || pResult1.nResults==0)
			//if(pResult1.nResults == 0)
				cvWaitKey(0);	
			cvWaitKey(10);	
#endif

			if(frstatus1 == 1)
			{
				spoofPassed = 1;
				break;
			}
			if(pResult1.nResults > 0 && frstatus1 == 0)
				spoofPassed = 0;

			//if(i == 9)
				//cvWaitKey(0);	
			
			cvReleaseImage(&frameInputgray2);
			cvReleaseImage(&frameInputcolor1);
			cvReleaseImage(&yuv420);
			cvReleaseImage(&framegray1);

			if(gray2 != framegray2)
			{
				cvReleaseImage(&framegray2);
			}
			if(color1 != framecolor1)
			{
				cvReleaseImage(&framecolor1);
			}

		}
		if(spoofPassed == 1)
			passcount++;
		if(spoofPassed >= 0)
			totalcount++;

		
		if(imageFound) printf("\n************ passcount %d, totalcount %d\n",passcount, totalcount);
	}

	wfFR_Release(&hFR);
}
int main15(int argc, char* argv[])
{
	printf("Testing Dualcam\n");
#if 0
	int camid = 1;
	int camid2 = 0;
	int mode = 0; // 0 for recognition, 1 for enroll
	string fname = "Jennifer";
	printf("USAGE: %s <colorCamIndex> <IRCamIndex> <mode - 0 for recognition, 1 for enroll> <Firstname (enroll mode)>\n", argv[0]);
	if(argc > 1)
		camid = atoi(argv[1]);
	if(argc > 2)
		camid2 = atoi(argv[2]);
	if(argc > 3)
		mode = atoi(argv[3]);
	if(argc > 4)
		fname = argv[4];
		
	if(mode == 0)
		testdualcamfrCamera(camid, camid2);
	else if (mode == 1)
		testdualcamfrEnrollCamera(camid, camid2, fname);

#else
	testdualcamRecImage();

	//testdualcamSignature();
		
	//testdualcamEnrollImage();

	//testdualcamRecImageID();
	//testIDCheckCameraDualcam(camid, camid2);
#endif
	return 0;

}
