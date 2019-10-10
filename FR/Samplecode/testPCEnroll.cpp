#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <time.h>

using namespace std;

#include "FRLibrary.h"
#include "FRLibraryPC.h"

#define USE_OPENCV (1)

#if USE_OPENCV
#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace cv;
#endif

/* Number of maximum faces to be detected in the scene */
#define MAX_FACES (1)

/* Time in seconds for which enrolling should be run. It can be in range of 5 to 20 seconds*/
#define ENROLLING_TIME (5) 

#define RESIZE_IMAGE (0)



// NV21 is YVU420 which is YCrCb420 as in below function. NV12 is is YUV420
void ConvertBGR_TO_YUV420SP(const IplImage* pBGR, IplImage* yuv420)
{
    //IplImage* pYUV = 0;
    int i, j;
    CvSize size;
    
    size = cvGetSize(pBGR);
    //pYUV = cvCreateImage(size, 8, 3);
    //cvCvtColor(pBGR, pYUV, CV_BGR2YCrCb);

    for (i = 0; i < pBGR->height; i++)
    {
        const uchar* pBGRData = (const uchar*) (pBGR->imageData + i*pBGR->widthStep);
        uchar* pYData = (uchar*) (yuv420->imageData + i*(yuv420->widthStep));
        uchar* pVUData = (uchar*) (yuv420->imageData + pBGR->height*(yuv420->widthStep) + (i/2)*(yuv420->widthStep));
        for (j = 0; j < pBGR->width; j++, pBGRData+=3)
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


/* Sample code to enroll face on PC. DB for person will be stored in folder "<BaseFolderPC>/wffrdbpc/<firstname> <secondname>/".
To update the DB on device with a person enrolled ion PC, copy folder "<BaseFolderPC>/wffrdbpc/<firstname> <secondname>/" to device
at "<BaseFolderDevice>/wffrdbpc/<firstname> <secondname>/".
On initializing FR on device, it will automatically update DB with all people stored in "<BaseFolderDevice>/wffrdbpc/".
*/
int testEnrollFromVideoPC(string imgname, string fname, string lname)
{
	printf("PC Enroll start\n");

	///////////// Read image from the file /////////////
#if USE_OPENCV
	IplImage* imageOrig = cvLoadImage(imgname.c_str(),0);
	
	#if RESIZE_IMAGE
		float scalefac = MAX((float)imageOrig->width/640,1);
		CvSize sz = cvSize(imageOrig->width/scalefac, imageOrig->height/scalefac);
		IplImage* image = cvCreateImage(cvSize(160+sz.width, 120+sz.height),8,1);
		cvZero(image);
		cvSetImageROI(image,cvRect(80,60,sz.width,sz.height));
		cvResize(imageOrig, image);
		cvResetImageROI(image);
		unsigned char* pixeldata = (uchar*)image->imageData;
		int width = image->width;
		int height = image->height;
		int step = image->widthStep;
	#else	
		IplImage* image = imageOrig;
		unsigned char* pixeldata = (uchar*)image->imageData;
		int width = image->width;
		int height = image->height;
		int step = image->widthStep;
	#endif

	
#else
	int width = 640;
	int height = 480;
	unsigned char* pixeldata = new unsigned char[width*height];
	FILE* fp = fopen("frtestimage.dat", "rb");
	fread(pixeldata, 1, width* height, fp);
	fclose(fp);
#endif
	

	//Set base path accordingly for your application. Base path should have files like p0.bin, p1.bin etc and a folder named wffrdbpc.
	//char pBasePath[255] = {0};	

	/* Initialize the engine for recognition. pBasePath is the full path of the data files like p0.bin p1.bin etc 
	Give first and second name of the person to be enrolled as input. 
	Database for the person will be stored in folder "wffrdbpc/<firstname> <secondname>" */

	void* hFR = NULL;
	char pBasePath[255] = {0};
	int rval = wfFRPC_EnrollInit(&hFR, pBasePath, fname.c_str(), lname.c_str(), 0);

	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init rval %d, hFR %d\n", rval, hFR);
		return 0;
	}
	printf("PC Init done Enrolling\n");fflush(stdout);

	
	double startTime = clock(); 
	int frameCounter = 0;

	while(1)
	{
		FRIDList pResult;
		int frstatus = 0;

		/* Send image from stream for enrollment. */
		rval =  wfFRPC_EnrollFace(hFR, (unsigned char*)(pixeldata), width,  height, step, &pResult, &frstatus);
		if(rval != FR_OK)
		{
			printf("Error in FR enroll face\n");
			//return 0;
		}
		printf("Num results %d\n", pResult.nResults);	// Number of detected Faces
		for(int i=0;i<pResult.nResults;i++)
		{
			printf("Frame %d: size %d\n",frameCounter , pResult.pFace[i].width);
		}


		double elaspedTime = (clock() - startTime) / (CLOCKS_PER_SEC );
		//double elaspedTime = (clock() - startTime) / CLOCKS_PER_SEC;
		if(elaspedTime > ENROLLING_TIME)	// end of enrolling is 10 seconds have passed.
			break;
		printf("Elapsed time %f\n", elaspedTime);
		frameCounter++;
		if(frameCounter >= 5)
			break;

		//if(pResult.nResults == 0)
		{
			cvShowImage("image", image);
			cvWaitKey(0);
		}

	}
	
	// Release FR engine after enrolling is done.
	wfFRPC_Release(&hFR);
#if !USE_OPENCV
	delete [] pixeldata;
#endif
	printf("Enroll ends\n");
	return 1;
}

int testEnrollFromJpegBufferPC(string imgname, string fname, string lname)
{		
	printf("Recognize from Saved image start\n");
	void* hFR = NULL;
	char pBasePath[255] = {0};
	int rval = 0;

	wfFRPC_setMinFaceDetectionSizePercent(10);

	rval = wfFRPC_EnrollInit(&hFR, pBasePath, fname.c_str(), lname.c_str(), 0);
	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init\n");
		return 0;
	}
	printf("Init done Recognition\n");fflush(stdout);


	printf("Testing image %s\n", imgname.c_str());
	FILE* fp = fopen(imgname.c_str(),"rb");
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
	rval = wfFRPC_EnrollFaceFromJpegBuffer(hFR, jpegBuffer, sz, &pResult, &frstatus);
	
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
	wfFRPC_Release(&hFR);
}


