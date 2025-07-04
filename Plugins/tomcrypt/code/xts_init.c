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
  Source donated by Elliptic Semiconductor Inc (www.ellipticsemi.com) to the LibTom Projects
*/

#ifdef LTC_XTS_MODE

/** Start XTS mode
   @param cipher      The index of the cipher to use
   @param key1        The encrypt key
   @param key2        The tweak encrypt key
   @param keylen      The length of the keys (each) in octets
   @param num_rounds  The number of rounds for the cipher (0 == default)
   @param xts         [out] XTS structure
   Returns CRYPT_OK upon success.
*/
i32 xts_start(i32 cipher, u8k *key1, u8k *key2, u64 keylen, i32 num_rounds,
              symmetric_xts *xts)
{
   i32 err;

   /* check inputs */
   LTC_ARGCHK(key1 != NULL);
   LTC_ARGCHK(key2 != NULL);
   LTC_ARGCHK(xts != NULL);

   /* check if valid */
   if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
      return err;
   }

   if (cipher_descriptor[cipher].block_length != 16) {
      return CRYPT_INVALID_ARG;
   }

   /* schedule the two ciphers */
   if ((err = cipher_descriptor[cipher].setup(key1, keylen, num_rounds, &xts->key1)) != CRYPT_OK) {
      return err;
   }
   if ((err = cipher_descriptor[cipher].setup(key2, keylen, num_rounds, &xts->key2)) != CRYPT_OK) {
      return err;
   }
   xts->cipher = cipher;

   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
