/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifdef LTC_MDH

/**
  Import DH key parts p and g from raw numbers

  @param p       DH's p (prime)
  @param plen    DH's p's length
  @param g       DH's g (group)
  @param glen    DH's g's length
  @param key     [out] the destination for the imported key
  @return CRYPT_OK if successful
*/
i32 dh_set_pg(u8k *p, u64 plen,
              u8k *g, u64 glen,
              dh_key *key)
{
   i32 err;

   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(p           != NULL);
   LTC_ARGCHK(g           != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   if ((err = mp_init_multi(&key->x, &key->y, &key->base, &key->prime, NULL)) != CRYPT_OK) {
      return err;
   }

   if ((err = mp_read_unsigned_bin(key->base, (u8*)g, glen)) != CRYPT_OK)     { goto LBL_ERR; }
   if ((err = mp_read_unsigned_bin(key->prime, (u8*)p, plen)) != CRYPT_OK)  { goto LBL_ERR; }

   return CRYPT_OK;

LBL_ERR:
   dh_free(key);
   return err;
}

/**
  Import DH key parts p and g from built-in DH groups

  @param groupsize  The size of the DH group to use
  @param key        [out] Where the newly created DH key will be stored
  @return CRYPT_OK if successful, note: on error all allocated memory will be freed automatically.
*/
i32 dh_set_pg_groupsize(i32 groupsize, dh_key *key)
{
   i32 err, i;

   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);
   LTC_ARGCHK(groupsize   > 0);

   for (i = 0; (groupsize > ltc_dh_sets[i].size) && (ltc_dh_sets[i].size != 0); i++);
   if (ltc_dh_sets[i].size == 0) return CRYPT_INVALID_KEYSIZE;

   if ((err = mp_init_multi(&key->x, &key->y, &key->base, &key->prime, NULL)) != CRYPT_OK) {
      return err;
   }
   if ((err = mp_read_radix(key->base, ltc_dh_sets[i].base, 16)) != CRYPT_OK)  { goto LBL_ERR; }
   if ((err = mp_read_radix(key->prime, ltc_dh_sets[i].prime, 16)) != CRYPT_OK) { goto LBL_ERR; }

   return CRYPT_OK;

LBL_ERR:
   dh_free(key);
   return err;
}

/**
  Import DH public or private key part from raw numbers

     NB: The p & g parts must be set beforehand

  @param in      The key-part to import, either public or private.
  @param inlen   The key-part's length
  @param type    Which type of key (PK_PRIVATE or PK_PUBLIC)
  @param key     [out] the destination for the imported key
  @return CRYPT_OK if successful
*/
i32 dh_set_key(u8k *in, u64 inlen, i32 type, dh_key *key)
{
   i32 err;

   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   if (type == PK_PRIVATE) {
      key->type = PK_PRIVATE;
      if ((err = mp_read_unsigned_bin(key->x, (u8*)in, inlen)) != CRYPT_OK) { goto LBL_ERR; }
      if ((err = mp_exptmod(key->base, key->x, key->prime, key->y)) != CRYPT_OK)       { goto LBL_ERR; }
   }
   else {
      key->type = PK_PUBLIC;
      if ((err = mp_read_unsigned_bin(key->y, (u8*)in, inlen)) != CRYPT_OK) { goto LBL_ERR; }
   }

   /* check public key */
   if ((err = dh_check_pubkey(key)) != CRYPT_OK) {
      goto LBL_ERR;
   }

   return CRYPT_OK;

LBL_ERR:
   dh_free(key);
   return err;
}

#endif /* LTC_MDH */

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
