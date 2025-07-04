/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#if defined(LTC_MDSA) || defined(LTC_MECC)
/**
  Generate a random number N with given bitlength (note: MSB can be 0)
*/

i32 rand_bn_bits(uk N, i32 bits, prng_state *prng, i32 wprng)
{
   i32 res, bytes;
   u8 *buf, mask;

   LTC_ARGCHK(N != NULL);
   LTC_ARGCHK(bits > 1);

   /* check PRNG */
   if ((res = prng_is_valid(wprng)) != CRYPT_OK) return res;

   bytes = (bits+7) >> 3;
   mask = 0xff << (8 - bits % 8);

   /* allocate buffer */
   if ((buf = XCALLOC(1, bytes)) == NULL) return CRYPT_MEM;

   /* generate random bytes */
   if (prng_descriptor[wprng].read(buf, bytes, prng) != (u64)bytes) {
      res = CRYPT_ERROR_READPRNG;
      goto cleanup;
   }
   /* mask bits */
   buf[0] &= ~mask;
   /* load value */
   if ((res = mp_read_unsigned_bin(N, buf, bytes)) != CRYPT_OK) goto cleanup;

   res = CRYPT_OK;

cleanup:
#ifdef LTC_CLEAN_STACK
   zeromem(buf, bytes);
#endif
   XFREE(buf);
   return res;
}

/**
  Generate a random number N in a range: 1 <= N < limit
*/
i32 rand_bn_upto(uk N, uk limit, prng_state *prng, i32 wprng)
{
   i32 res, bits;

   LTC_ARGCHK(N != NULL);
   LTC_ARGCHK(limit != NULL);

   bits = mp_count_bits(limit);
   do {
     res = rand_bn_bits(N, bits, prng, wprng);
     if (res != CRYPT_OK) return res;
   } while (mp_cmp_d(N, 0) != LTC_MP_GT || mp_cmp(N, limit) != LTC_MP_LT);

   return CRYPT_OK;
}
#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
