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
   @file dsa_export.c
   DSA implementation, export key, Tom St Denis
*/

#ifdef LTC_MDSA

/**
  Export a DSA key to a binary packet
  @param out    [out] Where to store the packet
  @param outlen [in/out] The max size and resulting size of the packet
  @param type   The type of key to export (PK_PRIVATE or PK_PUBLIC)
  @param key    The key to export
  @return CRYPT_OK if successful
*/
i32 dsa_export(u8 *out, u64 *outlen, i32 type, dsa_key *key)
{
   u64 zero=0;
   i32 err, std;

   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);
   LTC_ARGCHK(key    != NULL);

   std = type & PK_STD;
   type &= ~PK_STD;

   /* can we store the static header?  */
   if (type == PK_PRIVATE && key->type != PK_PRIVATE) {
      return CRYPT_PK_TYPE_MISMATCH;
   }

   if (type != PK_PUBLIC && type != PK_PRIVATE) {
      return CRYPT_INVALID_ARG;
   }

   if (type == PK_PRIVATE) {
      if (std) {
          return der_encode_sequence_multi(out, outlen,
                                         LTC_ASN1_SHORT_INTEGER, 1UL, &zero,
                                         LTC_ASN1_INTEGER,      1UL, key->p,
                                         LTC_ASN1_INTEGER,      1UL, key->q,
                                         LTC_ASN1_INTEGER,      1UL, key->g,
                                         LTC_ASN1_INTEGER,      1UL, key->y,
                                         LTC_ASN1_INTEGER,      1UL, key->x,
                                         LTC_ASN1_EOL,          0UL, NULL);
      }
      else {
          u8 flags[1];
          flags[0] = 1;
          return der_encode_sequence_multi(out, outlen,
                                         LTC_ASN1_BIT_STRING,   1UL, flags,
                                         LTC_ASN1_INTEGER,      1UL, key->g,
                                         LTC_ASN1_INTEGER,      1UL, key->p,
                                         LTC_ASN1_INTEGER,      1UL, key->q,
                                         LTC_ASN1_INTEGER,      1UL, key->y,
                                         LTC_ASN1_INTEGER,      1UL, key->x,
                                         LTC_ASN1_EOL,          0UL, NULL);
      }
   } else {
      if (std) {
          u64 tmplen = (u64)(mp_count_bits(key->y) / 8) + 8;
          u8* tmp = XMALLOC(tmplen);
          ltc_asn1_list int_list[3];

          if (tmp == NULL) {
              return CRYPT_MEM;
          }

          err = der_encode_integer(key->y, tmp, &tmplen);
          if (err != CRYPT_OK) {
              goto error;
          }

          LTC_SET_ASN1(int_list, 0, LTC_ASN1_INTEGER, key->p, 1UL);
          LTC_SET_ASN1(int_list, 1, LTC_ASN1_INTEGER, key->q, 1UL);
          LTC_SET_ASN1(int_list, 2, LTC_ASN1_INTEGER, key->g, 1UL);

          err = der_encode_subject_public_key_info(out, outlen, PKA_DSA, tmp,
              tmplen, LTC_ASN1_SEQUENCE, int_list,
              sizeof(int_list) / sizeof(int_list[0]));

error:
          XFREE(tmp);
          return err;
      }
      else {
          u8 flags[1];
          flags[0] = 0;
          return der_encode_sequence_multi(out, outlen,
                                     LTC_ASN1_BIT_STRING,   1UL, flags,
                                     LTC_ASN1_INTEGER,      1UL, key->g,
                                     LTC_ASN1_INTEGER,      1UL, key->p,
                                     LTC_ASN1_INTEGER,      1UL, key->q,
                                     LTC_ASN1_INTEGER,      1UL, key->y,
                                     LTC_ASN1_EOL,          0UL, NULL);
      }
   }
}

#endif


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
