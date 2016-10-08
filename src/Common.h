/*
 * Common.h
 */

#include <Hs.h>
#include <PalmOS.h>
#include <Form.h>
#include <palmOneResources.h>
 
#define MIN_VERSION  						sysMakeROMVersion(5, 0, 0, sysROMStageRelease, 0)
#define LAUNCH_FLAGS 						(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)

#define PWD_LENGTH							80 	// should be multiple of 8
#define KEY_LENGTH							8 	// should be multiple of 8
#define HSSN_PTR_OFFESET					4 //  offset for copying key from

// the following for registration...
#define HSSN_LENGTH							12
#define KEY_LENGTH							8
#define HEX_KEY_LENGTH						KEY_LENGTH * 2

typedef struct {
	UInt8		pwd[PWD_LENGTH];
	UInt8		key[KEY_LENGTH];
} AppSecurityType;

typedef enum {
	decrypt = 0,
	encrypt = 1
} CryptAction_e;

/* Compiler options **************************************************/
// #define ERROR_CHECK_FULL 				// something...
// #define WITH_REGISTRATION
#define BUILD_FOR_TREO_600 					// Treo 600 specific build
#define BUILD_FOR_TREO_680
#define BUILD_FOR_TREO_700p
/***********************************************************************/

#define ALERT_MEMORY_ERROR					"Memory Error"
#define ALERT_PHONE_ERROR					"Phone Error"

#define appExpiryFileCreator				'sysU'

#define EXPIRY_SECOND						0
#define EXPIRY_MINUTE						0
#define EXPIRY_HOUR							0
#define EXPIRY_DATE							31
#define EXPIRY_MONTH						3
#define EXPIRY_YEAR							2007

// 25 calls per day * 30 days * 3 months * 10 notifications per call = ~22500
#define MAX_RUNS 							20000

typedef struct {
	UInt32									expiryDate;
	UInt32									timeLastUsed;
	UInt32									maxRuns;
	UInt32									numRuns;
} AppExpiryPrefs_t;

#define VENDOR_NAME							"www.swCP3.com"
// #define VENDOR_NAME							"www.MobiHand.com"
// #define VENDOR_NAME							"www.MyTreo.net"
// #define VENDOR_NAME							"www.Handango.com"
// #define VENDOR_NAME							"www.PalmGear.com"
// #define VENDOR_NAME							"www.Softonic.com"	

/*
 * Miscellaneous stuff
 */
 
typedef enum {
	phnMute = 0,
	phnNormal = 1
} MuteAction_e;

typedef struct { 
	Boolean									bEnabled; // 1
	UInt16									timeMuteIdx; // 2
	UInt16									optionMaskIdx; // 2
	UInt16									keyPressIdx; // 2
	Boolean									strRegKey[HEX_KEY_LENGTH + 1]; // 17
	Boolean									bDisplayTimeSelection; // 1
	Boolean									bSystemSnd; // 1
	Boolean									bAlarmSnd; // 1
	Boolean									bGameSnd; // 1
	Boolean									bPhoneSnd; // 1
	Boolean									bMessagingSnd; // 1
	Boolean									bCalendarSnd; // 1
	Boolean									bBeepConfirmation; // 1
	Boolean									bVibrationON; // 1
	UInt8									junk[7]; // Total 33 + 7 bytes // to make AppPreferencesType an integral multiple of 8, for encryption
} AppPreferences_t;

#define PREFS_ENABLED						true
#define PREFS_MUTED							false
#define PREFS_TIME_MUTE_IDX					0
#define PREFS_OPTION_MASK_IDX				2
#define PREFS_KEYPRESS_IDX					2
#define PREFS_REG_KEY_STRING				"*NOT REGISTERED*"
#define PREFS_DISPLAY_TIME_SELECTION		true
#define PREFS_SYSTEM						true
#define PREFS_ALARM							true
#define PREFS_GAME							true
#define PREFS_PHONE							true
#define PREFS_MESSAGING						true
#define PREFS_CALENDAR						true
#define PREFS_BEEP_CONFIRMATION				true
#define PREFS_VIBRATION_ON					true

typedef struct {
	Boolean									bMuted;
	UInt16									sndLevelSystem;
	UInt16									sndLevelAlarm;
	UInt16									sndLevelGame;
	UInt16									phnAlertVolume;
	UInt16									smsAlertVolume;
	UInt16									calAlertVolume;
} USPrefs_t;

#define US_PREFS_MUTED						false
#define US_PREFS_SYS_VOLUME					sndMaxAmp / 4
#define US_PREFS_ALARM_VOLUME				sndMaxAmp / 4
#define US_PREFS_GAME_VOLUME				sndMaxAmp / 4
#define US_PREFS_PHN_ALERT_VOLUME			toneVolume4
#define US_PREFS_SMS_ALERT_VOLUME			toneVolume4
#define US_PREFS_CAL_ALERT_VOLUME			toneVolume4

#define DISP_WIDTH							160
#define DISP_HEIGHT							DISP_WIDTH

#define LINE_GAP							2

#define RECT_WIDTH_SMALL					58
#define RECT_HEIGHT_SMALL					16

#define RECT_WIDTH_BIG						110
#define RECT_HEIGHT_BIG						30

#define TXT_OFF_X							5
#define TXT_OFF_Y							2

#define STR_ON_AT							"On at: "
#define STR_CANCELLED						"CANCELLED"

#define POPUP_FORM_WAIT_TIME				3

/*
 * Common.h
 */
