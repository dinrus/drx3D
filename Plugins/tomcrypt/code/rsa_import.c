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
  @file rsa_import.c
  Import a PKCS RSA key, Tom St Denis
*/

#ifdef LTC_MRSA

/**
  Import an RSAPublicKey or RSAPrivateKey [two-prime only, only support >= 1024-bit keys, defined in PKCS #1 v2.1]
  @param in      The packet to import from
  @param inlen   It's length (octets)
  @param key     [out] Destination for newly imported key
  @return CRYPT_OK if successful, upon error allocated memory is freed
*/
i32 rsa_import(u8k *in, u64 inlen, rsa_key *key)
{
   i32           err;
   void         *zero;
   u8 *tmpbuf=NULL;
   u64 tmpbuf_len;

   LTC_ARGCHK(in          != NULL);
   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   /* init key */
   if ((err = mp_init_multi(&key->e, &key->d, &key->N, &key->dQ,
                            &key->dP, &key->qP, &key->p, &key->q, NULL)) != CRYPT_OK) {
      return err;
   }

   /* see if the OpenSSL DER format RSA public key will work */
   tmpbuf_len = inlen;
   tmpbuf = XCALLOC(1, tmpbuf_len);
   if (tmpbuf == NULL) {
       err = CRYPT_MEM;
       goto LBL_ERR;
   }

   err = der_decode_subject_public_key_info(in, inlen,
        PKA_RSA, tmpbuf, &tmpbuf_len,
        LTC_ASN1_NULL, NULL, 0);

   if (err == CRYPT_OK) { /* SubjectPublicKeyInfo format */

      /* now it should be SEQUENCE { INTEGER, INTEGER } */
      if ((err = der_decode_sequence_multi(tmpbuf, tmpbuf_len,
                                           LTC_ASN1_INTEGER, 1UL, key->N,
                                           LTC_ASN1_INTEGER, 1UL, key->e,
                                           LTC_ASN1_EOL,     0UL, NULL)) != CRYPT_OK) {
         goto LBL_ERR;
      }
      key->type = PK_PUBLIC;
      err = CRYPT_OK;
      goto LBL_FREE;
   }

   /* not SSL public key, try to match against PKCS #1 standards */
   err = der_decode_sequence_multi(in, inlen, LTC_ASN1_INTEGER, 1UL, key->N,
                                              LTC_ASN1_EOL,     0UL, NULL);

   if (err != CRYPT_OK && err != CRYPT_INPUT_TOO_LONG) {
      goto LBL_ERR;
   }

   if (mp_cmp_d(key->N, 0) == LTC_MP_EQ) {
      if ((err = mp_init(&zero)) != CRYPT_OK) {
         goto LBL_ERR;
      }
      /* it's a private key */
      if ((err = der_decode_sequence_multi(in, inlen,
                          LTC_ASN1_INTEGER, 1UL, zero,
                          LTC_ASN1_INTEGER, 1UL, key->N,
                          LTC_ASN1_INTEGER, 1UL, key->e,
                          LTC_ASN1_INTEGER, 1UL, key->d,
                          LTC_ASN1_INTEGER, 1UL, key->p,
                          LTC_ASN1_INTEGER, 1UL, key->q,
                          LTC_ASN1_INTEGER, 1UL, key->dP,
                          LTC_ASN1_INTEGER, 1UL, key->dQ,
                          LTC_ASN1_INTEGER, 1UL, key->qP,
                          LTC_ASN1_EOL,     0UL, NULL)) != CRYPT_OK) {
         mp_clear(zero);
         goto LBL_ERR;
      }
      mp_clear(zero);
      key->type = PK_PRIVATE;
   } else if (mp_cmp_d(key->N, 1) == LTC_MP_EQ) {
      /* we don't support multi-prime RSA */
      err = CRYPT_PK_INVALID_TYPE;
      goto LBL_ERR;
   } else {
      /* it's a public key and we lack e */
      if ((err = der_decode_sequence_multi(in, inlen,
                                     LTC_ASN1_INTEGER, 1UL, key->N,
                                     LTC_ASN1_INTEGER, 1UL, key->e,
                                     LTC_ASN1_EOL,     0UL, NULL)) != CRYPT_OK) {
         goto LBL_ERR;
      }
      key->type = PK_PUBLIC;
   }
   err = CRYPT_OK;
   goto LBL_FREE;

LBL_ERR:
   mp_clear_multi(key->d,  key->e, key->N, key->dQ, key->dP, key->qP, key->p, key->q, NULL);

LBL_FREE:
   if (tmpbuf != NULL)
     XFREE(tmpbuf);

   return err;
}

#endif /* LTC_MRSA */


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
