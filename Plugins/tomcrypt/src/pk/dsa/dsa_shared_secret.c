/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include "tomcrypt.h"

/**
  @file dsa_shared_secret.c
  DSA Crypto, Tom St Denis
*/

#ifdef LTC_MDSA

/**
  Create a DSA shared secret between two keys
  @param private_key      The private DSA key (the exponent)
  @param base             The base of the exponentiation (allows this to be used for both encrypt and decrypt)
  @param public_key       The public key
  @param out              [out] Destination of the shared secret
  @param outlen           [in/out] The max size and resulting size of the shared secret
  @return CRYPT_OK if successful
*/
i32 dsa_shared_secret(void          *private_key, uk base,
                      dsa_key       *public_key,
                      u8 *out,         u64 *outlen)
{
   u64  x;
   void          *res;
   i32            err;

   LTC_ARGCHK(private_key != NULL);
   LTC_ARGCHK(public_key  != NULL);
   LTC_ARGCHK(out         != NULL);
   LTC_ARGCHK(outlen      != NULL);

   /* make new point */
   if ((err = mp_init(&res)) != CRYPT_OK) {
      return err;
   }

   if ((err = mp_exptmod(base, private_key, public_key->p, res)) != CRYPT_OK) {
      mp_clear(res);
      return err;
   }

   x = (u64)mp_unsigned_bin_size(res);
   if (*outlen < x) {
      *outlen = x;
      err = CRYPT_BUFFER_OVERFLOW;
      goto done;
   }
   zeromem(out, x);
   if ((err = mp_to_unsigned_bin(res, out + (x - mp_unsigned_bin_size(res))))   != CRYPT_OK)          { goto done; }

   err     = CRYPT_OK;
   *outlen = x;
done:
   mp_clear(res);
   return err;
}

#endif
/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */

