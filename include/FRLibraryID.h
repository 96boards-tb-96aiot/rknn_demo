#ifndef _FRLIB_ID_H
#define _FRLIB_ID_H

#ifdef _WIN32
		#ifdef WFFR_EXPORTS
			#define WFFRID_API __declspec(dllexport)
		#else
			#define WFFRID_API __declspec(dllimport)
		#endif
//#define WFFRID_API
#define WINAPI //__stdcall
#else
	#if  (WFFR_ENABLE_PCENROLL_ONLY == 1)
		#define WFFRID_API __attribute__ ((visibility ("hidden")))
	#else
		#define WFFRID_API __attribute__ ((visibility ("default")))
	#endif
#define WINAPI
#endif

#define FR_OK 0
#define FR_ERROR_GENERAL 1
#define FR_ERROR_BAD_ARGS 2
#define FR_ERROR_LICENSE 3
#define FR_ERROR_NOTSUPPORTED 4
#define FR_ERROR_FRMODE 5
#define FR_ERROR_FACENOTDETECTED 6
#define FR_ERROR_TEMPLATENOTFOUND 7
#define FR_ERROR_VERIFYFORENROLL 8
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

//! Verify permanent license.
/*!
  \ingroup INI
  \param pBasePath Base path of the folder which contains all data files
*/
WFFRID_API int wfFRID_VerifyLic(const char* pBasePath);

//! Create new instance of FR Engine
/*!
  \ingroup INI
  \param pHandle pointer to the handle to be initialized
  \param pBasePath Base path of the folder which contains all data files
  \param enableAntiSpoofing Flag to enable anti-spoofing for face. Set 1 to enable and 0 to disable
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRID_API int wfFRID_Init(void** pHandle, const char* pBasePath, int enableAntiSpoofing = 0);



//! Destroy the instance of Engine and save database
/*!
  \param pHandle pointer to the handle to be freed
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRID_API int wfFRID_Release(void** pHandle);


/*! Recognize face from video
  \param handle the Engine's handle
  \param pFrameData query image's pixel data (Y channel). Only Gray image / Y channel supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRID_API int wfFRID_RecognizeFaces(void* handle, const unsigned char* pFrameData, int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResult, int* spoofStatus);



/*! Recognize face from dual camera
  \param handle the Engine's handle
  \param pFrameDataColor query 1st cam image's pixel data (Y channel). Only Gray image / Y channel supported
  \param pFrameDataIR query 2nd cam image's pixel data (Y channel). Only Gray image / Y channel supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param pResult1 Output search results for 1st cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pResult2 Output search results for 2nd cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRID_API int wfFRID_RecognizeFacesDualCam(void* handle, const unsigned char* pFrameDataColor, const unsigned char* pFrameDataIR, int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResultColor, FRIDList* pResultIR, int* pStatus);




/*! Recognize face saved image like jpeg/bmp/png
  \param handle the Engine's handle
  \param pImageFilename Image file name stored on device like  jpeg/bmp/png
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRID_API int wfFRID_RecognizeFacesFromSavedImage(void* handle, const char* pImageFilename, FRIDList* pResult, int* spoofStatus);


/*! Recognize face from Jpeg buffer
  \param handle the Engine's handle
  \param pJpegBuffer Input image Jpeg buffer data
  \param iJpegBufferSize Input image Jpeg buffer data size in bytes
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRID_API int wfFRID_RecognizeFacesFromJpegBuffer(void* handle, const char* pJpegBuffer, int iJpegBufferSize, FRIDList* pResult, int* spoofStatus);



/*! Enroll face from video/image
  \param handle the Engine's handle
  \param pImageData query image's pixel data (Y channel). Only Gray image / Y channel supported
  \param iImageWid query image's width
  \param iImageHig query image's height
  \param iImageStep query image's width step
  \param enrollStatus will be set to 1 if enrolling was successful.
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRID_API int wfFRID_EnrollID(void* handle, const unsigned char* pImageData, int iImageWid, int iImageHig, int iImageStep, int* enrollStatus);


/*! Enroll face from saved image like jpeg/bmp/png
  \param handle the Engine's handle
  \param pImageFilename Image file name stored on device like  jpeg/bmp/png
   \param enrollStatus will be set to 1 if enrolling was successful.
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRID_API int wfFRID_EnrollIDFromSavedImage(void* handle,  const char* pImageFilename, int* enrollStatus);


/*! Enroll face from Jpeg buffer
  \param handle the Engine's handle
  \param pJpegBuffer Input image Jpeg buffer data
  \param iJpegBufferSize Input image Jpeg buffer data size in bytes
  \param enrollStatus will be set to 1 if enrolling was successful.
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRID_API int wfFRID_EnrollIDFromJpegBuffer(void* handle,  const char* pJpegBuffer, int iJpegBufferSize, int* enrollStatus);

/*! Verify if image is suitable for enrollment from Jpeg buffer
  \param handle the Engine's handle
  \param pJpegBuffer Input image Jpeg buffer data
  \param iJpegBufferSize Input image Jpeg buffer data size in bytes
  \param isIRImage Input image is from IR or color camera. Set to 1 for IR camera, set to 0 otherwise
  \param enrollStatus will be set to 1 if image is verified for enrolling.
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRID_API int wfFRID_VerifyImageForEnrollJpegBuffer(void* handle,  const char* pJpegBuffer, int iJpegBufferSize, int isIRImage, int* enrollStatus);

//! Set Face Spoof detection sensitivity
/*!
\param handle the Engine's handle
\param sensitivity Sensitivity for face spoof. Range [0,2]. Default is 1 (medium). 
				0 is low (difficult), 
				1 is medium 		
				2 is high sensitivity (easy)
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRID_API int wfFRID_setSpoofingSensitivity(void* handle, int sensitivity);


//! Get Face Spoof detection sensitivity
/*!
\param handle the Engine's handle
\param psensitivity Output sensitivity for face spoof. Range [0,2]. 
				0 is low (difficult), 
				1 is medium 		
				2 is high sensitivity (easy)
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRID_API int wfFRID_getSpoofingSensitivity(void* handle, int* psensitivity);


//! Set Face Recognition confidence threshold
/*!
\param confidenceThresold Confidence threshold to be set for for face recognition. Range is [30,100], Default is 70
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRID_API int wfFRID_setRecognitionThreshold(float confidenceThresold);


//! Get Face Recognition confidence threshold
/*!
\param pconfidenceThresold Output confidence threshold for face recognition
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRID_API int wfFRID_getRecognitionThreshold(float* pconfidenceThresold);


//! Set High resolution ID check mode
/*!
\param handle the Engine's handle
\param isHighResMode Set to 1 to enable and 0 to disable
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRID_API int wfFRID_setHighResolutionMode(int isHighResMode);


//! Get  High resolution ID check mode
/*!
\param handle the Engine's handle
\param pisHighResMode Output 1 if high res is enabled, 0 otherwise
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRID_API int wfFRID_getHighResolutionMode(int* pisHighResMode);


#ifndef __ANDROID__
#ifdef __cplusplus
}
#endif
#endif

#endif //_FRLIB_ID_H