int testEnrollSingleCamFromVideoPC(string imgname, string fname, string lname)
{
	printf("PC Enroll start\n");
	///////////// Read image from the file /////////////
#if USE_OPENCV
	IplImage* imageOrig = cvLoadImage(imgname.c_str(), 1);
	if(imageOrig ==NULL)
	{
		printf("Error in loading image %s\n", imgname.c_str());
	}
	IplImage* image = imageOrig;

	IplImage *yuv420 = cvCreateImage(cvSize(image->width, image->height*1.5f), 8, 1);
	ConvertBGR_TO_YUV420SP(image, yuv420);
	unsigned char* pixeldata = (uchar*)yuv420->imageData;
	int width = image->width;
	int height = image->height;
	int step = image->width;

#else
	int width = 640;
	int height = 480;
	int step = 640;
	unsigned char* pixeldata = new unsigned char[width*height*1.5];
	FILE* fp = fopen("frtestimageNV21.dat", "rb");
	fread(pixeldata, 1, width* height*1.5, fp);
	fclose(fp);
#endif
	

	//Set base path accordingly for your application. Base path should have files like p0.bin, p1.bin etc and a folder named wffrdbpc.
	//char pBasePath[255] = {0};	

	/* Initialize the engine for recognition. pBasePath is the full path of the data files like p0.bin p1.bin etc 
	Give first and second name of the person to be enrolled as input. 
	Database for the person will be stored in folder "wffrdbpc/<firstname> <secondname>" */

	void* hFR = NULL;
	char pBasePath[255] = {0};
	int rval = wfFRPC_EnrollInit(&hFR, pBasePath, fname.c_str(), lname.c_str(), 0);

	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init rval %d, hFR %d\n", rval, hFR);
		return 0;
	}
	printf("PC Init done Enrolling\n");fflush(stdout);

	
	double startTime = clock(); 
	int frameCounter = 0;

	while(1)
	{
		FRIDList pResult;
		int frstatus = 0;

		/* Send image from stream for enrollment. */
		rval =  wfFRPC_EnrollFaceSingleCamSpoof(hFR, (unsigned char*)(pixeldata), width,  height, step, &pResult, &frstatus);
		if(rval != FR_OK)
		{
			printf("Error in FR enroll face\n");
			//return 0;
		}
		printf("Num results %d\n", pResult.nResults);	// Number of detected Faces
		for(int i=0;i<pResult.nResults;i++)
		{
			printf("Frame %d: size %d, conf %.1f\n",frameCounter , pResult.pFace[i].width, pResult.pConfidence[i]);
		}


		double elaspedTime = (clock() - startTime) / (CLOCKS_PER_SEC );
		//double elaspedTime = (clock() - startTime) / CLOCKS_PER_SEC;
		if(elaspedTime > ENROLLING_TIME)	// end of enrolling is 10 seconds have passed.
			break;
		printf("Elapsed time %f\n", elaspedTime);
		frameCounter++;
		if(frameCounter >= 50)
			break;

		if(pResult.nResults == 0)
		{
			//cvShowImage("image", image);
			//cvWaitKey(0);
		}

	}
	
	// Release FR engine after enrolling is done.
	wfFRPC_Release(&hFR);
