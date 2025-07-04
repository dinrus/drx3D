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
  @file f9_done.c
  f9 Support, terminate the state
*/

#ifdef LTC_F9_MODE

/** Terminate the f9-MAC state
  @param f9     f9 state to terminate
  @param out      [out] Destination for the MAC tag
  @param outlen   [in/out] Destination size and final tag size
  Return CRYPT_OK on success
*/
i32 f9_done(f9_state *f9, u8 *out, u64 *outlen)
{
   i32 err, x;
   LTC_ARGCHK(f9 != NULL);
   LTC_ARGCHK(out  != NULL);

   /* check structure */
   if ((err = cipher_is_valid(f9->cipher)) != CRYPT_OK) {
      return err;
   }

   if ((f9->blocksize > cipher_descriptor[f9->cipher].block_length) || (f9->blocksize < 0) ||
       (f9->buflen > f9->blocksize) || (f9->buflen < 0)) {
      return CRYPT_INVALID_ARG;
   }

   if (f9->buflen != 0) {
      /* encrypt */
      cipher_descriptor[f9->cipher].ecb_encrypt(f9->IV, f9->IV, &f9->key);
      f9->buflen = 0;
      for (x = 0; x < f9->blocksize; x++) {
         f9->ACC[x] ^= f9->IV[x];
      }
   }

   /* schedule modified key */
   if ((err = cipher_descriptor[f9->cipher].setup(f9->akey, f9->keylen, 0, &f9->key)) != CRYPT_OK) {
      return err;
   }

   /* encrypt the ACC */
   cipher_descriptor[f9->cipher].ecb_encrypt(f9->ACC, f9->ACC, &f9->key);
   cipher_descriptor[f9->cipher].done(&f9->key);

   /* extract tag */
   for (x = 0; x < f9->blocksize && (u64)x < *outlen; x++) {
      out[x] = f9->ACC[x];
   }
   *outlen = x;

#ifdef LTC_CLEAN_STACK
   zeromem(f9, sizeof(*f9));
#endif
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */

