/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* The implementation is based on:
 * Public Domain poly1305 from Andrew Moon
 * https://github.com/floodyberry/poly1305-donna
 */

#include "tomcrypt.h"
#include <stdarg.h>

#ifdef LTC_POLY1305

/**
   POLY1305 multiple blocks of memory to produce the authentication tag
   @param key       The secret key
   @param keylen    The length of the secret key (octets)
   @param mac       [out] Destination of the authentication tag
   @param maclen    [in/out] Max size and resulting size of authentication tag
   @param in        The data to POLY1305
   @param inlen     The length of the data to POLY1305 (octets)
   @param ...       tuples of (data,len) pairs to POLY1305, terminated with a (NULL,x) (x=don't care)
   @return CRYPT_OK if successful
*/
i32 poly1305_memory_multi(u8k *key, u64 keylen, u8 *mac, u64 *maclen, u8k *in,  u64 inlen, ...)
{
   poly1305_state st;
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
   if ((err = poly1305_init(&st, key, keylen)) != CRYPT_OK)          { goto LBL_ERR; }
   for (;;) {
      if ((err = poly1305_process(&st, curptr, curlen)) != CRYPT_OK) { goto LBL_ERR; }
      curptr = va_arg(args, u8k*);
      if (curptr == NULL) break;
      curlen = va_arg(args, u64);
   }
   err = poly1305_done(&st, mac, maclen);
LBL_ERR:
#ifdef LTC_CLEAN_STACK
   zeromem(&st, sizeof(poly1305_state));
#endif
   va_end(args);
   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