#if !USE_OPENCV
	delete [] pixeldata;
#endif
	printf("Enroll ends\n");
	return 1;
}

void testDbManage()
{
	void* hFR = NULL;
	char pBasePath[255] = {0};
	int rval = 0;
	double t0 = 0;

	t0 = cvGetTickCount();
	/* Initialize the engine for recognition. Give input width, height and widthstep for Y channel image.
	pBasePath is the full path of the data files like cp.bin, mpE.dat etc */
	rval = wfFR_Init(&hFR, pBasePath, 0, 0, 0, FRMODE_ENROLL, 0);

	t0 = (cvGetTickCount() - t0)/(1000.0*cvGetTickFrequency());
	printf("Init time %.1f\n", t0);

	if(rval != FR_OK ||  hFR == NULL)
	{
		printf("Error in FR init\n");
		return;
	}
	printf("Init done DB manage\n");fflush(stdout);

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


	int numzeros= 0;
	for(int i=0;i<outFirstNameList.size();i++)
	{
		
	
		if(outFirstNameList[i][0] == '0')
		{
			numzeros++;
			printf("%d: ID %d, Firstname <%s>, secondname <%s> \n", i, outIDList[i], outFirstNameList[i].c_str(), outSecondNameList[i].c_str());
		}
		
		std::vector<int> outRecordIDList, outNumImagesList;
		outRecordIDList.reserve(100);
		outNumImagesList.reserve(100);
		/////////// Get list of record ID's for a given name in database //////////
		double t1 = cvGetTickCount();
		rval =  wfFR_GetRecordIDListByName(hFR, outFirstNameList[i].c_str(), outSecondNameList[i].c_str(), outRecordIDList, outNumImagesList);
		t1 = (cvGetTickCount() - t1)/(1000.0*cvGetTickFrequency());
		
	}
	printf("numzeros %d\n", numzeros);

	


	printf("Release FR handle\n");	
	/* Release FR handle */
	wfFR_Release(&hFR);

}



int main(int argv, char* argc[])
{

	string inpname = "testimages/jen_aniston_004_small.jpg";
#if 1
	if(argv !=4)
		return 1;

	
	inpname = argc[1];
	string fname = argc[2];
	string lname = argc[3];

	//testEnrollFromVideoPC(inpname, fname, lname);
	//testEnrollSingleCamFromVideoPC(inpname, fname, lname);
	testEnrollFromJpegBufferPC(inpname, fname, lname);
#endif

#if 0
	for(int i=0;i<10000;i++)
	{
		printf("\n\n************************** %d ******************\n", i);
		char fname[10];
		char lname[10];
		sprintf(fname, "%d", i);
		sprintf(lname, "%d", i);	
		testEnrollFromVideoPC(inpname, fname, lname);
	}
#endif
}
