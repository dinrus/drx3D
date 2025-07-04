/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifdef LTC_RNG_MAKE_PRNG
/**
  @file rng_make_prng.c
  portable way to get secure random bits to feed a PRNG  (Tom St Denis)
*/

/**
  Create a PRNG from a RNG
  @param bits     Number of bits of entropy desired (64 ... 1024)
  @param wprng    Index of which PRNG to setup
  @param prng     [out] PRNG state to initialize
  @param callback A pointer to a void function for when the RNG is slow, this can be NULL
  @return CRYPT_OK if successful
*/
i32 rng_make_prng(i32 bits, i32 wprng, prng_state *prng,
                  void (*callback)(void))
{
   u8 buf[256];
   i32 err;

   LTC_ARGCHK(prng != NULL);

   /* check parameter */
   if ((err = prng_is_valid(wprng)) != CRYPT_OK) {
      return err;
   }

   if (bits < 64 || bits > 1024) {
      return CRYPT_INVALID_PRNGSIZE;
   }

   if ((err = prng_descriptor[wprng].start(prng)) != CRYPT_OK) {
      return err;
   }

   bits = ((bits/8)+((bits&7)!=0?1:0)) * 2;
   if (rng_get_bytes(buf, (u64)bits, callback) != (u64)bits) {
      return CRYPT_ERROR_READPRNG;
   }

   if ((err = prng_descriptor[wprng].add_entropy(buf, (u64)bits, prng)) != CRYPT_OK) {
      return err;
   }

   if ((err = prng_descriptor[wprng].ready(prng)) != CRYPT_OK) {
      return err;
   }

   #ifdef LTC_CLEAN_STACK
      zeromem(buf, sizeof(buf));
   #endif
   return CRYPT_OK;
}
#endif /* #ifdef LTC_RNG_MAKE_PRNG */


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
