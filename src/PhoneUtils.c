/*
 * PhoneUtils.c
 */
 
#include <Hs.h>
#include <HsPhone.h>
#include <HsNav.h>
#include <HsExt.h>
#include <palmOneResources.h>
#include <Form.h>

#include "Global.h"
#include "AppResources.h"

#define SND1_FREQUENCY			500 // 1760
#define SND1_DURATION			80

#define SND2_FREQUENCY			500 // 1760
#define SND2_DURATION			80

#define PAUSE_FREQUENCY			1 // 0 doesn't play any sound // we don't use this anyway
#define PAUSE_DURATION			80

// Prototypes
void 				DelayTask(UInt32 DelaySeconds);
void 				getROMID(UInt8* keyVal);
void 				beep(UInt8 numBeeps);
Boolean 			IsPhoneGSM(void);
void 				Alert(Char* MsgType, Char* Msg, Err err);
Err 				RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags);

/* 
 * DelayTask
 */
void DelayTask(UInt32 DelaySeconds)
{
	UInt32	TimeSleep = TimGetSeconds() + DelaySeconds;
									
	while (TimGetSeconds() < TimeSleep)
		;
		
	return;
} // DelayTask

/*
 * Alert
 */
void Alert(Char* MsgType, Char* Msg, Err err)
{
  	Char	errStr[32];

  	StrPrintF(errStr, "0x%x", err);
  	FrmCustomAlert(ALERT_BOX, MsgType, Msg, errStr);

} // Alert

/*
 * beep
 */
void beep(UInt8 numBeeps)
{
	SndCommandType 			sndCmd;
	UInt16					SndVolume = PrefGetPreference(prefSysSoundVolume);

	UInt8					i = 0;
	
	sndCmd.cmd = sndCmdFreqDurationAmp; 				
	sndCmd.param1 = SND1_FREQUENCY; // 1760; 
	sndCmd.param2 = SND1_DURATION;
	sndCmd.param3 = SndVolume;
	SndDoCmd(0, &sndCmd, true);	
	
	for (i = 1 ; i < numBeeps ; i++) // i = 1 because we already beeped once
	{
		sndCmd.cmd = sndCmdFreqDurationAmp; 				
		// sndCmd.param1 = PAUSE_FREQUENCY; // 1760; // this don't work
		sndCmd.param2 = PAUSE_DURATION;
		sndCmd.param3 = 0;
		SndDoCmd(0, &sndCmd, true);	
		
		sndCmd.cmd = sndCmdFreqDurationAmp;				
		sndCmd.param1 = SND2_FREQUENCY; // 1760; 
		sndCmd.param2 = SND2_DURATION;
		sndCmd.param3 = SndVolume;
		SndDoCmd(0, &sndCmd, true);
	}
}

/*
 * IsPhoneGSM
 */
Boolean IsPhoneGSM(void)
{
  	UInt32 phnType = hsAttrPhoneTypeGSM; 
  
  	HsAttrGet(hsAttrPhoneType, 0, &phnType);
  	return (phnType == hsAttrPhoneTypeGSM);

} // IsPhoneGSM

/*
 * getROMID
 */
void getROMID(UInt8* keyVal)
{
	Err 		error;
	
	Char 		bufP[32]; 
	UInt16 		bufLen = sizeof(bufP);	
	Char*		altKey = "TreoMuTe";
 
	error = HsGetVersionString(hsVerStrSerialNo, bufP, &bufLen);

	if (error)
		MemMove(keyVal, altKey, 8); // "TreoMuTe"
	else
		MemMove(keyVal, bufP + HSSN_PTR_OFFESET, 8);	// copy from 5th to 12th location of 12 digit Ser Num.
									  					// they are more likely to be different.
		
	return;
	
} // getROMID

/*
 * RomVersionCompatible
 */
Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{	
	UInt32 	rom = 0;
	UInt32 	hal = 0;
	UInt32	company = 0;
	UInt32	device = 0;
	UInt32	hsFtrVersion;

	if(FtrGet(hsFtrCreator, hsFtrIDVersion, &hsFtrVersion) != 0)
		return(1);

    FtrGet(sysFtrCreator, sysFtrNumOEMHALID, &hal);
    FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &company);
    FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &device);
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &rom);
		
	if (rom >= requiredVersion
		&&	(company == kPalmCompanyIDPalm
			|| company == kPalmCompanyIDOldPalm
			|| kPalmCompanyIDHandspring)

		&&	(device == kPalmOneDeviceIDTreo650
			|| device == kPalmOneDeviceIDTreo650Sim

			|| device == kPalmOneDeviceIDTreo600
			|| device == kPalmOneDeviceIDTreo600Sim

			|| device == 'D053' // Treo 680
			|| device == 'H104'

			|| device == 'D052' // Treo 700p
			|| device == 'H103'

			|| device == 'D060' // Treo 755p
			|| device == 'H104'
			
			|| device == 'D061' // Centro ???
			|| device == 'D062' // Centro
			)
			
		&&	(hal == hsHALIDHandspringOs5Rev2 // Treo 650
			|| hal == hsHALIDHandspringOs5Rev2Sim

			|| hal == hsHALIDHandspringOs5Rev1 // Treo 600
			|| hal == hsHALIDHandspringOs5Rev1Sim

			|| hal == hsHALIDHandspringOs5Rev4 // Treo 680, 755p
			|| hal == hsHALIDHandspringOs5Rev4Sim

			|| hal == hsHALIDHandspringOs5Rev3 // Treo 700p
			|| hal == hsHALIDHandspringOs5Rev3Sim
			
			|| hal == 'P052' // Centro ???
			|| hal == 'P053' // Centro
		))
	{
		return 0;
	}
	
	if ((launchFlags & LAUNCH_FLAGS) == LAUNCH_FLAGS)
	{
		// Pilot 1.0 will continuously relaunch this app unless we switch to 
		// another safe one.
		if (rom < sysMakeROMVersion(2, 0, 0, sysROMStageRelease, 0))
			AppLaunchWithCommand (sysFileCDefaultApp, 
					sysAppLaunchCmdNormalLaunch, NULL);
	}
	return sysErrRomIncompatible;

} // RomVersionCompatible

/*
 * PhoneUtils.c
 */
