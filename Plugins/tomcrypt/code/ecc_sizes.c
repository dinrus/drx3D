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
  @file ecc_sizes.c
  ECC Crypto, Tom St Denis
*/

#ifdef LTC_MECC

void ecc_sizes(i32 *low, i32 *high)
{
 i32 i;
 LTC_ARGCHKVD(low  != NULL);
 LTC_ARGCHKVD(high != NULL);

 *low = INT_MAX;
 *high = 0;
 for (i = 0; ltc_ecc_sets[i].size != 0; i++) {
     if (ltc_ecc_sets[i].size < *low)  {
        *low  = ltc_ecc_sets[i].size;
     }
     if (ltc_ecc_sets[i].size > *high) {
        *high = ltc_ecc_sets[i].size;
     }
 }
}

#endif
/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */

