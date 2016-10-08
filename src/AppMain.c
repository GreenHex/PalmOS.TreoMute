/*
 * AppMain.c
 */
 
#include <Hs.h>
#include <HsPhone.h>
#include <HsNav.h>
#include <HsExt.h>
#include <palmOneResources.h>
#include <PalmTypes.h>
#include <Form.h>
#include <AlarmMgr.h>
#include <TonesLibTypes.h>
#include <TonesLib.h>

#include "Global.h"
#include "AppResources.h"

// PhoneUtils.c
extern void 		 		DelayTask(UInt32 DelaySeconds);
extern void 				Alert(Char* MsgType, Char* Msg, Err err);
extern void 				beep(UInt8 numBeeps);
extern Boolean 				IsPhoneGSM(void);
extern Err 					RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags);

// Panl.c
extern void 				PanelFormClose(FormPtr pForm);

// Expire.c
extern Boolean 				isExpired(void);

#ifdef WITH_REGISTRATION
// Register.c
extern UInt16 				DisplayRegForm(Char* StrKey);

// Encrypt.c
extern void 				encBuf(UInt8* io_buffer, UInt16 buf_len, UInt8* key, CryptAction_e action);
extern Boolean 				encAppPrefs(AppPreferences_t* prefsP, CryptAction_e action);
extern Boolean 				verifyRegistration(Char* RegKey);
#endif /* WITH_REGISTRATION */

// Prototypes
static void 				RegisterForNotifications(Boolean bRegister);
static void 				writePrefs(AppPreferences_t* prefsP);
static void 				readPrefs(AppPreferences_t* prefsP);
static void 				writeUSPrefs(USPrefs_t* usPrefsP);
static void 				readUSPrefs(USPrefs_t* usPrefsP);
static Err 					AppStart(void);
static void 				AppStop(void);
static void 				InitializeMainForm(FormType* pForm, AppPreferences_t* prefsP);
static Boolean 				MainFormHandleEvent(EventType* pEvent);
static Boolean 				AppHandleEvent(EventType* pEvent);
static void 				AppEventLoop(void);
static void 				SaveRect(RectangleType* rectP, WinHandle* winHP, RectangleType* rectObscuredP);
static void 				RestoreRect(WinHandle* winHP, RectangleType* rectObscuredP);
static void 				DisplayState(Char* str1, Char* str2, UInt16 rectWidth, UInt16 rectHeight);
static void 				setPhnSoundPrefs(AppPreferences_t* prefsP, USPrefs_t* usPrefsP, MuteAction_e state);
static void 				SetMuteState(AppPreferences_t* prefsP, USPrefs_t* usPrefsP, MuteAction_e state);
static void 				getStrDelayTime(UInt32 delayTime, Char* str);
static void 				getStrDateTime(UInt32 TimeSecs, Char* dtStr);
static void 				popupFormCallback(UInt16 almProcCmd, SysAlarmTriggeredParamType *paramP);
static Boolean 				popupFormHandleEvent(EventType* pEvent);
static Boolean 				popupForm(UInt16* initIdxP, Boolean* vibrateON);
UInt32 						PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);

/*
 * writePrefs
 */
static void writePrefs(AppPreferences_t* prefsP)
{
#ifdef WITH_REGISTRATION
	encAppPrefs(prefsP, encrypt);
#endif /* WITH_REGISTRATION */

	PrefSetAppPreferences(appFileCreator, appPrefID, appPrefVersionNum, prefsP, sizeof(AppPreferences_t), true);

#ifdef WITH_REGISTRATION
	encAppPrefs(prefsP, decrypt); // decrypt immediately...
#endif /* WITH_REGISTRATION */

} // writePrefs

/*
 * readPrefs
 */
static void readPrefs(AppPreferences_t* prefsP)
{	
	UInt16 prefSize = sizeof(AppPreferences_t);
	
	if (PrefGetAppPreferences(appFileCreator, appPrefID, prefsP, &prefSize, true) == noPreferenceFound)
	{	
		// default application preference values
		prefsP->bEnabled = PREFS_ENABLED;
		prefsP->timeMuteIdx = PREFS_TIME_MUTE_IDX;
		prefsP->optionMaskIdx = PREFS_OPTION_MASK_IDX;
		prefsP->keyPressIdx = PREFS_KEYPRESS_IDX;
		StrNCopy(prefsP->strRegKey, PREFS_REG_KEY_STRING, sizeof(PREFS_REG_KEY_STRING)-1);
		prefsP->strRegKey[sizeof(PREFS_REG_KEY_STRING)-1] = chrNull;
		prefsP->bDisplayTimeSelection = PREFS_DISPLAY_TIME_SELECTION;
		prefsP->bSystemSnd = PREFS_SYSTEM;
		prefsP->bAlarmSnd = PREFS_ALARM;
		prefsP->bGameSnd = PREFS_GAME;
		prefsP->bPhoneSnd = PREFS_PHONE;
		prefsP->bMessagingSnd = PREFS_MESSAGING;
		prefsP->bCalendarSnd = PREFS_CALENDAR;
		prefsP->bBeepConfirmation = PREFS_BEEP_CONFIRMATION;
		prefsP->bVibrationON = PREFS_VIBRATION_ON;
		
		writePrefs(prefsP);
	}
	
#ifdef WITH_REGISTRATION
	else
	{
		encAppPrefs(prefsP, decrypt); // prefsP encrypted, so decrypt it...
	}
#endif /* WITH_REGISTRATION */

} // readPrefs

