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
  @file ecc_get_size.c
  ECC Crypto, Tom St Denis
*/

#ifdef LTC_MECC

/**
  Get the size of an ECC key
  @param key    The key to get the size of
  @return The size (octets) of the key or INT_MAX on error
*/
i32 ecc_get_size(ecc_key *key)
{
   LTC_ARGCHK(key != NULL);
   if (ltc_ecc_is_valid_idx(key->idx))
      return key->dp->size;
   else
      return INT_MAX; /* large value known to cause it to fail when passed to ecc_make_key() */
}

#endif
/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */

