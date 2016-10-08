/*
 * Encrypt.c
 */
 
#include <PalmOS.h>
#include <HsExt.h>
#include <Encrypt.h>

#include "Global.h"

// PhoneUtils.c
extern void 					getROMID(UInt8* keyVal); // returns 5th to 12th digits of HS SN

// Prototypes
void 							encBuf(UInt8* io_buffer, UInt16 buf_len, UInt8* key, CryptAction_e action);
Boolean 						encExpiryPrefs(AppExpiryPrefs_t* appExpiryPrefs, CryptAction_e action);
Boolean 						encAppPrefs(AppPreferences_t* pPrefs, CryptAction_e action);
Boolean 						verifyRegistration(Char* RegKey);

static void 					hex2bin(int len, char* hexnum, char* binnum);
// static void 					bin2hex(int len, unsigned char* binnum, char* hexnum);
static Boolean 					hasDES(void);

static const char hex[16] =
{
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

#ifdef _LIBC
# define hexval(c) \
  (c >= '0' && c <= '9'							      \
   ? c - '0'								      \
   : ({	int upp = toupper (c);						      \
	upp >= 'A' && upp <= 'Z' ? upp - 'A' + 10 : -1; }))
#else
static char hexval(char c);
#endif

/*
 * Hex to binary conversion
 */
static void hex2bin(int len, char* hexnum, char* binnum)
{
  int i;

  for (i = 0; i < len; i++)
    *binnum++ = 16 * hexval(hexnum[2 * i]) + hexval(hexnum[2 * i + 1]);
}

/*
 * Binary to hex conversion
 */
 /* NOT USED!!!
static void bin2hex(int len, unsigned char* binnum, char* hexnum)
{
  int i;
  unsigned val;

  for (i = 0; i < len; i++)
    {
      val = binnum[i];
      hexnum[i * 2] = hex[val >> 4];
      hexnum[i * 2 + 1] = hex[val & 0xf];
    }
  hexnum[len * 2] = 0;
}
*/

/*
 * hexval
 */
static char hexval(char c)
{
  if (c >= '0' && c <= '9')
    return (c - '0');
  else if (c >= 'a' && c <= 'z')
    return (c - 'a' + 10);
  else if (c >= 'A' && c <= 'Z')
    return (c - 'A' + 10);
  else
    return -1;
}

/*
 * hasDES
 */
static Boolean hasDES(void)
{
	UInt32 		attributes;
	
	return (FtrGet(sysFtrCreator, sysFtrNumEncryption, &attributes) == errNone)
    			&& ((attributes & sysFtrNumEncryptionMaskDES) != 0);
}

/*
 * encBuf
 */
void encBuf(UInt8* io_buffer, UInt16 buf_len, UInt8* key, CryptAction_e action)
{
	UInt8*	temp_buffer;
	UInt16	i;
	
	if (!hasDES())
		return;
	
	temp_buffer = MemPtrNew(buf_len);
	if (temp_buffer)
	{
		MemSet(temp_buffer, buf_len, 0);
		
		for (i = 0; i < buf_len; i += 8)
		{
			EncDES(&(io_buffer[i]), key, &(temp_buffer[i]), (action == encrypt));
		}
		  	
		MemMove(io_buffer, temp_buffer, buf_len);
	}
}

/*
 * encExpiryPrefs
 */
Boolean encExpiryPrefs(AppExpiryPrefs_t* appExpiryPrefs, CryptAction_e action)
{
	Boolean					retVal = false; 
	UInt8*					iPrefs = NULL; // to store appExpirtyPrefs for crypt operations
	UInt8					key[8];

	iPrefs = MemPtrNew(sizeof(AppExpiryPrefs_t));
	if ((appExpiryPrefs) && (iPrefs))
	{
		MemSet(iPrefs, sizeof(AppExpiryPrefs_t), 0);

		getROMID(key);

		MemMove(iPrefs, appExpiryPrefs, sizeof(AppExpiryPrefs_t));
		encBuf(iPrefs, sizeof(AppExpiryPrefs_t), key, action);
		MemMove(appExpiryPrefs, iPrefs, sizeof(AppExpiryPrefs_t)); // move back decrypted
		
		MemPtrFree(iPrefs);	
		retVal = true;
	}
	return retVal;
}

/*
 * encAppPrefs
 */
Boolean encAppPrefs(AppPreferences_t* pPrefs, CryptAction_e action)
{
	Boolean					retVal = false; 
	UInt8*					iPrefs = NULL; // to store appExpirtyPrefs for crypt operations
	UInt8					key[8];

	iPrefs = MemPtrNew(sizeof(AppPreferences_t));
	if ((pPrefs) && (iPrefs))
	{
		MemSet(iPrefs, sizeof(AppPreferences_t), 0);

		getROMID(key);

		MemMove(iPrefs, pPrefs, sizeof(AppPreferences_t));
		encBuf(iPrefs, sizeof(AppPreferences_t), key, action);
		MemMove(pPrefs, iPrefs, sizeof(AppPreferences_t)); // move back decrypted
		
		MemPtrFree(iPrefs);	
		retVal = true;
	}
	return retVal;
}

/*
 * Verify the registration key, RegKey is length (HEX_KEY_LENGTH + 1)
 */
Boolean verifyRegistration(Char* RegKey) // 
{
	Char	StrHSSN[HSSN_LENGTH + 1];
	UInt16	HSSNbufLen = sizeof(StrHSSN);
	
	UInt8	source[KEY_LENGTH + 1];
	UInt8	encKey[KEY_LENGTH + 1];
	UInt8	dest[KEY_LENGTH + 1];
	
	UInt32 	CrID = appFileCreator;
	Char	sCID[KEY_LENGTH + 1];

	StrIToH(sCID, CrID); // convert creator ID

	if ((RegKey) && (!HsGetVersionString(hsVerStrSerialNo, StrHSSN, &HSSNbufLen)))
	{
		// the Source
		MemMove(source, StrHSSN, KEY_LENGTH);
		MemMove(source, sCID + 4, 4); // overwrite first four characters with CrID
		source[KEY_LENGTH] = chrNull;
		
		// the Key
		MemMove(encKey, StrHSSN + 4, KEY_LENGTH);
		encKey[KEY_LENGTH] = chrNull;
		
		// decode and decrypt...
		hex2bin(KEY_LENGTH, RegKey, dest);
		encBuf(dest, (UInt16) KEY_LENGTH, encKey, true);
		dest[KEY_LENGTH] = chrNull;
	
		return (StrCompare(source, dest) == 0);
	}
	else
		return false;	
}

/*
 * Encrypt.c
 */