/*
 * writeUSPrefs
 */
static void writeUSPrefs(USPrefs_t* usPrefsP)
{
	PrefSetAppPreferences(appFileCreator, appPrefID, appPrefVersionNum, usPrefsP, sizeof(USPrefs_t), false);

} // writeUSPrefs

/*
 * readUSPrefs
 */
static void readUSPrefs(USPrefs_t* usPrefsP)
{	
	UInt16 usPrefSize = sizeof(USPrefs_t);
	
	if (PrefGetAppPreferences(appFileCreator, appPrefID, usPrefsP, &usPrefSize, false) == noPreferenceFound)
	{	
		// default application preference values
		usPrefsP->bMuted = US_PREFS_MUTED;
		usPrefsP->sndLevelSystem = US_PREFS_SYS_VOLUME;
		usPrefsP->sndLevelAlarm = US_PREFS_ALARM_VOLUME;
		usPrefsP->sndLevelGame = US_PREFS_GAME_VOLUME;
		usPrefsP->phnAlertVolume = US_PREFS_PHN_ALERT_VOLUME;
		usPrefsP->smsAlertVolume = US_PREFS_SMS_ALERT_VOLUME;
		usPrefsP->calAlertVolume = US_PREFS_CAL_ALERT_VOLUME;
		
		writeUSPrefs(usPrefsP);
	}
	
} // readUSPrefs

/*
 * AppStart
 */
static Err AppStart(void)
{
	FrmGotoForm(MAIN_FORM);
	return errNone;
	
} // AppStart

/*
 * AppStop
 */
static void AppStop(void)
{
	FrmCloseAllForms();	

} // AppStop

/*
 * RegisterForNotifications
 */
static void RegisterForNotifications(Boolean bRegister)
{
	UInt16 				cardNo; 
	LocalID  			dbID;
	
	SysCurAppDatabase(&cardNo, &dbID);
		
	if (bRegister)
	{
		SysNotifyRegister(cardNo, dbID, sysNotifyVirtualCharHandlingEvent, NULL, sysNotifyNormalPriority, NULL);
	}
	else
	{
		SysNotifyUnregister(cardNo, dbID, sysNotifyVirtualCharHandlingEvent, sysNotifyNormalPriority);
	}
				
} // RegisterForNotifications

/*
 * InitializeMainForm
 */
static void InitializeMainForm(FormType* pForm, AppPreferences_t* prefsP)
{	
	ControlType*    pCtl;
	ListType*		pList;
		
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_ENABLE_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bEnabled);
	
	pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_MUTE_LIST));
	LstSetSelection(pList, prefsP->timeMuteIdx);
	LstSetHeight(pList, 10);
	LstSetTopItem(pList, prefsP->timeMuteIdx);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_MUTE_POPUP));
	CtlSetLabel(pCtl, LstGetSelectionText(pList, LstGetSelection(pList)));
	
	pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_OPTION_MASK_LIST));
	LstSetSelection(pList, prefsP->optionMaskIdx);
	LstSetHeight(pList, 4);
	LstSetTopItem(pList, prefsP->optionMaskIdx);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_OPTION_MASK_POPUP));
	CtlSetLabel(pCtl, LstGetSelectionText(pList, LstGetSelection(pList)));
	
	pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_KEY_PRESS_LIST));
	LstSetSelection(pList, prefsP->keyPressIdx);
	LstSetHeight(pList, 10);
	LstSetTopItem(pList, prefsP->keyPressIdx);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_KEY_PRESS_POPUP));
	CtlSetLabel(pCtl, LstGetSelectionText(pList, LstGetSelection(pList)));
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_DISPLAY_TIME_SELECTION_POPUP_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bDisplayTimeSelection);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_SYSTEM_SOUND_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bSystemSnd);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_ALARM_SOUND_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bAlarmSnd);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_GAME_SOUND_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bGameSnd);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_PHONE_SOUND_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bPhoneSnd);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_MESSAGING_SOUND_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bMessagingSnd);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_CALENDAR_SOUND_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bCalendarSnd);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_BEEP_CONFIRMATION_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bBeepConfirmation);

	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_VIBRATION_ON_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bVibrationON);

#ifdef WITH_REGISTRATION
	if (verifyRegistration(prefsP->strRegKey))
		FrmHideObject(pForm, FrmGetObjectIndex(pForm, REGISTER_BUTTON));
#endif /* WITH_REGISTRATION */

	FrmDrawForm(pForm);
	
} // InitializeMainForm

/*
 * MainFormHandleEvent
 */
