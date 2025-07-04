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
  @file rsa_sign_hash.c
  RSA PKCS #1 v1.5 and v2 PSS sign hash, Tom St Denis and Andreas Lange
*/

#ifdef LTC_MRSA

/**
  PKCS #1 pad then sign
  @param in        The hash to sign
  @param inlen     The length of the hash to sign (octets)
  @param out       [out] The signature
  @param outlen    [in/out] The max size and resulting size of the signature
  @param padding   Type of padding (LTC_PKCS_1_PSS, LTC_PKCS_1_V1_5 or LTC_PKCS_1_V1_5_NA1)
  @param prng      An active PRNG state
  @param prng_idx  The index of the PRNG desired
  @param hash_idx  The index of the hash desired
  @param saltlen   The length of the salt desired (octets)
  @param key       The private RSA key to use
  @return CRYPT_OK if successful
*/
i32 rsa_sign_hash_ex(u8k *in,       u64  inlen,
                           u8 *out,      u64 *outlen,
                           i32            padding,
                           prng_state    *prng,     i32            prng_idx,
                           i32            hash_idx, u64  saltlen,
                           rsa_key *key)
{
   u64 modulus_bitlen, modulus_bytelen, x, y;
   i32           err;

   LTC_ARGCHK(in       != NULL);
   LTC_ARGCHK(out      != NULL);
   LTC_ARGCHK(outlen   != NULL);
   LTC_ARGCHK(key      != NULL);

   /* valid padding? */
   if ((padding != LTC_PKCS_1_V1_5) &&
       (padding != LTC_PKCS_1_PSS) &&
       (padding != LTC_PKCS_1_V1_5_NA1)) {
     return CRYPT_PK_INVALID_PADDING;
   }

   if (padding == LTC_PKCS_1_PSS) {
     /* valid prng ? */
     if ((err = prng_is_valid(prng_idx)) != CRYPT_OK) {
        return err;
     }
   }

   if (padding != LTC_PKCS_1_V1_5_NA1) {
     /* valid hash ? */
     if ((err = hash_is_valid(hash_idx)) != CRYPT_OK) {
        return err;
     }
   }

   /* get modulus len in bits */
   modulus_bitlen = mp_count_bits((key->N));

  /* outlen must be at least the size of the modulus */
  modulus_bytelen = mp_unsigned_bin_size((key->N));
  if (modulus_bytelen > *outlen) {
     *outlen = modulus_bytelen;
     return CRYPT_BUFFER_OVERFLOW;
  }

  if (padding == LTC_PKCS_1_PSS) {
    /* PSS pad the key */
    x = *outlen;
    if ((err = pkcs_1_pss_encode(in, inlen, saltlen, prng, prng_idx,
                                 hash_idx, modulus_bitlen, out, &x)) != CRYPT_OK) {
       return err;
    }
  } else {
    /* PKCS #1 v1.5 pad the hash */
    u8 *tmpin;

    if (padding == LTC_PKCS_1_V1_5) {
      ltc_asn1_list digestinfo[2], siginfo[2];
      /* not all hashes have OIDs... so sad */
      if (hash_descriptor[hash_idx].OIDlen == 0) {
         return CRYPT_INVALID_ARG;
      }

    /* construct the SEQUENCE
        SEQUENCE {
           SEQUENCE {hashoid OID
                     blah    NULL
           }
         hash    OCTET STRING
        }
     */
      LTC_SET_ASN1(digestinfo, 0, LTC_ASN1_OBJECT_IDENTIFIER, hash_descriptor[hash_idx].OID, hash_descriptor[hash_idx].OIDlen);
      LTC_SET_ASN1(digestinfo, 1, LTC_ASN1_NULL,              NULL,                          0);
      LTC_SET_ASN1(siginfo,    0, LTC_ASN1_SEQUENCE,          digestinfo,                    2);
      LTC_SET_ASN1(siginfo,    1, LTC_ASN1_OCTET_STRING,      in,                            inlen);

      /* allocate memory for the encoding */
      y = mp_unsigned_bin_size(key->N);
      tmpin = XMALLOC(y);
      if (tmpin == NULL) {
         return CRYPT_MEM;
      }

      if ((err = der_encode_sequence(siginfo, 2, tmpin, &y)) != CRYPT_OK) {
         XFREE(tmpin);
         return err;
      }
    } else {
      /* set the pointer and data-length to the input values */
      tmpin = (u8*)in;
      y = inlen;
    }

    x = *outlen;
    err = pkcs_1_v1_5_encode(tmpin, y, LTC_PKCS_1_EMSA, modulus_bitlen, NULL, 0, out, &x);

    if (padding == LTC_PKCS_1_V1_5) {
      XFREE(tmpin);
    }

    if (err != CRYPT_OK) {
      return err;
    }
  }

  /* RSA encode it */
  return ltc_mp.rsa_me(out, x, out, outlen, PK_PRIVATE, key);
}

#endif /* LTC_MRSA */

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
