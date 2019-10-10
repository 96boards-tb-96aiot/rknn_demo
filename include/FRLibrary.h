#ifndef _FRLIB_H
#define _FRLIB_H




#ifdef _WIN32
	#if  (WFFR_ENABLE_PCENROLL_ONLY == 1)	// Enable enrolling on PC
		#ifdef WFFR_EXPORTS
			#define WFFR_API __declspec(dllexport)
			#define WFFRREC_API
		#else
			#define WFFR_API __declspec(dllimport)
			#define WFFRREC_API
		#endif
	#else
		#ifdef WFFR_EXPORTS
			#define WFFR_API __declspec(dllexport)
			#define WFFRREC_API __declspec(dllexport)
		#else
			#define WFFR_API __declspec(dllimport)
			#define WFFRREC_API __declspec(dllimport)
		#endif
	#endif
#else
	#if  (WFFR_ENABLE_PCENROLL_ONLY == 1)
		#define WFFR_API __attribute__ ((visibility ("default")))
		#define WFFRREC_API __attribute__ ((visibility ("hidden")))
	#else
		#define WFFR_API __attribute__ ((visibility ("default")))
		#define WFFRREC_API __attribute__ ((visibility ("default")))
	#endif
#endif

#define FR_OK 0
#define FR_ERROR_GENERAL 1
#define FR_ERROR_BAD_ARGS 2
#define FR_ERROR_LICENSE 3
#define FR_ERROR_NOTSUPPORTED 4
#define FR_ERROR_FRMODE 5
#define FR_ERROR_LOADDB 6
#define FR_ERROR_DBUPDATE 7
#define FR_ERROR_LOADUPDATEDDB 8
#define FR_ERROR_INIT 9
#define FR_ERROR_ADDRECORD 10
#define FR_ERROR_CREATEDBPATH 11
#define FR_ERROR_LOADPCDB 12
#define FR_ERROR_INITCONFLICT (15)

/////////// Licese Error Codes //////////
#define FR_LICERROR_HOSTADDR (-1)
#define FR_LICERROR_SOCKET (-2)
#define FR_LICERROR_SERVERERROR (-3)
#define FR_LICERROR_TIMEOUT (-4)
#define FR_LICERROR_SOCKWRITE (-5)
#define FR_LICERROR_SOCKREAD (-6)
#define FR_LICERROR_READDATA (-7)
#define FR_LICERROR_INCORRECTDATA (-13)
/////////// Licese Error Codes //////////


#include "FRLibraryTypes.h"