static Boolean MainFormHandleEvent(EventType* pEvent)
{
	Boolean 				handled = false;
	FormType* 				pForm = NULL;
	ControlType*    		pCtl = NULL;
	ListType*				pList = NULL;

	AppPreferences_t*		prefsP = NULL;
	
	EventType 				newEvent;
	
	prefsP = MemPtrNew(sizeof(AppPreferences_t));
	if (!prefsP)
	{
		Alert(ALERT_MEMORY_ERROR, "(AppPreferences_t)", 0);
		return handled;
	}
	MemSet(prefsP, sizeof(AppPreferences_t), 0);

	pForm = FrmGetActiveForm(); // THE CAUSE OF SO MANY CRASHES!!!
	
	switch (pEvent->eType)
	{
		case frmOpenEvent:
		
			readPrefs(prefsP); // have to do this first so that prefsP are read before initialization...
			InitializeMainForm(pForm, prefsP);			
			FrmDrawForm(pForm);
			
			handled = true;
			break;
			
		case frmCloseEvent:
		
			PanelFormClose(pForm);
			
			handled = true;
			break;
			
		case ctlSelectEvent:
			switch (pEvent->data.ctlSelect.controlID)
			{	
#ifdef WITH_REGISTRATION
				
				case REGISTER_BUTTON:
				
					readPrefs(prefsP);	

					if (DisplayRegForm(prefsP->strRegKey) == 0) // empty strRegKey passed in...			
					{
						if (verifyRegistration(prefsP->strRegKey))
						{
							writePrefs(prefsP);
							
							FrmHideObject(pForm, FrmGetObjectIndex(pForm, REGISTER_BUTTON));
							FrmCustomAlert(ALERT_BOX, "Thank you for registering!", "", "");
						}
						else
						{
							FrmCustomAlert(ALERT_BOX, "Invalid key!", "", "");	
							break;
						}
					}	
					break;
#endif /* WITH_REGISTRATION */
					
				case MAIN_DONE_BUTTON:
					{									
						readPrefs(prefsP); // What a waste, but not so...
						
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_ENABLE_CHECKBOX));
						prefsP->bEnabled = (CtlGetValue(pCtl) == 1);
						
						pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_MUTE_LIST));
						prefsP->timeMuteIdx = LstGetSelection(pList);
						
						pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_OPTION_MASK_LIST));
						prefsP->optionMaskIdx = LstGetSelection(pList);
						
						pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_KEY_PRESS_LIST));
						prefsP->keyPressIdx = LstGetSelection(pList);
						
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_DISPLAY_TIME_SELECTION_POPUP_CHECKBOX));
						prefsP->bDisplayTimeSelection = (CtlGetValue(pCtl) == 1);
	
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_SYSTEM_SOUND_CHECKBOX));
						prefsP->bSystemSnd = (CtlGetValue(pCtl) == 1);
						
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_ALARM_SOUND_CHECKBOX));
						prefsP->bAlarmSnd = (CtlGetValue(pCtl) == 1);
						
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_GAME_SOUND_CHECKBOX));
						prefsP->bGameSnd = (CtlGetValue(pCtl) == 1);
						
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_PHONE_SOUND_CHECKBOX));
						prefsP->bPhoneSnd = (CtlGetValue(pCtl) == 1);
						
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_MESSAGING_SOUND_CHECKBOX));
						prefsP->bMessagingSnd = (CtlGetValue(pCtl) == 1);
						
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_CALENDAR_SOUND_CHECKBOX));
						prefsP->bCalendarSnd = (CtlGetValue(pCtl) == 1);
						
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_BEEP_CONFIRMATION_CHECKBOX));
						prefsP->bBeepConfirmation = (CtlGetValue(pCtl) == 1);
						
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_VIBRATION_ON_CHECKBOX));
						prefsP->bVibrationON = (CtlGetValue(pCtl) == 1);
						
						writePrefs(prefsP);
						
						RegisterForNotifications(prefsP->bEnabled);
						
						newEvent.eType = appStopEvent;
						EvtAddEventToQueue(&newEvent);
						
						handled = true;
					}
					break;
	
				default:
					break;
			}
			break;
			
		default:
			break;
	}
	
	if (prefsP)
		MemPtrFree(prefsP);
	
	return handled;
	
} // MainFormHandleEvent

/*
 * AppHandleEvent
 */
static Boolean AppHandleEvent(EventType* pEvent)
{
	UInt16 		formID;
	FormType* 	pForm;
	Boolean		handled = false;

	if (pEvent->eType == frmLoadEvent)
	{
		// Load the form resource.
		formID = pEvent->data.frmLoad.formID;
		
		pForm = FrmInitForm(formID);
		FrmSetActiveForm(pForm);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		if (formID == MAIN_FORM)
			FrmSetEventHandler(pForm, MainFormHandleEvent);
			
		handled = true;
	}
	
	return handled;
	
} // AppHandleEvent

/*
 * AppEventLoop
 */
static void AppEventLoop(void)
{
	// Err			error;
	EventType	event;

	do {
		EvtGetEvent(&event, evtWaitForever);

		if (SysHandleEvent(&event))
			continue;
			
		// if (MenuHandleEvent(0, &event, &error))
		//	continue;
			
		if (AppHandleEvent(&event))
			continue; 

		FrmDispatchEvent(&event);

	} while (event.eType != appStopEvent);

} // AppEventLoop

