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
#include "tomcrypt.h"

/**
  @file ecc_make_key.c
  ECC Crypto, Tom St Denis
*/

#ifdef LTC_MECC

/**
  Make a new ECC key
  @param prng         An active PRNG state
  @param wprng        The index of the PRNG you wish to use
  @param keysize      The keysize for the new key (in octets from 20 to 65 bytes)
  @param key          [out] Destination of the newly created key
  @return CRYPT_OK if successful, upon error all allocated memory will be freed
*/
i32 ecc_make_key(prng_state *prng, i32 wprng, i32 keysize, ecc_key *key)
{
   i32 x, err;

   /* find key size */
   for (x = 0; (keysize > ltc_ecc_sets[x].size) && (ltc_ecc_sets[x].size != 0); x++);
   keysize = ltc_ecc_sets[x].size;

   if (keysize > ECC_MAXSIZE || ltc_ecc_sets[x].size == 0) {
      return CRYPT_INVALID_KEYSIZE;
   }
   err = ecc_make_key_ex(prng, wprng, key, &ltc_ecc_sets[x]);
   key->idx = x;
   return err;
}

i32 ecc_make_key_ex(prng_state *prng, i32 wprng, ecc_key *key, const ltc_ecc_set_type *dp)
{
   i32            err;
   ecc_point     *base;
   void          *prime, *order;
   u8 *buf;
   i32            keysize;

   LTC_ARGCHK(key         != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);
   LTC_ARGCHK(dp          != NULL);

   /* good prng? */
   if ((err = prng_is_valid(wprng)) != CRYPT_OK) {
      return err;
   }

   key->idx = -1;
   key->dp  = dp;
   keysize  = dp->size;

   /* allocate ram */
   base = NULL;
   buf  = XMALLOC(ECC_MAXSIZE);
   if (buf == NULL) {
      return CRYPT_MEM;
   }

   /* make up random string */
   if (prng_descriptor[wprng].read(buf, (u64)keysize, prng) != (u64)keysize) {
      err = CRYPT_ERROR_READPRNG;
      goto ERR_BUF;
   }

   /* setup the key variables */
   if ((err = mp_init_multi(&key->pubkey.x, &key->pubkey.y, &key->pubkey.z, &key->k, &prime, &order, NULL)) != CRYPT_OK) {
      goto ERR_BUF;
   }
   base = ltc_ecc_new_point();
   if (base == NULL) {
      err = CRYPT_MEM;
      goto errkey;
   }

   /* read in the specs for this key */
   if ((err = mp_read_radix(prime,   (char *)key->dp->prime, 16)) != CRYPT_OK)                  { goto errkey; }
   if ((err = mp_read_radix(order,   (char *)key->dp->order, 16)) != CRYPT_OK)                  { goto errkey; }
   if ((err = mp_read_radix(base->x, (char *)key->dp->Gx, 16)) != CRYPT_OK)                     { goto errkey; }
   if ((err = mp_read_radix(base->y, (char *)key->dp->Gy, 16)) != CRYPT_OK)                     { goto errkey; }
   if ((err = mp_set(base->z, 1)) != CRYPT_OK)                                                  { goto errkey; }
   if ((err = mp_read_unsigned_bin(key->k, (u8*)buf, keysize)) != CRYPT_OK)         { goto errkey; }

   /* the key should be smaller than the order of base point */
   if (mp_cmp(key->k, order) != LTC_MP_LT) {
       if((err = mp_mod(key->k, order, key->k)) != CRYPT_OK)                                    { goto errkey; }
   }
   /* make the public key */
   if ((err = ltc_mp.ecc_ptmul(key->k, base, &key->pubkey, prime, 1)) != CRYPT_OK)              { goto errkey; }
   key->type = PK_PRIVATE;

   /* free up ram */
   err = CRYPT_OK;
   goto cleanup;
errkey:
   mp_clear_multi(key->pubkey.x, key->pubkey.y, key->pubkey.z, key->k, NULL);
cleanup:
   ltc_ecc_del_point(base);
   mp_clear_multi(prime, order, NULL);
ERR_BUF:
#ifdef LTC_CLEAN_STACK
   zeromem(buf, ECC_MAXSIZE);
#endif
   XFREE(buf);
   return err;
}

#endif
/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */

