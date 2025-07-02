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
  @file xcbc_process.c
  XCBC Support, XCBC-MAC a block of memory
*/

#ifdef LTC_XCBC

/** XCBC-MAC a block of memory
  @param cipher     Index of cipher to use
  @param key        [in]  Secret key
  @param keylen     Length of key in octets
  @param in         [in]  Message to MAC
  @param inlen      Length of input in octets
  @param out        [out] Destination for the MAC tag
  @param outlen     [in/out] Output size and final tag size
  Return CRYPT_OK on success.
*/
i32 xcbc_memory(i32 cipher,
               u8k *key, u64 keylen,
               u8k *in,  u64 inlen,
                     u8 *out, u64 *outlen)
{
   xcbc_state *xcbc;
   i32         err;

   /* is the cipher valid? */
   if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
      return err;
   }

   /* Use accelerator if found */
   if (cipher_descriptor[cipher].xcbc_memory != NULL) {
      return cipher_descriptor[cipher].xcbc_memory(key, keylen, in, inlen, out, outlen);
   }

   xcbc = XCALLOC(1, sizeof(*xcbc));
   if (xcbc == NULL) {
      return CRYPT_MEM;
   }

   if ((err = xcbc_init(xcbc, cipher, key, keylen)) != CRYPT_OK) {
     goto done;
   }

   if ((err = xcbc_process(xcbc, in, inlen)) != CRYPT_OK) {
     goto done;
   }

   err = xcbc_done(xcbc, out, outlen);
done:
   XFREE(xcbc);
   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