/*
 * SaveRect
 */
static void SaveRect(RectangleType* rectP, WinHandle* winHP, RectangleType* rectObscuredP)
{
	Err						error = errNone;
	
	if ((rectP) && (winHP)) 
	{
		*winHP = NULL; // preset
		
		WinGetFramesRectangle(dialogFrame, rectP, rectObscuredP);			
		*winHP = WinSaveBits(rectObscuredP, &error);
		
		if (error)
			*winHP = NULL;		
	}

} // SaveRect

/*
 * RestoreRect
 */
static void RestoreRect(WinHandle* winHP, RectangleType* rectObscuredP)
{
	if ((*winHP) && (rectObscuredP))
	{			
		WinRestoreBits(*winHP, rectObscuredP->topLeft.x, rectObscuredP->topLeft.y);
		*winHP = NULL;
	}
	
} // RestoreRect

/*
 * DisplayState
 */
static void DisplayState(Char* str1, Char* str2, UInt16 rectWidth, UInt16 rectHeight)
{
	RectangleType			rectObscured;
	WinHandle				winH = NULL;
	FormActiveStateType		activeState;
	// WinHandle				oldWin;
	// WinHandle				newWin;
		
	UInt16					rectLeft_X = (DISP_WIDTH - rectWidth) / 2;
	UInt16					rectTop_Y = ((DISP_HEIGHT - rectHeight) / 2); //  - 4;
	UInt16					lineWidth = rectWidth - (2 * LINE_GAP);
	
    RectangleType 			rect = {{rectLeft_X, rectTop_Y}, {rectWidth, rectHeight}};
    RGBColorType 			rgb;
    IndexedColorType		colorWhite;
    IndexedColorType		colorBlack;
    IndexedColorType		colorGrey;
	IndexedColorType 		colorRed;
	IndexedColorType		colorGreen;
	
	UInt16					str1Width = 0;
	UInt16					str2Width = 0;
	UInt16					strHeight = 0;
	
	Boolean					onFlag = (StrCompare(str1, APP_NAME) != 0);
	Boolean					cancelledFlag = (StrCompare(str2, STR_CANCELLED) == 0);

	WinDisplayToWindowPt(&rectLeft_X, &rectTop_Y);

	rect.topLeft.x = rectLeft_X;
	rect.topLeft.y = rectTop_Y;
	
    FrmSaveActiveState(&activeState);

	// oldWin = WinGetActiveWindow();
	// newWin = WinGetDisplayWindow();
	
	// WinSetActiveWindow(newWin);
	// WinSetDrawWindow(newWin); 
	
	WinPushDrawState();
		
	SaveRect(&rect, &winH, &rectObscured);
		
	rgb.r=255; rgb.g=255; rgb.b=255; // white
	colorWhite = WinRGBToIndex(&rgb);	
	
	rgb.r=0; rgb.g=0; rgb.b=0; // black
	colorBlack = WinRGBToIndex(&rgb);
	
    rgb.r=150; rgb.g=150; rgb.b=150; // grey?
	colorGrey = WinRGBToIndex(&rgb);
	
	rgb.r=255; rgb.g=0; rgb.b=51; // red?
	colorRed = WinRGBToIndex(&rgb);
		
	rgb.r=0; rgb.g=153; rgb.b=51; // green?
	colorGreen = WinRGBToIndex(&rgb);	
		
	WinSetForeColor(colorGrey);
	WinSetBackColor(colorWhite);
		
	WinEraseRectangleFrame(dialogFrame, &rect); 
    WinEraseRectangle(&rect, 2); // 2nd arg is corner "diameter"!

	WinEraseRectangleFrame(dialogFrame, &rect);
	WinPaintRectangleFrame(dialogFrame, &rect);
	
	FntSetFont(stdFont);
	    
	str1Width = FntLineWidth(str1, StrLen(str1));
	
	if (onFlag)
	{	
		WinSetTextColor(colorRed);
	}
	else
	{
		WinSetTextColor(colorBlack);
	}
	
    WinPaintChars(str1, StrLen(str1),
				rectLeft_X + (rectWidth - str1Width)/2,
				rectTop_Y + TXT_OFF_Y);
			
	if (str2)
	{
		if (onFlag)
		{
			WinSetTextColor(colorBlack);
		}
		else if (cancelledFlag)
		{
			WinSetTextColor(colorRed);
			FntSetFont(boldFont);
		}
		else
		{
			WinSetTextColor(colorGreen);
		}
		
		WinPaintLine(rectLeft_X + LINE_GAP, rectTop_Y + (rectHeight / 2), rectLeft_X + lineWidth, rectTop_Y + (rectHeight / 2));
		
		strHeight = FntLineHeight();
		str2Width = FntLineWidth(str2, StrLen(str2));
		
	    WinPaintChars(str2, StrLen(str2),
				rectLeft_X + (rectWidth - str2Width)/2,
				rectTop_Y + TXT_OFF_Y + strHeight + 4);	
	}
    
	DelayTask(2);
	RestoreRect(&winH, &rectObscured);

	WinPopDrawState();

	// WinSetActiveWindow(oldWin);
	// WinSetDrawWindow(oldWin);
	
	FrmRestoreActiveState(&activeState);
   
} // DisplayState

