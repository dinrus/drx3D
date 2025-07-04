/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include "tomcrypt.h"

#if defined(LTC_MRSA) || (!defined(LTC_NO_MATH) && !defined(LTC_NO_PRNGS))

/**
  @file rand_prime.c
  Generate a random prime, Tom St Denis
*/

#define USE_BBS 1

i32 rand_prime(uk N, long len, prng_state *prng, i32 wprng)
{
   i32            err, res, type;
   u8 *buf;

   LTC_ARGCHK(N != NULL);

   /* get type */
   if (len < 0) {
      type = USE_BBS;
      len = -len;
   } else {
      type = 0;
   }

   /* allow sizes between 2 and 512 bytes for a prime size */
   if (len < 2 || len > 512) {
      return CRYPT_INVALID_PRIME_SIZE;
   }

   /* valid PRNG? Better be! */
   if ((err = prng_is_valid(wprng)) != CRYPT_OK) {
      return err;
   }

   /* allocate buffer to work with */
   buf = XCALLOC(1, len);
   if (buf == NULL) {
       return CRYPT_MEM;
   }

   do {
      /* generate value */
      if (prng_descriptor[wprng].read(buf, len, prng) != (u64)len) {
         XFREE(buf);
         return CRYPT_ERROR_READPRNG;
      }

      /* munge bits */
      buf[0]     |= 0x80 | 0x40;
      buf[len-1] |= 0x01 | ((type & USE_BBS) ? 0x02 : 0x00);

      /* load value */
      if ((err = mp_read_unsigned_bin(N, buf, len)) != CRYPT_OK) {
         XFREE(buf);
         return err;
      }

      /* test */
      if ((err = mp_prime_is_prime(N, LTC_MILLER_RABIN_REPS, &res)) != CRYPT_OK) {
         XFREE(buf);
         return err;
      }
   } while (res == LTC_MP_NO);

#ifdef LTC_CLEAN_STACK
   zeromem(buf, len);
#endif

   XFREE(buf);
   return CRYPT_OK;
}

#endif /* LTC_NO_MATH */


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