#ifndef __ANDROID__
#ifdef __cplusplus
extern "C" {
#endif
#endif

//! Get version information of SDK.
/*!
  \ingroup INI
  \return Version string
*/
WFFR_API const char* wfFR_GetVersionInfo();

//! Verify permanent license.
/*!
  \ingroup INI
  \param pBasePath Base path of the folder which contains all data files
*/
WFFR_API int wfFR_VerifyLic(const char* pBasePath);

//! Create new instance of FR Engine
/*!
  \ingroup INI
  \param pHandle pointer to the handle to be initialized
  \param pBasePath Base path of the folder which contains all data files
  \param iFrameWid video width 
  \param iFrameHig video height 
  \param iFrameStep video width step 
  \param eFrMode Set FRMODE_RECOGNIZE for recognition mode, set FRMODE_ENROLL for enroll mode
  \param enableAntiSpoofing Flag to enable anti-spoofing for face. Set 1 to enable and 0 to disable
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_Init(void** pHandle, const char* pBasePath, int iFrameWid, int iFrameHig, int iFrameStep, FRMode eFrMode, int enableAntiSpoofing);



//! Destroy the instance of Engine and save database
/*!
  \param pHandle pointer to the handle to be freed
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_Release(void** pHandle);

/*! Recognize face from video
  \param handle the Engine's handle
  \param pFrameData query image's pixel data (Y / NV21). Only Gray image / Y channel / NV21 supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_RecognizeFaces(void* handle, const unsigned char* pFrameData, int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResult, int* pStatus);



/*! Enroll face in video mode
  \param handle the Engine's handle
  \param pImageData query image's pixel data. Supports Gray image / Y channel and NV21/NV12
  \param iImageWid query image's width
  \param iImageHig query image's height
  \param iImageStep query image's width step
  \param lRecordID record ID to be trained with new images
  \param iForce set to 1 for image mode, 0 for video mode. Only 0 is supported at the moment
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_EnrollFace(void* handle, const unsigned char* pImageData, int iImageWid, int iImageHig, int iImageStep, unsigned long lRecordID, int iForce, FRIDList* pResult, int* pStatus);



/*! Enroll face in picture mode
  \param handle the Engine's handle
  \param pImageData query image's pixel data. Supports Gray image / Y channel and NV21/NV12
  \param iImageWid query image's width
  \param iImageHig query image's height
  \param iImageStep query image's width step
  \param lRecordID record ID to be trained with new images
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_EnrollFacePictureMode(void* handle, const unsigned char* pImageData, int iImageWid, int iImageHig, int iImageStep, unsigned long lRecordID, FRIDList* pResult, int* pStatus);


/*! Recognize face saved image like jpeg/bmp/png
  \param handle the Engine's handle
  \param pImageFilename Image file name stored on device like  jpeg/bmp/png
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_RecognizeFacesFromSavedImage(void* handle, const char* pImageFilename, FRIDList* pResult, int* pStatus);


/*! Recognize face from Jpeg buffer
  \param handle the Engine's handle
  \param pJpegBuffer Input image Jpeg buffer data
  \param iJpegBufferSize Input image Jpeg buffer data size in bytes
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_RecognizeFacesFromJpegBuffer(void* handle, const char* pJpegBuffer, int iJpegBufferSize, FRIDList* pResult, int* spoofStatus);


/*! Detect and Recognize face from video in multi-thread. Detection is synchronous and recognition is async
  \param handle the Engine's handle
  \param pFrameData query image's pixel data (Y / NV21). Only Gray image / Y channel / NV21 supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_DetectRecognizeFacesMultiThread(void* handle, const unsigned char* pFrameData, int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResult, int* pStatus);


/*! Detect and Recognize face from video in multi-thread with single camera anti-spoof. Detection is syncronous and recognition is async
  \param handle the Engine's handle
  \param pFrameData query image's pixel data (YUV420). Only YUV420 supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_DetectRecognizeSingleCamSpoofMultiThread(void* handle, const unsigned char* pFrameData, int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResult, int* pStatus);


/*! Set format of last recognized image input in multi thread (wfFR_DetectRecognizeFacesMultiThread) that can be fetched using wfFR_getLastRecImageMultiThread
  \param format Format of the last recognized image. Only IMAGE_GRAY and IMAGE_YUV420 are supported
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_setLastRecImageFormatMultiThread(ImageFormat format);

/*! Get last recognized image in multi thread (wfFR_DetectRecognizeFacesMultiThread)
  \param pFrameDataOut Output image (Y or NV21)
  \param iFrameWid Output image's width
  \param iFrameHig Output image's height
  \param iFrameStep Output image's width step
  \param imageFormat Output image's format IMAGE_GRAY or IMAGE_YUV420
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_getLastRecImageMultiThread(unsigned char* pFrameDataOut, int iFrameWid, int  iFrameHig, int iFrameStep,  ImageFormat imageFormat);

/*! Get last recognized results in multi thread (wfFR_DetectRecognizeFacesMultiThread)
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_getLastRecResultsMultiThread(FRIDList* pResult);

/*! Detect and Recognize face from video in multi-thread queue based. Detection is syncronous and recognition is async using queue pushed by detector
  \param handle the Engine's handle
  \param pFrameData query image's pixel data (Y / NV21). Only Gray image / Y channel / NV21 supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param queueLength Current length of the recognition queue
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_DetectRecognizeFacesQueue(void* handle, const unsigned char* pFrameData, int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResult, int* queueLength);


/*! Recognize face from dual camera
  \param handle the Engine's handle
  \param pFrameDataColor query 1st cam image's pixel data (Y / NV21). Only Gray image / Y channel / NV21 supported
  \param pFrameDataIR query 2nd cam image's pixel data (Y / NV21). Only Gray image / Y channel / NV21 supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param pResult1 Output search results for 1st cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pResult2 Output search results for 2nd cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_RecognizeFacesDualCam(void* handle, const unsigned char* pFrameDataColor, const unsigned char* pFrameDataIR, int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResultColor, FRIDList* pResultIR, int* pStatus);


/*! Enroll face from dual camera
  \param handle the Engine's handle
  \param pFrameDataColor query 1st cam image's pixel data (Y / NV21). Only Gray image / Y channel / NV21 supported
  \param pFrameDataIR query 2nd cam image's pixel data (Y / NV21). Only Gray image / Y channel / NV21 supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param lRecordID record ID to be trained with new images
  \param pResult1 Output results for 1st cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pResult2 Output results for 2nd cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_EnrollFaceDualCam(void* handle, const unsigned char* pFrameDataColor,  const unsigned char* pFrameDataIR, 
	int iFrameWid, int  iFrameHig, int iFrameStep, unsigned long lRecordID, FRIDList* pResultColor, FRIDList* pResultIR, int* spoofStatus);


/*! Recognize face from single camera with image anti-spoof
  \param handle the Engine's handle
  \param pFrameData query 1st cam image's pixel data (YUV420). Only YUV420 supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param pResult Output search results for 1st cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_RecognizeFacesSingleCamSpoof(void* handle, const unsigned char* pFrameData, int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResult, int* pStatus);


/*! Recognize face from Jpeg buffer single camera with image anti-spoof
  \param handle the Engine's handle
  \param pJpegBuffer Input image Jpeg buffer data
  \param iJpegBufferSize Input image Jpeg buffer data size in bytes
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param spoofStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_RecognizeFacesSingleCamSpoofFromJpegBuffer(void* handle, const char* pJpegBuffer, int iJpegBufferSize, FRIDList* pResult, int* spoofStatus);


/*! Enroll face from single camera with image anti-spoof
  \param handle the Engine's handle
  \param pFrameData query 1st cam image's pixel data (YUV420). Only YUV420 supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param lRecordID record ID to be trained with new images
  \param pResult Output results for 1st cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_EnrollFaceSingleCamSpoof(void* handle, const unsigned char* pFrameData,  int iFrameWid, int  iFrameHig, int iFrameStep, unsigned long lRecordID, FRIDList* pResult, int* spoofStatus);


/*! Enroll face from Jpeg buffer single camera with image anti-spoof
  \param handle the Engine's handle
  \param pJpegBuffer Input image Jpeg buffer data
  \param iJpegBufferSize Input image Jpeg buffer data size in bytes
  \param lRecordID record ID to be trained with new images
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param spoofStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_EnrollFaceSingleCamSpoofFromJpegBuffer(void* handle,  const char* pJpegBuffer, int iJpegBufferSize, unsigned long lRecordID, FRIDList* pResult, int* spoofStatus);


/*! Enroll face from saved image like jpeg/bmp/png
  \param handle the Engine's handle
  \param pImageFilename Image file name stored on device like  jpeg/bmp/png
  \param lRecordID record ID to be trained with new images
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_EnrollFaceFromSavedImage(void* handle,  const char* pImageFilename, unsigned long lRecordID, FRIDList* pResult, int* pStatus);


/*! Enroll face from Jpeg buffer
  \param handle the Engine's handle
  \param pJpegBuffer Input image Jpeg buffer data
  \param iJpegBufferSize Input image Jpeg buffer data size in bytes
  \param lRecordID record ID to be trained with new images
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param spoofStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_EnrollFaceFromJpegBuffer(void* handle,  const char* pJpegBuffer, int iJpegBufferSize, unsigned long lRecordID, FRIDList* pResult, int* spoofStatus);

/*! Verify if image is suitable for enrollment from frame buffer.
  \param handle the Engine's handle
  \param pFrameData query 1st cam image's pixel data (Y / NV21). Only Gray image / Y channel / NV21 supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_VerifyFrameForEnroll(void* handle, const unsigned char* pFrameData, int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResult, int* pStatus);


/*! Verify if image is suitable for enrollment from Jpeg buffer
  \param handle the Engine's handle
  \param pJpegBuffer Input image Jpeg buffer data
  \param iJpegBufferSize Input image Jpeg buffer data size in bytes
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_VerifyImageForEnrollJpegBuffer(void* handle,  const char* pJpegBuffer, int iJpegBufferSize, FRIDList* pResult, int* pStatus);


/*! Verify if image is suitable for enrollment from an image file name
  \param handle the Engine's handle
  \param pImageFilename Image file name stored on device like  jpeg/bmp/png
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_VerifyImageForEnrollSavedImage(void* handle,  const char* pImageFilename, FRIDList* pResult, int* pStatus);




/*! Detect helmet on head from video
  \param handle the Engine's handle
  \param pFrameData query image's pixel data (Y / NV21). Only Gray image / Y channel / NV21 supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pHelmetStatus Output helmet detection, 1 if helmet is detection, 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_DetectHelmet(void* handle, const unsigned char* pFrameData, int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResult, int* pHelmetStatus);


//! Set threshold for helmet detected
/*!
\param threshold Set helmet detected threshold. Default is 0. Range is [-10.0f, 10.0]. Decreasing threshold will increase helmet detection sensitivity
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setHelmetThreshold(float threshold);

//! Get threshold for helmet detected
/*!
\param pthreshold Output Get helmet detected threshold. 
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getHelmetThreshold(float* pthreshold);

/*! Set format of input image buffer. Default is NV21
  \param format Format of input image buffer. Only IMAGE_GRAY, IMAGE_YV12, IMAGE_YV12_INV, IMAGE_YUV420 (NV21) and IMAGE_YUV420_NV12 are supported
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setInputImageBufferFormat(ImageFormat format);

/*! Get format of input image buffer. Default is NV21
  \param format Format of input image buffer. Only IMAGE_GRAY, IMAGE_YV12, IMAGE_YV12_INV, IMAGE_YUV420 (NV21) and IMAGE_YUV420_NV12 are supported
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getInputImageBufferFormat(ImageFormat *format);


//! Creates new record and returns new recordID.
/*!
  \param handle the Engine's handle
  \param pRecordID pointer to record's ID (output)
  \param firstName First name of the person
  \param secondName Second name of the person
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_AddRecord(void* handle, unsigned long* pRecordID, const char* firstName, const char* secondName);



//! Remove single record whose record ID is lRecordID from the database. It will remove only one record for the person.
/*!
  \param handle the Engine's handle
  \param lRecordID record ID to be removed
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_RemoveSingleRecord(void* handle, unsigned long  lRecordID);


//! Remove all record of the person whose record ID is lRecordID from the database. It will completely remove the person from the database. 
/*! 
  \param handle the Engine's handle
  \param lRecordID record ID of the person
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_RemovePersonByRecordID(void* handle, unsigned long  lRecordID);


//! Remove person from the database by name. It will completely remove the person from the database. 
/*!
  \param handle the Engine's handle
  \param firstName First name of the person
  \param secondName Second name of the person
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_RemovePersonByName(void* handle, const char* firstName, const char* secondName);

//! Remove person from the database by first name. It will completely remove the person from the database. The first name should be unique in database. 
/*!
  \param handle the Engine's handle
  \param firstName First name of the person
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_RemovePersonByFirstName(void* handle, const char* firstName);

//! Delete all enrolled people from the database. Database will be empty after this API
/*!
\param handle the Engine's handle
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_RemoveAllRecord(void* handle);


//! Rename record whose record ID is lRecordID from the database.
/*! 
  \param handle the Engine's handle
  \param lRecordID record ID of the person
  \param firstName First name of the person
  \param secondName Second name of the person
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_RenameRecordID(void* handle, unsigned long  lRecordID, const char* firstName, const char* secondName);


//! Rename person by name in the database.
/*! 
  \param handle the Engine's handle
  \param firstName Existing First name of the person
  \param secondName Existing Second name of the person
  \param newfirstName New First name of the person
  \param newsecondName New Second name of the person
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_RenamePersonByName(void* handle, const char* firstName, const char* secondName, const char* newfirstName, const char* newsecondName);

//! Extract person from database by name. Extracted person will saved in another folder <BasePath>/wffrdbExtract.
/*!
  \param handle the Engine's handle
  \param firstName First name of the person
  \param secondName Second name of the person
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_ExtractPersonByName(void* handle, const char* firstName, const char* secondName);


#ifdef __cplusplus
//! Get list of names for people enrolled in DB. Also returns the first record ID for each name in database.
/*!
\param handle the Engine's handle
\param outFirstNameList Output list of first names
\param outSecondNameList Output list of second names
\param outRecordIDList Output list of first record ID for each name
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/

WFFR_API int wfFR_GetNamelist(void* handle, std::vector<std::string>& outFirstNameList,  
	std::vector<std::string>& outSecondNameList,  std::vector<int>& outRecordIDList);


//! Get list of all recordID's in sorted order for the person with input name. It also gets number of images enrolled for each recordID.
/*!
\param handle the Engine's handle
\param firstName First name of the person
\param secondName Second name of the person
\param outRecordIDList Output list of unique recordID's enrolled for input name
\param outNumImagesList Output list of number of images enrolled for each recordID
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_GetRecordIDListByName(void* handle, const char* firstName, const char* secondName,  
				std::vector<int>& outRecordIDList, std::vector<int>& outNumImagesList);

#endif

//! Get all images for people enrolled in DB one by one. It will fetch images from all records for the person with record ID lRecordID
/*!
\param handle the Engine's handle
\param lRecordID any one of the Record ID for the person
\param inpImageIndex Image index to to be retrieved from DB
\param totalImages (Ouptut) Total number of images for the person.
\param outImageJpeg (Output) Pointer to the jpeg image byte data.
\param outImageJpegSize (Output) Size in bytes for the ouptut jpeg image.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_GetImageFromDb(void* handle, unsigned long lRecordID, int inpImageIndex,
				int* totalImages, char** outImageJpeg, int* outImageJpegSize);

//! Set Face Spoof detection sensitivity
/*!
\param handle the Engine's handle
\param sensitivity Sensitivity for face spoof. Range [0,3]. Default is 1 (medium). 
				0 is low (difficult), 
				1 is medium 		
				2 is high sensitivity (easy)
				3 is very high sensitivity (very easy)
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setSpoofingSensitivity(void* handle, int sensitivity);


//! Get Face Spoof detection sensitivity
/*!
\param handle the Engine's handle
\param psensitivity Output sensitivity for face spoof. Range [0,3]. 
				0 is low (difficult), 
				1 is medium 		
				2 is high sensitivity (easy)
				3 is very high sensitivity (very easy)
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getSpoofingSensitivity(void* handle, int* psensitivity);


//! Set Face Recognition confidence threshold
/*!
\param confidenceThresold Confidence threshold to be set for for face recognition. Range is [10,80]
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRREC_API int wfFR_setRecognitionThreshold(float confidenceThresold);


//! Get Face Recognition confidence threshold
/*!
\param pconfidenceThresold Output confidence threshold for face recognition
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRREC_API int wfFR_getRecognitionThreshold(float* pconfidenceThresold);

//! Set Face detection minimum size in percentage. It should be called before wfFR_Init.
/*!
\param minfacesize Minimum face detection size in percentage of input image dimentions. Range is [10,100]
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setMinFaceDetectionSizePercent(int minfacesize);

//! Get Face detection minimum size in percentage
/*!
\param pminfacesize Output minimum face detection size in percentage of input image dimentions.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getMinFaceDetectionSizePercent(int* pminfacesize);

//! Set Face detection minimum size in pixels. It should be called before wfFR_Init.
/*!
\param handle the Engine's handle
\param minfacesize Minimum face detection size in percentage of input image dimentions. Range is [50,1000]
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setMinFaceDetectionSizePixels(int minfacesizePixels);

//! Get Face detection minimum size in pixels.
/*!
\param handle the Engine's handle
\param pminfacesize Output minimum face detection size in percentage of input image dimentions.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getMinFaceDetectionSizePixels(int* pminfacesizePixels);



//! Set engine mode to run Face detection only. Turns off recognition 
/*!
\param runDetectionOnly Set to 1 to run detection only, set to 0 to run detection and recognition
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setDetectionOnlyMode(int runDetectionOnly);

//! Get engine mode to run Face detection only. 
/*!
\param prunDetectionOnly Output run detection only mode
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getDetectionOnlyMode(int* prunDetectionOnly);



//! Set face detection algorithm type. Should be set before wfFR_Init is called
/*!
\param algotype Set to 0 for fast detector, set to 1 for more accurate and slower detector
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setDetectionAlgoType(int algotype);

//! Get face detection algorith type. 
/*!
\param palgotype Output detection algorithm type. 0 for fast detector, 1 for more accurate and slower detector
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getDetectionAlgoType(int* palgotype);


//! Set Number of cores for processing. Set should be called before first Init() API call 
/*!
\param handle the Engine's handle
\param numcores Number of processing cores. Range is [1,8]
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setNumProcessingCores(int numcores);

//! Get Number of cores for processing. Get should be called after Init().
/*!
\param handle the Engine's handle
\param pnumcores Output Number of processing cores.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getNumProcessingCores(int* pnumcores);



//! Set threshold for single cam spoof to make it more or less sensitive
/*!
\param spoofThresh Set single cam spoof threshold. Range is [-30.0f, 30.0]. Decreasing threshold will increase sensitivity
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setSingleCamSpoofThreshold(float spoofThresh);

//! Get threshold for single cam spoof
/*!
\param pspoofThresh Output Get single cam spoof threshold. 
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getSingleCamSpoofThreshold(float* pspoofThresh);



//! Set engine mode to save detected faces
/*!
\param saveDetectedFaces Set to 1 or 2 to save detected faces to folder <basepath>/fd/
			1: save tightly cropped face
			2: save detected face with 20% border
\param saveImagePath Path to save image. If set to NULL, the faces will be saved in default folder <basepath>/fd/
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setSaveDetectedFaceFlag(int saveDetectedFaces, const char* saveImagePath);

//! Get engine mode to save detected faces
/*!
\param psaveDetectedFaces Output save flag, 0 for saving off and 1 or 2 for saving on
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getSaveDetectedFaceFlag(int* psaveDetectedFaces);

//! Get name of the detected face image saved during last recognition call
/*!
\param pImageName Output image name. Should be allocated 2048 bytes for ex "char pImageName[2048]"
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getSaveDetectedImageName(char pImageName[]);


//! Get name of the detected face WFG image saved during last recognition call
/*!
\param pImageName Output image name. Should be allocated 2048 bytes for ex "char pImageName[2048]"
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getDetectedWfgName(char pImageName[]);

//! Set Verbose for SDK to enable debugging prints
/*!
\param isEnabled Set 1 to enable prints from SDK, set to 0 to disable prints
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setVerbose(const char* basepath, int isEnabled);

//! Enable input image saving for debugging
/*!
\param enableSaving Set 1 to enable image saving in SDK, set to 0 to disable
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_enableImageSaveForDebugging(int enableSaving);


//! Enable or Disable enroll image saving
/*!
\param handle the Engine's handle. Handle can be NULL for this API
\param enableSaving Set 1 to enable enroll image saving and 0 to disable enroll image saving
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_saveEnrollImages(void* handle, int enableSaving);


//! Set engine mode to enable/disable update from PC database (wffrdbpc)
/*!
\param enablePCDB Set to 1 to enable update from PCDB, set to 0 to disable update from PCDB. It is enabled by default
\param maxPCDBCount Set maximum allowed records update at a time.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setUpdateFromPCDB(int enablePCDB, int maxPCDBCount);

//! Get engine mode is enabled or disabled for update from PC database (wffrdbpc)
/*!
\param penablePCDB Output PCDB update is enabled or disabled.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getUpdateFromPCDB(int* penablePCDB);


//! Set flag to enable deletion of existing ID's with same name when updated from PC database
/*!
\param enableDelete Set to 1 to enable delete, set to 0 to disable delete. 
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setDeleteExistingNamePCDBUpdate(int enableDelete);

//! Get flag for enable/disable deletion of existing ID's with same name when updated from PC database
/*!
\param penableDelete Output flag for enabled/disabled flag for deleting existing ID's.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getDeleteExistingNamePCDBUpdate(int* penableDelete);



//! Set flag to enable deletion of all existing ID's with same name when enrolling on device
/*!
\param enableDelete Set to 1 to enable delete, set to 0 to disable delete.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setDeleteExistingNameInEnrolling(int enableDelete);

//! Get flag for enable/disable deletion of existing ID's with same name when enrolling on device
/*!
\param penableDelete Output flag for enabled/disabled flag for deleting existing ID's.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getDeleteExistingNameInEnrolling(int* penableDelete);


//! Get status of enroll image saving
/*!
\param handle the Engine's handle. Handle can be NULL for this API
\param saveEnrollImagesStatus Output enroll image saving status, 1 if it is enabled and 0 if it is disabled.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getSaveEnrollImagesStatus(void* handle, int *saveEnrollImagesStatus);

//! Set base path for Database folder "wffrdb" and "wffrdbpc". It should be called before wfFR_Init() API
/*!
\param pDbBasePath Input base directory folder for database
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setDatabaseFolderPath(const char* pDbBasePath);

//! Get base path for Database folder "wffrdb" and "wffrdbpc"
/*!
\param pDbBasePath Output base directory folder for database
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getDatabaseFolderPath(char* pDbBasePath);


//! Set engine to enable quality check of enroll face like face angle is frontal. If it is enable, rejected faces will have confidence = -1. 
/*!
\param enablecheck Set to 1 or 2 or 3 to enable quality check, set to 0 to disable. 
				1 will enable relaxed quality check
				2 will enable very strict quality check
				3 will enable only face angle check.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setEnrollQualityCheckFlag(int enablecheck);

//! Get flag for quality check of enroll face like face angle. If it is enable, rejected faces will return confidence = -1.
/*!
\param penablecheck Output, 1 or 2 or 3 if quality check enabled, 0 if disable
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getEnrollQualityCheckFlag(int* penablecheck);

//! Get failure code for enrolling
/*!
\param penrollfailcode Output failure code, 0 - pass, 1 - not detected, 2 -multiface, 3 - spooffail, 4 - lowcontrast, 
					   5 - faceangle, 6 - blur, 7 - lowlight, 8 - blur, 9 - similarface in db, 10 - multienroll
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getEnrollFailureCode(int* penrollfailcode);


//! Set engine to enable quality check of recognition face like face angle is frontal. If it is enable, rejected faces will have confidence = -1. 
/*!
\param enablecheck Set to 1 or 2 or 3 to enable quality check, set to 0 to disable. 
				1 will enable relaxed quality check
				2 will enable very strict quality check
				3 will enable only face angle check.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setRecogQualityCheckFlag(int enablecheck);

//! Get flag for quality check of recognition face like face angle. If it is enable, rejected faces will return confidence = -1.
/*!
\param penablecheck Output, 1 or 2 or 3 if quality check enabled, 0 if disable
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getRecogQualityCheckFlag(int* penablecheck);


//! Set engine to enable automatic online license generation. Warning: Once enabled license will be automatically generated on wfFR_Init.
/*!
\param enable Set to 1 to enable and 0 to disable online licensing
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setOnlineLicensing(int enable);

//! Get flag engine to enable automatic online license generation. Warning: Once enabled license will be automatically generated on wfFR_Init.
/*!
\param penable Output, 1 if enabled and 0 if disable online licensing
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getOnlineLicensingFlag(int* penable);


/*! Rotate face by 90, 180 or 270 degrees
  \param pFrameData input image's pixel data. Gray image / YUV420sp / NV21 / NV12 / YV12 supported
  \param iFrameWid input image's width
  \param iFrameHig input image's height
  \param iFrameStep input image's width step
  \param pFrameDataOut output image's pixel data .Gray image / YUV420sp / NV21 / NV12 / YV12 supported
  \param imageFormat Input image format - 0 for single channel gray image, 1 for YUV420/NV21 format, 2 for YV12 format
  \param iRotationAngle input angle of rotation. It can be 90, 180 or 270 degrees only.
  \param iMirrorImage mirror about Y axis. 0 to disable and 1 to enable.
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/

WFFR_API int wfFR_rotateImage( unsigned char* pFrameData,  int iFrameWid, int  iFrameHig, int iFrameStep, unsigned char* pFrameDataOut, ImageFormat imageFormat, int iRotationAngle, int iMirrorImage);


/*! Swap Y pixels. It will swap input pixels {y1,y2,y3,y4,y5,y6...} to {y2,y1,y4,y3,y6,y5...}
  \param pFrameData input image's pixel data (Y channel). Only Gray image / Y channel supported
  \param iFrameWid input image's width
  \param iFrameHig input image's height
  \param iFrameStep input image's width step
  \param pFrameDataOut output image's pixel data (Y channel). Only Gray image / Y channel supported
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/

WFFR_API int wfFR_swapImagePixels( unsigned char* pFrameData,  int iFrameWid, int  iFrameHig, int iFrameStep, unsigned char* pFrameDataOut);


/*! Resize image
  \param pFrameData input image's pixel data (Y channel). Only Gray image / Y channel supported
  \param iFrameWid input image's width
  \param iFrameHig input image's height
  \param iFrameStep input image's width step
  \param pFrameDataOut output image's pixel data (Y channel). Only Gray image / Y channel supported
  \param oFrameWid output image's width
  \param oFrameHig output image's height
  \param oFrameStep output image's width step
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/

WFFR_API int wfFR_resizeImage( unsigned char* pFrameData,  int iFrameWid, int  iFrameHig, int iFrameStep, unsigned char* pFrameDataOut, int oFrameWid, int oFrameHig, int oFrameStep);


/*! Detect low light condition in image.
  \param pFrameData input image's pixel data (Y channel). Only Gray image / Y channel supported
  \param iFrameWid input image's width
  \param iFrameHig input image's height
  \param iFrameStep input image's width step
  \param lightStrength input strength of light to control low light output. Range [0,25], Set Default = 5. Lower values will detect low light in more darker conditions. Higher value will detect low light in brighter conditions
  \param pIsLowLight output low light detection, 1 if low light is detected and 0 if not detected
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_lowLightDetection(unsigned char* pFrameData,  int iFrameWid, int  iFrameHig, int iFrameStep, int* pIsLowLight, int lightStrength);


//! Set miscellaneous parameters in SDK . 
/*!
\param mparams Structure containing parameters for sdk.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setMiscParameters(FRMiscParams mparams);

//! Get miscellaneous parameters in SDK . 
/*!
\param pmparams Output Structure containing parameters for sdk.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getMiscParameters(FRMiscParams* pmparams);


//! Set Enroll Quality parameters in SDK . 
/*!
\param mparams Structure containing enroll quality parameters for sdk.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setEnrollQualityParameters(FREnrollQualityParams mparams);

//! Get Enroll Quality parameters in SDK . 
/*!
\param pmparams Output Structure containing  enroll quality parameters for sdk.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getEnrollQualityParameters(FREnrollQualityParams* pmparams);


//! Set Recognition Quality parameters in SDK . 
/*!
\param mparams Structure containing Recognition quality parameters for sdk.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setRecogQualityParameters(FRRecogQualityParams mparams);

//! Get Recognition Quality parameters in SDK . 
/*!
\param pmparams Output Structure containing Recognition quality parameters for sdk.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getRecogQualityParameters(FRRecogQualityParams* pmparams);

//! Get face signature size
/*!
\param handle the Engine's handle
\param pSigSizeBytes Signature size in bytes
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRREC_API int wfFR_getFaceSignatureSize(void* handle, int* pSigSizeBytes);


/*! Calculate and return face signature
  \param handle the Engine's handle
  \param pImageData query image's pixel data in NV21 format. 
  \param iImageWid query image's width
  \param iImageHig query image's height
  \param iImageStep query image's width step
  \param pSig Output face signature. Memroy should be allocated by application, size of memory is same as in wfFR_getFaceSignatureSize
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_calcFaceSignature(void* handle, const unsigned char* pImageData, int iImageWid, int iImageHig, int iImageStep, char* pSig, int* sigsuccess);


/*! Get signature for face from dual camera
  \param handle the Engine's handle
  \param pFrameDataColor query 1st cam image's pixel data (Y / NV21). Only Gray image / Y channel / NV21 supported
  \param pFrameDataIR query 2nd cam image's pixel data (Y / NV21). Only Gray image / Y channel / NV21 supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param pResult1 Output search results for 1st cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pResult2 Output search results for 2nd cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \param pSig Output face signature. Memroy should be allocated by application, size of memory is same as in wfFR_getFaceSignatureSize
  \param sigsuccess Output is 1 if signature calculation success, 0 otherwise.
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_calcFaceSignatureDualCam(void* handle, const unsigned char* pFrameDataColor, const unsigned char* pFrameDataIR, int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResultColor, FRIDList* pResultIR, int* pStatus, char* pSig, int* sigsuccess);

/*! Compare two face signatures and return matching confidence
  \param handle the Engine's handle
  \param pSig1 Signature input 1
  \param pSig1 Signature input 2
  \param pConfidence Output matching confidence
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_compareFaceSignatures(void* handle, char* pSig1, char* pSig2, float* pConfidence);



#ifndef __ANDROID__
#ifdef __cplusplus
}
#endif
#endif


#endif //_FRLIB_H