/*
 * popupFormHandleEvent
 */
static Boolean popupFormHandleEvent(EventType* pEvent)
{
	Boolean 				handled = false;
	FormType* 				pForm = FrmGetActiveForm();
	
	switch (pEvent->eType)
	{
		case keyDownEvent:
			{
				ListType*	pList;
				UInt16		idx;
				
				AlmSetProcAlarm(*popupFormCallback, 0, TimGetSeconds() + POPUP_FORM_WAIT_TIME);
				
				pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, POP_MUTE_TIME_LIST));
				idx = LstGetSelection(pList);
				
				if (pEvent->data.keyDown.keyCode == vchrRockerDown)
				{
					if (idx < (LstGetNumberOfItems(pList) - 1))
						++idx;		
					
					handled = true;
				}
				else if (pEvent->data.keyDown.keyCode == vchrRockerUp)
				{
					if(idx > 0)
						--idx;
					
					handled = true;
				}	
				else
				{
					EventType 				newEvent;

					CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, POP_EXIT_STATE_CHECKBOX)),
							((pEvent->data.keyDown.keyCode == vchrRockerCenter)
							|| (pEvent->data.keyDown.keyCode == chrSpace)
							|| (pEvent->data.keyDown.keyCode == chrLineFeed)
							|| (pEvent->data.keyDown.keyCode == chrCarriageReturn)));
					 
					newEvent.eType = frmCloseEvent;
					newEvent.data.frmClose.formID = POP_FORM;
					EvtAddEventToQueue(&newEvent);
					
					handled = true;
				}
					
				LstSetSelection(pList, idx);
			}
			
			break;
					
		case frmOpenEvent:
		
			break;
			
		case frmCloseEvent:
			
			break;
			
		case ctlSelectEvent:
		
			if (pEvent->data.ctlSelect.controlID == POP_VIBRATE_ON_CHECKBOX)
			{
				if (CtlGetValue(pEvent->data.ctlSelect.pControl))
				{
					CtlSetLabel(pEvent->data.ctlSelect.pControl, "Vibrate ON");
				}
				else
				{
					CtlSetLabel(pEvent->data.ctlSelect.pControl, "Vibrate OFF");
				}
				
				AlmSetProcAlarm(*popupFormCallback, 0, TimGetSeconds() + POPUP_FORM_WAIT_TIME);	
			}
			break;
			
		case lstSelectEvent:
			{		
				EventType 				newEvent;
				
				newEvent.eType = frmCloseEvent;
				newEvent.data.frmClose.formID = POP_FORM;
				EvtAddEventToQueue(&newEvent);
			}
			handled = true;
			break;
			
		case frmObjectFocusTakeEvent:
		case frmObjectFocusLostEvent:	
			// handled = (pEvent->data.frmObjectFocusTake.objectID == POP_MUTE_TIME_LIST);
			handled = true;	
			break;
			
		default:
		
			break;
	}			
	return handled;
	
} // popupFormHandleEvent

/*
 * popupFormCallback
 */
static void popupFormCallback(UInt16 almProcCmd, SysAlarmTriggeredParamType *paramP)
{
	if (almProcCmd == almProcCmdTriggered)
	{				
		EventType 				newEvent;
		
		newEvent.eType = frmCloseEvent;
		EvtAddEventToQueue(&newEvent);
	}
		
	EvtWakeup();		
	
	return;
	
} // popupFormCallback

/*
 * popupForm
 */
static Boolean popupForm(UInt16* initIdxP, Boolean* vibrateON)
{
	Boolean				retVal = true;
	
	FormType*			pOldForm = NULL;
	FormType*			pForm = NULL;
	ListType*			pList = NULL;
	ControlType*		pCtl = NULL;
	
	pOldForm = FrmGetActiveForm();
	
	pForm = FrmInitForm(POP_FORM);
	FrmSetEventHandler(pForm, *popupFormHandleEvent);
	
	pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, POP_MUTE_TIME_LIST));
	LstSetSelection(pList, *initIdxP);
	LstSetTopItem(pList, *initIdxP);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, POP_VIBRATE_ON_CHECKBOX));
	CtlSetValue(pCtl, *vibrateON);

	if (*vibrateON)
	{
		CtlSetLabel(pCtl, "Vibrate ON");
	}
	else
	{
		CtlSetLabel(pCtl, "Vibrate OFF");
	}
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, 	POP_EXIT_STATE_CHECKBOX));
	CtlSetValue(pCtl, true);
	 						
	AlmSetProcAlarm(*popupFormCallback, 0, TimGetSeconds() + POPUP_FORM_WAIT_TIME);

	FrmDoDialog(pForm);
	
	AlmSetProcAlarm(*popupFormCallback, 0, 0);
	
	pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, POP_MUTE_TIME_LIST));
	*initIdxP = LstGetSelection(pList);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, POP_VIBRATE_ON_CHECKBOX));
	*vibrateON = (CtlGetValue(pCtl) == 1);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, 	POP_EXIT_STATE_CHECKBOX));
	retVal = (CtlGetValue(pCtl) == 1);
	
	FrmEraseForm(pForm);
	FrmDeleteForm(pForm);
	
	FrmSetActiveForm(pOldForm);
	
	return (retVal);
	
} // popupForm

