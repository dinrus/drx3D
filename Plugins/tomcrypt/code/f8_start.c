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
   @file f8_start.c
   F8 implementation, start chain, Tom St Denis
*/


#ifdef LTC_F8_MODE

/**
   Initialize an F8 context
   @param cipher      The index of the cipher desired
   @param IV          The initialization vector
   @param key         The secret key
   @param keylen      The length of the secret key (octets)
   @param salt_key    The salting key for the IV
   @param skeylen     The length of the salting key (octets)
   @param num_rounds  Number of rounds in the cipher desired (0 for default)
   @param f8          The F8 state to initialize
   @return CRYPT_OK if successful
*/
i32 f8_start(                i32  cipher, u8k *IV,
             u8k *key,                    i32  keylen,
             u8k *salt_key,               i32  skeylen,
                             i32  num_rounds,   symmetric_F8  *f8)
{
   i32           x, err;
   u8 tkey[MAXBLOCKSIZE];

   LTC_ARGCHK(IV       != NULL);
   LTC_ARGCHK(key      != NULL);
   LTC_ARGCHK(salt_key != NULL);
   LTC_ARGCHK(f8       != NULL);

   if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
      return err;
   }

#ifdef LTC_FAST
   if (cipher_descriptor[cipher].block_length % sizeof(LTC_FAST_TYPE)) {
      return CRYPT_INVALID_ARG;
   }
#endif

   /* copy details */
   f8->blockcnt = 0;
   f8->cipher   = cipher;
   f8->blocklen = cipher_descriptor[cipher].block_length;
   f8->padlen   = f8->blocklen;

   /* now get key ^ salt_key [extend salt_ket with 0x55 as required to match length] */
   zeromem(tkey, sizeof(tkey));
   for (x = 0; x < keylen && x < (i32)sizeof(tkey); x++) {
       tkey[x] = key[x];
   }
   for (x = 0; x < skeylen && x < (i32)sizeof(tkey); x++) {
       tkey[x] ^= salt_key[x];
   }
   for (; x < keylen && x < (i32)sizeof(tkey); x++) {
       tkey[x] ^= 0x55;
   }

   /* now encrypt with tkey[0..keylen-1] the IV and use that as the IV */
   if ((err = cipher_descriptor[cipher].setup(tkey, keylen, num_rounds, &f8->key)) != CRYPT_OK) {
      return err;
   }

   /* encrypt IV */
   if ((err = cipher_descriptor[f8->cipher].ecb_encrypt(IV, f8->MIV, &f8->key)) != CRYPT_OK) {
      cipher_descriptor[f8->cipher].done(&f8->key);
      return err;
   }
   zeromem(tkey, sizeof(tkey));
   zeromem(f8->IV, sizeof(f8->IV));

   /* terminate this cipher */
   cipher_descriptor[f8->cipher].done(&f8->key);

   /* init the cipher */
   return cipher_descriptor[cipher].setup(key, keylen, num_rounds, &f8->key);
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
