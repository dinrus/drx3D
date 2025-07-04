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

#ifdef LTC_POLY1305

i32 poly1305_test(void)
{
#ifndef LTC_TEST
   return CRYPT_NOP;
#else
   /* https://tools.ietf.org/html/rfc7539#section-2.5.2 */
   u8 k[]   = { 0x85, 0xd6, 0xbe, 0x78, 0x57, 0x55, 0x6d, 0x33, 0x7f, 0x44, 0x52, 0xfe, 0x42, 0xd5, 0x06, 0xa8, 0x01, 0x03, 0x80, 0x8a, 0xfb, 0x0d, 0xb2, 0xfd, 0x4a, 0xbf, 0xf6, 0xaf, 0x41, 0x49, 0xf5, 0x1b };
   u8 tag[] = { 0xA8, 0x06, 0x1D, 0xC1, 0x30, 0x51, 0x36, 0xC6, 0xC2, 0x2B, 0x8B, 0xAF, 0x0C, 0x01, 0x27, 0xA9 };
   char m[] = "Cryptographic Forum Research Group";
   u64 len = 16, mlen = strlen(m);
   u8 out[1000];
   poly1305_state st;
   i32 err;

   /* process piece by piece */
   if ((err = poly1305_init(&st, k, 32)) != CRYPT_OK)                                return err;
   if ((err = poly1305_process(&st, (u8*)m,      5)) != CRYPT_OK)         return err;
   if ((err = poly1305_process(&st, (u8*)m + 5,  4)) != CRYPT_OK)         return err;
   if ((err = poly1305_process(&st, (u8*)m + 9,  3)) != CRYPT_OK)         return err;
   if ((err = poly1305_process(&st, (u8*)m + 12, 2)) != CRYPT_OK)         return err;
   if ((err = poly1305_process(&st, (u8*)m + 14, 1)) != CRYPT_OK)         return err;
   if ((err = poly1305_process(&st, (u8*)m + 15, mlen - 15)) != CRYPT_OK) return err;
   if ((err = poly1305_done(&st, out, &len)) != CRYPT_OK)                            return err;
   if (compare_testvector(out, len, tag, sizeof(tag), "POLY1305-TV1", 1) != 0)       return CRYPT_FAIL_TESTVECTOR;
   /* process in one go */
   if ((err = poly1305_init(&st, k, 32)) != CRYPT_OK)                                return err;
   if ((err = poly1305_process(&st, (u8*)m, mlen)) != CRYPT_OK)           return err;
   if ((err = poly1305_done(&st, out, &len)) != CRYPT_OK)                            return err;
   if (compare_testvector(out, len, tag, sizeof(tag), "POLY1305-TV2", 1) != 0)       return CRYPT_FAIL_TESTVECTOR;
   return CRYPT_OK;
#endif
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