/*
 * setPhnSoundPrefs
 */
static void setPhnSoundPrefs(AppPreferences_t* prefsP, USPrefs_t* usPrefsP, MuteAction_e state)
{
	Err					error = errNone;
	
	UInt16				tonesLibRefNum = 0;
	SoundPreference		sndPref;
	
	error = SysLibFind(tonesLibName, &tonesLibRefNum);
		if (error) SysLibLoad(tonesLibType, tonesLibCreator, &tonesLibRefNum);

	error = TonesLibOpen(tonesLibRefNum);
					
	if (!error)
	{
		if (prefsP->bPhoneSnd)
		{
			TonesLibGetSoundPrefs(tonesLibRefNum, soundPrefTypePhone, &sndPref);
	
			if (state == phnMute)
			{
				if (sndPref.soundOnVolume != toneVolumeOff)
					usPrefsP->phnAlertVolume = sndPref.soundOnVolume; // save previous value...
			
				sndPref.soundOnVolume = toneVolumeOff;
				sndPref.soundOnVibrate = prefsP->bVibrationON;
				sndPref.soundOffVibrate = prefsP->bVibrationON;
			}
			else if (state == phnNormal)
			{	
				if ((sndPref.soundOnVolume == toneVolumeOff) && (usPrefsP->phnAlertVolume != toneVolumeOff))
					sndPref.soundOnVolume = usPrefsP->phnAlertVolume;
					
				sndPref.soundOnVibrate = false;
				sndPref.soundOffVibrate = true;	
			}
			
			TonesLibSetSoundPrefs(tonesLibRefNum, soundPrefTypePhone, &sndPref);
		}
		
		if (prefsP->bMessagingSnd)
		{
			TonesLibGetSoundPrefs(tonesLibRefNum, soundPrefTypeSMS, &sndPref);
	
			if (state == phnMute)
			{
				if (sndPref.soundOnVolume != toneVolumeOff)
					usPrefsP->smsAlertVolume = sndPref.soundOnVolume; // save previous value...
			
				sndPref.soundOnVolume = toneVolumeOff;
				sndPref.soundOnVibrate = prefsP->bVibrationON;
				sndPref.soundOffVibrate = prefsP->bVibrationON;
				
			}
			else if (state == phnNormal)
			{	
				if ((sndPref.soundOnVolume == toneVolumeOff) && (usPrefsP->smsAlertVolume != toneVolumeOff))
					sndPref.soundOnVolume = usPrefsP->smsAlertVolume;
					
				sndPref.soundOnVibrate = false;
				sndPref.soundOffVibrate = true;	
			}
			
			TonesLibSetSoundPrefs(tonesLibRefNum, soundPrefTypeSMS, &sndPref);
		}
		
		if (prefsP->bCalendarSnd)
		{
			TonesLibGetSoundPrefs(tonesLibRefNum, soundPrefTypeCalendar, &sndPref);
	
			if (state == phnMute)
			{
				if (sndPref.soundOnVolume != toneVolumeOff)
					usPrefsP->calAlertVolume = sndPref.soundOnVolume; // save previous value...
			
				sndPref.soundOnVolume = toneVolumeOff;
				sndPref.soundOnVibrate = prefsP->bVibrationON;
				sndPref.soundOffVibrate = prefsP->bVibrationON;
				
			}
			else if (state == phnNormal)
			{	
				if ((sndPref.soundOnVolume == toneVolumeOff) && (usPrefsP->calAlertVolume != toneVolumeOff))
					sndPref.soundOnVolume = usPrefsP->calAlertVolume;
					
				sndPref.soundOnVibrate = false;
				sndPref.soundOffVibrate = true;	
			}
			
			TonesLibSetSoundPrefs(tonesLibRefNum, soundPrefTypeCalendar, &sndPref);
		}
		
		TonesLibClose(tonesLibRefNum);	
	}
} // setPhnSoundPrefs

/*
 * SetMuteState
 */
