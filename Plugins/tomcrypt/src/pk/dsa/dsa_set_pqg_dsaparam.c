/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include "tomcrypt.h"


#ifdef LTC_MDSA

/**
  Import DSA's p, q & g from dsaparam

      dsaparam data: openssl dsaparam -outform DER -out dsaparam.der 2048

  @param dsaparam    The DSA param DER encoded data
  @param dsaparamlen The length of dhparam data
  @param key         [out] the destination for the imported key
  @return CRYPT_OK if successful.
*/
i32 dsa_set_pqg_dsaparam(u8k *dsaparam, u64 dsaparamlen,
                         dsa_key *key)
{
   i32 err, stat;

   LTC_ARGCHK(dsaparam    != NULL);
   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   /* init key */
   err = mp_init_multi(&key->p, &key->g, &key->q, &key->x, &key->y, NULL);
   if (err != CRYPT_OK) return err;

   if ((err = der_decode_sequence_multi(dsaparam, dsaparamlen,
                                        LTC_ASN1_INTEGER, 1UL, key->p,
                                        LTC_ASN1_INTEGER, 1UL, key->q,
                                        LTC_ASN1_INTEGER, 1UL, key->g,
                                        LTC_ASN1_EOL,     0UL, NULL)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   key->qord = mp_unsigned_bin_size(key->q);

   /* quick p, q, g validation, without primality testing */
   if ((err = dsa_int_validate_pqg(key, &stat)) != CRYPT_OK) {
      goto LBL_ERR;
   }
   if (stat == 0) {
      err = CRYPT_INVALID_PACKET;
      goto LBL_ERR;
   }

   return CRYPT_OK;

LBL_ERR:
   dsa_free(key);
   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
