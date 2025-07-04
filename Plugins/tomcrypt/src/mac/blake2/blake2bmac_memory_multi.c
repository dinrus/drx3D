/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include "tomcrypt.h"
#include <stdarg.h>

#ifdef LTC_BLAKE2BMAC

/**
   BLAKE2B MAC multiple blocks of memory to produce the authentication tag
   @param key       The secret key
   @param keylen    The length of the secret key (octets)
   @param mac       [out] Destination of the authentication tag
   @param maclen    [in/out] Max size and resulting size of authentication tag
   @param in        The data to BLAKE2B MAC
   @param inlen     The length of the data to BLAKE2B MAC (octets)
   @param ...       tuples of (data,len) pairs to BLAKE2B MAC, terminated with a (NULL,x) (x=don't care)
   @return CRYPT_OK if successful
*/
i32 blake2bmac_memory_multi(u8k *key, u64 keylen, u8 *mac, u64 *maclen, u8k *in,  u64 inlen, ...)
{
   blake2bmac_state st;
   i32 err;
   va_list args;
   u8k *curptr;
   u64 curlen;

   LTC_ARGCHK(key    != NULL);
   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(mac    != NULL);
   LTC_ARGCHK(maclen != NULL);

   va_start(args, inlen);
   curptr = in;
   curlen = inlen;
   if ((err = blake2bmac_init(&st, *maclen, key, keylen)) != CRYPT_OK)          { goto LBL_ERR; }
   for (;;) {
      if ((err = blake2bmac_process(&st, curptr, curlen)) != CRYPT_OK) { goto LBL_ERR; }
      curptr = va_arg(args, u8k*);
      if (curptr == NULL) break;
      curlen = va_arg(args, u64);
   }
   err = blake2bmac_done(&st, mac, maclen);
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(&st, sizeof(blake2bmac_state));
#endif
   va_end(args);
   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
