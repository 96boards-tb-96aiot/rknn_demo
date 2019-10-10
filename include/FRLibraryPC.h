#ifndef _FRLIBPC_H
#define _FRLIBPC_H

#ifndef WFFRPC_API
	#ifdef _WIN32
		#ifdef WFFRPC_EXPORTS
			#define WFFRPC_API __declspec(dllexport)
		#else
			#define WFFRPC_API __declspec(dllimport)
		#endif
	#else
		#define WFFRPC_API __attribute__ ((visibility ("default")))
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
WFFRPC_API const char* wfFRPC_GetVersionInfo();

//! Create new instance of FR Engine
/*!
  \ingroup INI
  \param pHandle pointer to the handle to be initialized
  \param pBasePath Base path of the folder which contains all data files
  \param firstName First name of the person
  \param secondName Second name of the person
  \param enableAntiSpoofing Flag to enable anti-spoofing for face. Set 1 to enable and 0 to disable
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRPC_API int wfFRPC_EnrollInit(void** pHandle, const char* pBasePath,  const char* firstName, const char* secondName, int enableAntiSpoofing = 0);



//! Destroy the instance of Engine and save database
/*!
  \param pHandle pointer to the handle to be freed
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRPC_API int wfFRPC_Release(void** pHandle);


//! Merge PC database (wffrdbpc) to database from device (wffrdb) while processing on PC
/*!
  \param pBasePath Base path of the folder which contains all data files
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRPC_API int wfFRPC_mergePCDB(const char* pBasePath);


/*! Enroll face from video/image
  \param handle the Engine's handle
  \param pImageData query image's pixel data (Y channel). Only Gray image / Y channel supported
  \param iImageWid query image's width
  \param iImageHig query image's height
  \param iImageStep query image's width step
  \param pResult search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRPC_API int wfFRPC_EnrollFace(void* handle, const unsigned char* pImageData, int iImageWid, int iImageHig, int iImageStep, FRIDList* pResult, int* pStatus);


/*! Enroll face from saved image like jpeg/bmp/png
  \param handle the Engine's handle
  \param pImageFilename Image file name stored on device like  jpeg/bmp/png
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRPC_API int wfFRPC_EnrollFaceFromSavedImage(void* handle,  const char* pImageFilename, FRIDList* pResult, int* pStatus);


/*! Enroll face from Jpeg buffer
  \param handle the Engine's handle
  \param pJpegBuffer Input image Jpeg buffer data
  \param iJpegBufferSize Input image Jpeg buffer data size in bytes
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRPC_API int wfFRPC_EnrollFaceFromJpegBuffer(void* handle,  const char* pJpegBuffer, int iJpegBufferSize, FRIDList* pResult, int* spoofStatus);


/*! Enroll face from single camera with image anti-spoof
  \param handle the Engine's handle
  \param pFrameData query 1st cam image's pixel data (Y channel). Only Gray image / Y channel supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param lRecordID record ID to be trained with new images
  \param pResult Output results for 1st cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRPC_API int wfFRPC_EnrollFaceSingleCamSpoof(void* handle, const unsigned char* pFrameData,  int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResult, int* spoofStatus);


/*! Verify Enrolling face image from saved image like jpeg/bmp/png
  \param handle the Engine's handle
  \param pImageFilename Image file name stored on device like  jpeg/bmp/png
  \param pStatus Output enroll verify results, 1 if face in image is OK to enroll and 0 if image cannot be used for enrolling
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRPC_API int wfFRPC_VerifyEnrollFaceFromSavedImage(void* handle,  const char* pImageFilename, int* pVerifyStatus);


//! Set Face Spoof detection sensitivity
/*!
\param handle the Engine's handle
\param sensitivity Sensitivity for face spoof. Range [0,2]. Default is 1. 0 is low, 1 is medium and 2 is high sensitivity for spoof detection.
*/
WFFRPC_API int wfFRPC_setSpoofingSensitivity(void* handle, int sensitivity);


//! Get Face Spoof detection sensitivity
/*!
\param handle the Engine's handle
\param psensitivity Output sensitivity for face spoof. Range [0,2]. 0 is low, 1 is medium and 2 is high sensitivity for spoof detection.
*/
WFFRPC_API int wfFRPC_getSpoofingSensitivity(void* handle, int* psensitivity);


//! Set Face detection minimum size in percentage. It should be called before wfFR_Init.
/*!
\param handle the Engine's handle
\param minfacesize Minimum face detection size in percentage of input image dimentions. Range is [10,100]
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRPC_API int wfFRPC_setMinFaceDetectionSizePercent(int minfacesize);

//! Get Face detection minimum size in percentage.
/*!
\param handle the Engine's handle
\param pminfacesize Output minimum face detection size in percentage of input image dimentions.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRPC_API int wfFRPC_getMinFaceDetectionSizePercent(int* pminfacesize);


//! Set Face detection minimum size in pixels. It should be called before wfFR_Init.
/*!
\param handle the Engine's handle
\param minfacesize Minimum face detection size in percentage of input image dimentions. Range is [50,1000]
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRPC_API int wfFRPC_setMinFaceDetectionSizePixels(int minfacesizePixels);

//! Get Face detection minimum size in pixels.
/*!
\param handle the Engine's handle
\param pminfacesize Output minimum face detection size in percentage of input image dimentions.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRPC_API int wfFRPC_getMinFaceDetectionSizePixels(int* pminfacesizePixels);


//! Set engine to enable quality check of enroll image like face angle is frontal. If it is enable, rejected faces will have confidence = -1. 
/*!
\param enablecheck Set to 1 or 2 to enable quality check, set to 0 to disable. Setting to 1 will enable relaxed quality check and 2 will enable very strict quality check
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRPC_API int wfFRPC_setEnrollQualityCheckFlag(int enablecheck);


//! Get flag for quality check of enroll image like face angle. If it is enable, rejected faces will return confidence = -1.
/*!
\param penablecheck Output, 1 or 2 if quality check enabled, 0 if disable
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRPC_API int wfFRPC_getEnrollQualityCheckFlag(int* penablecheck);


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

WFFRPC_API int wfFRPC_resizeImage( unsigned char* pFrameData,  int iFrameWid, int  iFrameHig, int iFrameStep, unsigned char* pFrameDataOut, int oFrameWid, int oFrameHig, int oFrameStep);

#ifndef __ANDROID__
#ifdef __cplusplus
}
#endif
#endif


#endif //_FRLIBPC_H



