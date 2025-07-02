/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include "tomcrypt.h"

#ifdef LTC_RC4_STREAM

i32 rc4_stream_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   rc4_state st;
   i32 err;
   u8k key[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
   u8k pt[]  = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
   u8k ct[]  = { 0x75, 0xb7, 0x87, 0x80, 0x99, 0xe0, 0xc5, 0x96 };
   u8 buf[10];

   if ((err = rc4_stream_setup(&st, key, sizeof(key))) != CRYPT_OK)    return err;
   if ((err = rc4_stream_crypt(&st, pt, sizeof(pt), buf)) != CRYPT_OK) return err;
   if (compare_testvector(buf, sizeof(ct), ct, sizeof(ct), "RC4", 0))  return CRYPT_FAIL_TESTVECTOR;
   if ((err = rc4_stream_done(&st)) != CRYPT_OK)                       return err;

   return CRYPT_OK;
#endif
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
