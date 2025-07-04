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
  @file f9_process.c
  f9 Support, Process a block through F9-MAC
*/

#ifdef LTC_F9_MODE

/** f9-MAC a block of memory
  @param cipher     Index of cipher to use
  @param key        [in]  Secret key
  @param keylen     Length of key in octets
  @param in         [in]  Message to MAC
  @param inlen      Length of input in octets
  @param out        [out] Destination for the MAC tag
  @param outlen     [in/out] Output size and final tag size
  Return CRYPT_OK on success.
*/
i32 f9_memory(i32 cipher,
               u8k *key, u64 keylen,
               u8k *in,  u64 inlen,
                     u8 *out, u64 *outlen)
{
   f9_state *f9;
   i32         err;

   /* is the cipher valid? */
   if ((err = cipher_is_valid(cipher)) != CRYPT_OK) {
      return err;
   }

   /* Use accelerator if found */
   if (cipher_descriptor[cipher].f9_memory != NULL) {
      return cipher_descriptor[cipher].f9_memory(key, keylen, in, inlen, out, outlen);
   }

   f9 = XCALLOC(1, sizeof(*f9));
   if (f9 == NULL) {
      return CRYPT_MEM;
   }

   if ((err = f9_init(f9, cipher, key, keylen)) != CRYPT_OK) {
     goto done;
   }

   if ((err = f9_process(f9, in, inlen)) != CRYPT_OK) {
     goto done;
   }

   err = f9_done(f9, out, outlen);
done:
   XFREE(f9);
   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