static void SetMuteState(AppPreferences_t* prefsP, USPrefs_t* usPrefsP, MuteAction_e state)
{
	
	setPhnSoundPrefs(prefsP, usPrefsP, state);
	
	if ((state == phnMute) && (!usPrefsP->bMuted))
	{	
		if (prefsP->bSystemSnd)
		{
			if (PrefGetPreference(prefSysSoundVolume) != 0)
				usPrefsP->sndLevelSystem = PrefGetPreference(prefSysSoundVolume);
			
			PrefSetPreference(prefSysSoundVolume, 0);
		}
		
		if (prefsP->bAlarmSnd)
		{	
			if (PrefGetPreference(prefAlarmSoundVolume) != 0)
				usPrefsP->sndLevelAlarm = PrefGetPreference(prefAlarmSoundVolume); 
			
			PrefSetPreference(prefAlarmSoundVolume, 0);
		}
		
		if (prefsP->bGameSnd)
		{	
			if (PrefGetPreference(prefGameSoundVolume) != 0)
				usPrefsP->sndLevelGame = PrefGetPreference(prefGameSoundVolume);
			
			PrefSetPreference(prefGameSoundVolume, 0);
		}
		
		usPrefsP->bMuted = true;
	}
	else if ((state == phnNormal) && (usPrefsP->bMuted))
	{
		if ((prefsP->bSystemSnd) && (usPrefsP->sndLevelSystem != 0))
			PrefSetPreference(prefSysSoundVolume, usPrefsP->sndLevelSystem);
		
		if ((prefsP->bAlarmSnd) && (usPrefsP->sndLevelAlarm != 0))
			PrefSetPreference(prefAlarmSoundVolume, usPrefsP->sndLevelAlarm); 
		
		if ((prefsP->bGameSnd) && (usPrefsP->sndLevelGame != 0))
			PrefSetPreference(prefGameSoundVolume, usPrefsP->sndLevelGame);

		usPrefsP->bMuted = false;		
	}
	
	writeUSPrefs(usPrefsP);
	
} // SetMuteState

/*
 * getStrDelayTime
 */
static void getStrDelayTime(UInt32 delayTime, Char* str)
{
	DateTimeType		dt;
	
	TimSecondsToDateTime(delayTime, &dt);
	StrPrintF(str, "Sounds Off  [%02uh %02um]", dt.hour, dt.minute);	

} // getStrDelayTime

/*
 * getStrDateTime
 */
static void getStrDateTime(UInt32 TimeSecs, Char* dtStr)
{
	DateTimeType 		dtNow;
	Char				dateStr[dateStringLength];
	Char				timeStr[timeStringLength];
	
	TimSecondsToDateTime(TimeSecs, &dtNow);
	
	DateToAscii(dtNow.month, dtNow.day, dtNow.year, PrefGetPreference(prefDateFormat), dateStr);
	TimeToAscii(dtNow.hour, dtNow.minute, PrefGetPreference(prefTimeFormat), timeStr);
	
	MemMove(dtStr, STR_ON_AT, StrLen(STR_ON_AT));
	MemMove(dtStr + StrLen(STR_ON_AT), timeStr, StrLen(timeStr));
	dtStr[StrLen(STR_ON_AT) + StrLen(timeStr)] = ' ';
	dtStr[StrLen(STR_ON_AT) + StrLen(timeStr) + 1] = ' ';
	MemMove(dtStr + StrLen(STR_ON_AT) +  StrLen(timeStr) + 2, dateStr, StrLen(dateStr));
	dtStr[StrLen(STR_ON_AT) + StrLen(dateStr) + StrLen(timeStr) + 2] = chrNull;
		
} // getStrDateTime

