/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* Implements ECC over Z/pZ for curve y^2 = x^3 - 3x + b
 *
 * All curves taken from NIST recommendation paper of July 1999
 * Available at http://csrc.nist.gov/cryptval/dss.htm
 */
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

/**
  @file ecc_shared_secret.c
  ECC Crypto, Tom St Denis
*/

#ifdef LTC_MECC

/**
  Create an ECC shared secret between two keys
  @param private_key      The private ECC key
  @param public_key       The public key
  @param out              [out] Destination of the shared secret (Conforms to EC-DH from ANSI X9.63)
  @param outlen           [in/out] The max size and resulting size of the shared secret
  @return CRYPT_OK if successful
*/
i32 ecc_shared_secret(ecc_key *private_key, ecc_key *public_key,
                      u8 *out, u64 *outlen)
{
   u64  x;
   ecc_point     *result;
   void          *prime;
   i32            err;

   LTC_ARGCHK(private_key != NULL);
   LTC_ARGCHK(public_key  != NULL);
   LTC_ARGCHK(out         != NULL);
   LTC_ARGCHK(outlen      != NULL);

   /* type valid? */
   if (private_key->type != PK_PRIVATE) {
      return CRYPT_PK_NOT_PRIVATE;
   }

   if (ltc_ecc_is_valid_idx(private_key->idx) == 0 || ltc_ecc_is_valid_idx(public_key->idx) == 0) {
      return CRYPT_INVALID_ARG;
   }

   if (XSTRCMP(private_key->dp->name, public_key->dp->name) != 0) {
      return CRYPT_PK_TYPE_MISMATCH;
   }

   /* make new point */
   result = ltc_ecc_new_point();
   if (result == NULL) {
      return CRYPT_MEM;
   }

   if ((err = mp_init(&prime)) != CRYPT_OK) {
      ltc_ecc_del_point(result);
      return err;
   }

   if ((err = mp_read_radix(prime, (char *)private_key->dp->prime, 16)) != CRYPT_OK)                               { goto done; }
   if ((err = ltc_mp.ecc_ptmul(private_key->k, &public_key->pubkey, result, prime, 1)) != CRYPT_OK)                { goto done; }

   x = (u64)mp_unsigned_bin_size(prime);
   if (*outlen < x) {
      *outlen = x;
      err = CRYPT_BUFFER_OVERFLOW;
      goto done;
   }
   zeromem(out, x);
   if ((err = mp_to_unsigned_bin(result->x, out + (x - mp_unsigned_bin_size(result->x))))   != CRYPT_OK)           { goto done; }

   err     = CRYPT_OK;
   *outlen = x;
done:
   mp_clear(prime);
   ltc_ecc_del_point(result);
   return err;
}

#endif
/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */

