/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

/**
   @file cfb_getiv.c
   CFB implementation, get IV, Tom St Denis
*/

#ifdef LTC_CFB_MODE

/**
   Get the current initialization vector
   @param IV   [out] The destination of the initialization vector
   @param len  [in/out]  The max size and resulting size of the initialization vector
   @param cfb  The CFB state
   @return CRYPT_OK if successful
*/
i32 cfb_getiv(u8 *IV, u64 *len, symmetric_CFB *cfb)
{
   LTC_ARGCHK(IV  != NULL);
   LTC_ARGCHK(len != NULL);
   LTC_ARGCHK(cfb != NULL);
   if ((u64)cfb->blocklen > *len) {
      *len = cfb->blocklen;
      return CRYPT_BUFFER_OVERFLOW;
   }
   XMEMCPY(IV, cfb->IV, cfb->blocklen);
   *len = cfb->blocklen;

   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