/*
 * PilotMain
 */
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err 						error = errNone;

	UInt32		muteTime[11] = { 300, 600, 900, 1800, 2700, 3600, 5400, 7200, 10800, 14400, 21600 }; 
	UInt16		keyPressOptionMask[4] = { 0, shiftKeyMask, optionKeyMask, doubleTapKeyMask };
	UInt16 		keyPress[10] = { 0, vchrRockerUp, vchrRockerDown, vchrRockerLeft, vchrRockerRight, vchrRockerCenter,
								vchrLaunch, vchrMenu, vchrHard2, vchrHard3 };

	AppPreferences_t* 			prefsP = NULL;
	USPrefs_t*					usPrefsP = NULL;
	
	prefsP = MemPtrNew(sizeof(AppPreferences_t));
	if (!prefsP)
	{
		goto EXIT_MAIN;
	}
	
	usPrefsP = MemPtrNew(sizeof(USPrefs_t));
	if (!usPrefsP)
	{
		goto EXIT_MAIN;
	}
	
	MemSet(prefsP, sizeof(AppPreferences_t), 0);
  	readPrefs(prefsP);
  	
  	MemSet(usPrefsP, sizeof(USPrefs_t), 0);
	readUSPrefs(usPrefsP);

	switch (cmd)
	{
		case sysAppLaunchCmdNormalLaunch:
	    case sysAppLaunchCmdPanelCalledFromApp:
	    case sysAppLaunchCmdReturnFromPanel:

			// Check device
			if ((error = RomVersionCompatible(MIN_VERSION, launchFlags)))
			{
				Alert(ALERT_PHONE_ERROR, "Incompatible Device", error);
				
				goto EXIT_MAIN;
			} 
#ifdef WITH_REGISTRATION			
			// exit if expired.
			if (isExpired())
			{
				if (!verifyRegistration(prefsP->strRegKey))
				{
					if (DisplayRegForm(prefsP->strRegKey) == 0)					
					{
						if (verifyRegistration(prefsP->strRegKey))
						{
							writePrefs(prefsP);
							
							FrmCustomAlert(ALERT_BOX, "Thank you for registering!", "", "");
						}
						else
						{
							prefsP->bEnabled = false; // disable TreoFlex...
							writePrefs(prefsP);
							
							FrmCustomAlert(ALERT_BOX, "Invalid key, exiting!", "", "");	
							
							break;
						}
					}
					else
					{
						prefsP->bEnabled = false; // disable TreoFlex...
						writePrefs(prefsP);

						break;
					}
				}
			}
#endif /* WITH_REGISTRATION */
	
			if ((error = AppStart()) == 0)
			{
				AppEventLoop();
				AppStop();
			}
			break;
			
		// Register for notifications on reset
		case sysAppLaunchCmdSystemReset:
			// Check device
			if (!(error = RomVersionCompatible(MIN_VERSION, launchFlags)))
			{
				RegisterForNotifications(prefsP->bEnabled);
				
				if (prefsP->bEnabled)
					SetMuteState(prefsP, usPrefsP, phnNormal);
			}		
			break;
			
		case sysAppLaunchCmdNotify:
				
			if (((SysNotifyParamType*)cmdPBP)->notifyType == sysNotifyVirtualCharHandlingEvent)
			{
				SysNotifyParamType* 		notifyParam = (SysNotifyParamType *)cmdPBP;
					
				if (prefsP->keyPressIdx)
				{
					UInt16 				cardNo; 
					LocalID  			dbID;
					
						UInt16				keyMask = ((SysNotifyVirtualCharHandlingType *)notifyParam->notifyDetailsP)->keyDown.modifiers;
						UInt16				keyCode = ((SysNotifyVirtualCharHandlingType *)notifyParam->notifyDetailsP)->keyDown.keyCode;
						
					if ((keyMask & keyPressOptionMask[prefsP->optionMaskIdx]) //  That's "&" NOT "&&"
							|| (prefsP->optionMaskIdx == 0))
					{
						if (keyCode == keyPress[prefsP->keyPressIdx])
						{
							if (!(usPrefsP->bMuted))
							{
								UInt32			onTime = 0;
								Boolean			bDoMute = true;
								
								if (prefsP->bDisplayTimeSelection)
								{
									if (prefsP->bBeepConfirmation)
										beep(1);
									
									bDoMute = popupForm(&(prefsP->timeMuteIdx), &(prefsP->bVibrationON));
								}
								
								if (prefsP->bBeepConfirmation)
										beep(1);
									
								if (bDoMute)
								{
									SetMuteState(prefsP, usPrefsP, phnMute);
									
									onTime = TimGetSeconds() + muteTime[prefsP->timeMuteIdx];
									
									{
										Char		timeStr1[36];
										Char		timeStr2[StrLen(STR_ON_AT) + dateStringLength + 1 + timeStringLength];
										
										getStrDelayTime(muteTime[prefsP->timeMuteIdx], timeStr1);
										getStrDateTime(onTime, timeStr2);
										DisplayState(timeStr1, timeStr2, RECT_WIDTH_BIG, RECT_HEIGHT_BIG);
									}
								
									if (!SysCurAppDatabase(&cardNo, &dbID))
									{						
					 					AlmSetAlarm(cardNo, dbID, 0, onTime, true);
					 				}
								}
								else
								{
									DisplayState(APP_NAME, STR_CANCELLED, RECT_WIDTH_BIG, RECT_HEIGHT_BIG);
								}
							}
							else if (usPrefsP->bMuted)
							{
								if (!SysCurAppDatabase(&cardNo, &dbID))						
				 					AlmSetAlarm(cardNo, dbID, 0, 0, true);
				 					
								SetMuteState(prefsP, usPrefsP, phnNormal);
								
								if (prefsP->bBeepConfirmation)
									beep(2);
									
								DisplayState(APP_NAME, "Sounds On", RECT_WIDTH_BIG, RECT_HEIGHT_BIG);
			 				}
			 			
			 				notifyParam->handled = true;	
						}
					}
				}
			}
			break;
		
		case sysAppLaunchCmdAlarmTriggered:
	
			((SysAlarmTriggeredParamType*)cmdPBP)->purgeAlarm = false;
			
			break;
			
		case sysAppLaunchCmdDisplayAlarm:
		
			if (!((SysDisplayAlarmParamType*)cmdPBP)->ref)
			{
				((SysDisplayAlarmParamType*)cmdPBP)->ref += 1;
			
				if (usPrefsP->bMuted)
				{
					SetMuteState(prefsP, usPrefsP, phnNormal);

					if (prefsP->bBeepConfirmation)
						beep(2);
						
					DisplayState(APP_NAME, "Sounds On", RECT_WIDTH_BIG, RECT_HEIGHT_BIG);		
				}	
			}
			break;
			
		default:
			break;
	}

EXIT_MAIN:

	if (usPrefsP)
		MemPtrFree(usPrefsP);
		
	if (prefsP)
		MemPtrFree(prefsP);

	return error;
	
} // PilotMain

/*
 * AppMain.c
 */

