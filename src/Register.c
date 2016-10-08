/*
 * Register.c
 */
 
#include <PalmOS.h>
#include <Form.h>
#include <HsExt.h>
// #include <DLServer.h>

#include "Global.h"
#include "AppResources.h"

// Encrypt.c
extern Boolean 				verifyRegistration(Char* RegKey);

// Prototypes
static Boolean 				RegAlertCallback(Int16 ButtonID, Char* str);
static UInt16 				DisplayRegAlert(Char* vendorName, Char* StrHSSN, Char* StrKey);
UInt16 						DisplayRegForm(Char* StrKey);

/*
 * RegAlertCallback
 */
static Boolean RegAlertCallback(Int16 ButtonID, Char* str)
{
	return ((ButtonID == 1) || ((ButtonID == 0) && (StrLen(str) == HEX_KEY_LENGTH)));

} // RegAlertCallback

/*
 * DisplayRegAlert
 */
static UInt16 DisplayRegAlert(Char* vendorName, Char* StrHSSN, Char* StrKey)
{	
	return FrmCustomResponseAlert(REGISTRATION_FORM, vendorName,
			StrHSSN, "", StrKey, HEX_KEY_LENGTH + 1, *RegAlertCallback);

} // DisplayRegAlert

/*
 * DisplayRegForm
 */
UInt16 DisplayRegForm(Char* StrKey)
{
	Char	StrHSSN[HSSN_LENGTH + 1];
	UInt16	HSSNbufLen = sizeof(StrHSSN);
	
	if (!HsGetVersionString(hsVerStrSerialNo, StrHSSN, &HSSNbufLen))
	{
		return DisplayRegAlert(VENDOR_NAME, StrHSSN, StrKey);
	}
	else
	{
		FrmCustomAlert(10024, "Cannnot register!\n", "HSSN Error.", "");
		return 999; // don't care
	}
	
} // DisplayRegForm

/*
 * Register.c
 */

