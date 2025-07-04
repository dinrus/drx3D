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
  Import DSA's p, q & g from raw numbers
  @param p       DSA's p  in binary representation
  @param plen    The length of p
  @param q       DSA's q  in binary representation
  @param qlen    The length of q
  @param g       DSA's g  in binary representation
  @param glen    The length of g
  @param key     [out] the destination for the imported key
  @return CRYPT_OK if successful.
*/
i32 dsa_set_pqg(u8k *p,  u64 plen,
                u8k *q,  u64 qlen,
                u8k *g,  u64 glen,
                dsa_key *key)
{
   i32 err, stat;

   LTC_ARGCHK(p           != NULL);
   LTC_ARGCHK(q           != NULL);
   LTC_ARGCHK(g           != NULL);
   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   /* init key */
   err = mp_init_multi(&key->p, &key->g, &key->q, &key->x, &key->y, NULL);
   if (err != CRYPT_OK) return err;

   if ((err = mp_read_unsigned_bin(key->p, (u8*)p , plen)) != CRYPT_OK) { goto LBL_ERR; }
   if ((err = mp_read_unsigned_bin(key->g, (u8*)g , glen)) != CRYPT_OK) { goto LBL_ERR; }
   if ((err = mp_read_unsigned_bin(key->q, (u8*)q , qlen)) != CRYPT_OK) { goto LBL_ERR; }

   key->qord = mp_unsigned_bin_size(key->q);

   /* do only a quick validation, without primality testing */
   if ((err = dsa_int_validate_pqg(key, &stat)) != CRYPT_OK)                        { goto LBL_ERR; }
   if (stat == 0) {
      err = CRYPT_INVALID_PACKET;
      goto LBL_ERR;
   }

   return CRYPT_OK;

LBL_ERR:
   dsa_free(key);
   return err;
}

/**
  Import DSA public or private key-part from raw numbers

     NB: The p, q & g parts must be set beforehand

  @param in      The key-part to import, either public or private.
  @param inlen   The key-part's length
  @param type    Which type of key (PK_PRIVATE or PK_PUBLIC)
  @param key     [out] the destination for the imported key
  @return CRYPT_OK if successful.
*/
i32 dsa_set_key(u8k *in, u64 inlen, i32 type, dsa_key *key)
{
   i32 err, stat = 0;

   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(key->x      != NULL);
   LTC_ARGCHK(key->y      != NULL);
   LTC_ARGCHK(key->p      != NULL);
   LTC_ARGCHK(key->g      != NULL);
   LTC_ARGCHK(key->q      != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   if (type == PK_PRIVATE) {
      key->type = PK_PRIVATE;
      if ((err = mp_read_unsigned_bin(key->x, (u8*)in, inlen)) != CRYPT_OK) { goto LBL_ERR; }
      if ((err = mp_exptmod(key->g, key->x, key->p, key->y)) != CRYPT_OK)               { goto LBL_ERR; }
   }
   else {
      key->type = PK_PUBLIC;
      if ((err = mp_read_unsigned_bin(key->y, (u8*)in, inlen)) != CRYPT_OK) { goto LBL_ERR; }
   }

   if ((err = dsa_int_validate_xy(key, &stat)) != CRYPT_OK)                             { goto LBL_ERR; }
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
