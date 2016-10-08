/*
 * Expire.c
 */
 
#include <PalmOS.h>
#include <HsExt.h>
#include <TimeMgr.h>

#include "Global.h"

// PhoneUtils.c
extern void 					Alert(Char* MsgType, Char* Msg, Err err);

// Encrypt.c
extern void 					encBuf(UInt8* io_buffer, UInt16 buf_len, UInt8* key, CryptAction_e action);
extern Boolean 					encExpiryPrefs(AppExpiryPrefs_t* appExpiryPrefs, CryptAction_e action);

// Prototypes
Boolean 						isExpired(void);
static void 					writeExpiryPrefs(AppExpiryPrefs_t* expiryPrefs);
static void 					readExpiryPrefs(AppExpiryPrefs_t* expiryPrefs);

/*
 * isExpired
 */
Boolean isExpired(void)
{
	Boolean 				bExpired = true;
	
	UInt32					secNow;
	UInt32					secPrefs;
	UInt32					secToExpiry;
	
	AppExpiryPrefs_t* 	appExpiryPrefs = NULL;
	
	// Get seconds now
	secNow = TimGetSeconds();

	appExpiryPrefs = MemPtrNew(sizeof(AppExpiryPrefs_t));
	
	if (appExpiryPrefs)
	{
		MemSet(appExpiryPrefs, sizeof(AppExpiryPrefs_t), 0);
			
		readExpiryPrefs(appExpiryPrefs);	

		appExpiryPrefs->numRuns += 1; // increment number of runs
		secToExpiry = appExpiryPrefs->expiryDate;
		secPrefs = appExpiryPrefs->timeLastUsed;

		if (secNow > secPrefs)
		{
			appExpiryPrefs->timeLastUsed = secNow;
			secPrefs = secNow; // copy secNow to secPrefs so that we do only one check (of secPrefs) later...
		}

		if ((secPrefs < secToExpiry) && (appExpiryPrefs->numRuns < appExpiryPrefs->maxRuns))
			bExpired = false;

		writeExpiryPrefs(appExpiryPrefs); // careful, appExpiryPrefs are encrypted here...

		MemPtrFree(appExpiryPrefs);
	}
	return bExpired;
}

/*
 * writeExpiryPrefs
 */
static void writeExpiryPrefs(AppExpiryPrefs_t* expiryPrefs)
{
	encExpiryPrefs(expiryPrefs, encrypt);
	PrefSetAppPreferences(appExpiryFileCreator, appPrefID, appPrefVersionNum,
				expiryPrefs, sizeof(AppExpiryPrefs_t), true);
	encExpiryPrefs(expiryPrefs, decrypt);
}

/*
 * readExpiryPrefs
 */
static void readExpiryPrefs(AppExpiryPrefs_t* expiryPrefs)
{	
	UInt16 expiryPrefSize = sizeof(AppExpiryPrefs_t);
	
	if (PrefGetAppPreferences(appExpiryFileCreator, appPrefID, expiryPrefs, &expiryPrefSize, true)
			== noPreferenceFound)
	{	
		expiryPrefs->expiryDate = TimGetSeconds() + (604800); // 7 days
		expiryPrefs->timeLastUsed = 0;
		expiryPrefs->maxRuns = MAX_RUNS;
		expiryPrefs->numRuns = 0;
				
		writeExpiryPrefs(expiryPrefs); // expiryPerfs encrypted here...
	}
	else
		encExpiryPrefs(expiryPrefs, decrypt);	// ....we decrypt it here!
}

/*
 * Expire.c
 */
