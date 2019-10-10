#ifndef _FRLIB_TYPES_H
#define _FRLIB_TYPES_H


#define IDLISTMAX 10					/*!< Maximum number of matched faces output */
#define FRINFO_STRING_SIZE 128				/*!< Length of string storing first/second name of person */

/*! Structure to store face ROI */
typedef struct _FRFace
{
	int 	x;					/*!< Face ROI top left x */
	int 	y;					/*!< Face ROI top left y */
	int 	width;					/*!< Face ROI width */
	int 	height;					/*!< Face ROI height */
}FRFace;

/*! Structure to store FR output */
typedef struct _FRIDList
{
    FRFace	  pFace[IDLISTMAX];			 /*!< Face ROI */
    char 	  pFName[IDLISTMAX][FRINFO_STRING_SIZE]; /*!< Person's first name */
    char 	  pLName[IDLISTMAX][FRINFO_STRING_SIZE]; /*!< Person's last name */
    float         pConfidence[IDLISTMAX]; 		 /*!< confidences */
    unsigned long pRecordID  [IDLISTMAX]; 		 /*!< record ids, or ULONG_MAX in case of unknown */
    float	  pValues[IDLISTMAX][1];		 /*!< misc data values */
    int		  pTrackID[IDLISTMAX];		 	 /*!< Face track ID*/
    int           nResults;                     	 /*!< number of results */
}FRIDList;

/*! FR running mode - recognition or enrolling */
typedef enum _FRMode
{
	FRMODE_RECOGNIZE = 0,				/*!< Recognition mode */
	FRMODE_ENROLL = 1					/*!< Enrolling mode */
}FRMode;

/*! Input Image format */
typedef enum _ImageFormat
{
	IMAGE_GRAY = 0,				/*!< Gray or Y image, single channel only */
	IMAGE_YUV420 = 1,			/*!< YUV420sp or NV21 format */
	IMAGE_YV12 = 2,				/*!< YV12 format */
	IMAGE_BGR = 3,				/*!< BGR format, three channels */
	IMAGE_BGRA = 4,				/*!< BGRA/RGBA format, four channels */
	IMAGE_YUV420_NV12 = 5,			/*!< YUV420sp or NV12 format */
	IMAGE_YV12_INV = 6,			/*!< YV12 format with inverted colors*/
}ImageFormat;

typedef struct _FRMiscParams
{
	int enrollSaveImageCropSize;		/* 0 for no cropping, 1 for default size (75% boundary), 2 for 25% extra boundary, 3 for 10% extra boundary*/
	int enrollSaveImageIsColor;		/* 0 to save gray image, 1 to save color image in enrolled database. */
	int enrollMultiFaceCheck;		/*check if multiple person are present in enrolling. 0-disable, 1-enable, 
										2-strong enable: discard enroll if multiface in even single frame
										3-stronger enable: discard enroll different face found*/
	int enableDbBackup;			/* 0 to disable DB backup file, 1 to enable DB back file. */
	int enablePCEnrollImageCopy;		/* 1 to enable copy of image to DB (wffrdb) when enrolling from PC DB (wffrdbpc). 0 to disable*/
	int singlecamSpoofRecoverTime;		/* Time to recover from spoof attack. Default is 3 seconds. Range is [1,3]*/
	int enableSinglecamAntiSpoofBlock;	/* 1 to enable and 0 to disable. When enabled, FR will be disabled for few seconds if spoof attack is detected.*/
	int enableSinglecamBGReject;		/* 1 to enable and 0 to disable. When enabled, singlecam spoof will use background to reject spoof*/
	int enableDualcamBGReject;		/* 1 to enable and 0 to disable. When enabled, dualcam spoof will use background to reject spoof*/
}FRMiscParams;

typedef struct _FREnrollQualityParams
{
	float leftRightYaw;			/* left-right Yaw angle allowed. Default is 0.33f. Range is [0.1, 0.7]. Higher values will pass more frontal faces */
	float upDownPitch;			/* up-down pitch angle allowed. Default is 0.3. Range is [0.1, 0.7]. Higher values will pass more frontal faces */
	float inPlaneRoll;			/* In plane roll angle allowed. Default is 0.25. Range is [0.1, 0.7]. Lower values will pass more frontal faces */

	float maxBlur;				/* Maximum blur in face. Default is 120. Range is [20, 1000]. Higher value will pass higher blur. */
	float minContrast;			/* Minimum face contrast. Default is 9. Range is [1,50]. Higher value will pass higher contrast. */
	float minFaceAvg;			/* Minimum face intensity average value. Default is 60. Range is [10,150]. Higher values will pass brighter faces. */

}FREnrollQualityParams;

typedef struct _FRRecogQualityParams
{
	float leftRightYaw;			/* left-right Yaw angle allowed. Default is 0.55f. Range is [0.1, 0.7]. Higher values will pass more frontal faces */
	float upDownPitch;			/* up-down pitch angle allowed. Default is 0.3. Range is [0.1, 0.7]. Higher values will pass more frontal faces */
	float maxBlur;				/* Maximum blur in face. Default is 120. Range is [20, 1000]. Higher value will pass higher blur. */
}FRRecogQualityParams;

#endif //_FRLIB_TYPES_H
