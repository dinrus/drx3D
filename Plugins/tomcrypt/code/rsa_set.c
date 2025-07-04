/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>


#ifdef LTC_MRSA

/**
  Import RSA key from raw numbers

  @param N       RSA's N
  @param Nlen    RSA's N's length
  @param e       RSA's e
  @param elen    RSA's e's length
  @param d       RSA's d  (only private key, NULL for public key)
  @param dlen    RSA's d's length
  @param key     [out] the destination for the imported key
  @return CRYPT_OK if successful
*/
i32 rsa_set_key(u8k *N,  u64 Nlen,
                u8k *e,  u64 elen,
                u8k *d,  u64 dlen,
                rsa_key *key)
{
   i32 err;

   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(N           != NULL);
   LTC_ARGCHK(e           != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   err = mp_init_multi(&key->e, &key->d, &key->N, &key->dQ, &key->dP, &key->qP, &key->p, &key->q, NULL);
   if (err != CRYPT_OK) return err;

   if ((err = mp_read_unsigned_bin(key->N , (u8*)N , Nlen)) != CRYPT_OK)    { goto LBL_ERR; }
   if ((err = mp_read_unsigned_bin(key->e , (u8*)e , elen)) != CRYPT_OK)    { goto LBL_ERR; }
   if (d && dlen) {
      if ((err = mp_read_unsigned_bin(key->d , (u8*)d , dlen)) != CRYPT_OK) { goto LBL_ERR; }
      key->type = PK_PRIVATE;
   }
   else {
      key->type = PK_PUBLIC;
   }
   return CRYPT_OK;

LBL_ERR:
   rsa_free(key);
   return err;
}

/**
  Import factors of an RSA key from raw numbers

  Only for private keys.

  @param p       RSA's p
  @param plen    RSA's p's length
  @param q       RSA's q
  @param qlen    RSA's q's length
  @param key     [out] the destination for the imported key
  @return CRYPT_OK if successful
*/
i32 rsa_set_factors(u8k *p,  u64 plen,
                    u8k *q,  u64 qlen,
                    rsa_key *key)
{
   i32 err;

   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(p           != NULL);
   LTC_ARGCHK(q           != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   if (key->type != PK_PRIVATE) return CRYPT_PK_TYPE_MISMATCH;

   if ((err = mp_read_unsigned_bin(key->p , (u8*)p , plen)) != CRYPT_OK) { goto LBL_ERR; }
   if ((err = mp_read_unsigned_bin(key->q , (u8*)q , qlen)) != CRYPT_OK) { goto LBL_ERR; }
   return CRYPT_OK;

LBL_ERR:
   rsa_free(key);
   return err;
}

/**
  Import CRT parameters of an RSA key from raw numbers

  Only for private keys.

  @param dP      RSA's dP
  @param dPlen   RSA's dP's length
  @param dQ      RSA's dQ
  @param dQlen   RSA's dQ's length
  @param qP      RSA's qP
  @param qPlen   RSA's qP's length
  @param key     [out] the destination for the imported key
  @return CRYPT_OK if successful
*/
i32 rsa_set_crt_params(u8k *dP, u64 dPlen,
                       u8k *dQ, u64 dQlen,
                       u8k *qP, u64 qPlen,
                       rsa_key *key)
{
   i32 err;

   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(dP          != NULL);
   LTC_ARGCHK(dQ          != NULL);
   LTC_ARGCHK(qP          != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   if (key->type != PK_PRIVATE) return CRYPT_PK_TYPE_MISMATCH;

   if ((err = mp_read_unsigned_bin(key->dP, (u8*)dP, dPlen)) != CRYPT_OK) { goto LBL_ERR; }
   if ((err = mp_read_unsigned_bin(key->dQ, (u8*)dQ, dQlen)) != CRYPT_OK) { goto LBL_ERR; }
   if ((err = mp_read_unsigned_bin(key->qP, (u8*)qP, qPlen)) != CRYPT_OK) { goto LBL_ERR; }
   return CRYPT_OK;

LBL_ERR:
   rsa_free(key);
   return err;
}

#endif /* LTC_MRSA */

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
