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
  @file cfb_decrypt.c
  CFB implementation, decrypt data, Tom St Denis
*/

#ifdef LTC_CFB_MODE

/**
   CFB decrypt
   @param ct      Ciphertext
   @param pt      [out] Plaintext
   @param len     Length of ciphertext (octets)
   @param cfb     CFB state
   @return CRYPT_OK if successful
*/
i32 cfb_decrypt(u8k *ct, u8 *pt, u64 len, symmetric_CFB *cfb)
{
   i32 err;

   LTC_ARGCHK(pt != NULL);
   LTC_ARGCHK(ct != NULL);
   LTC_ARGCHK(cfb != NULL);

   if ((err = cipher_is_valid(cfb->cipher)) != CRYPT_OK) {
       return err;
   }

   /* is blocklen/padlen valid? */
   if (cfb->blocklen < 0 || cfb->blocklen > (i32)sizeof(cfb->IV) ||
       cfb->padlen   < 0 || cfb->padlen   > (i32)sizeof(cfb->pad)) {
      return CRYPT_INVALID_ARG;
   }

   while (len-- > 0) {
       if (cfb->padlen == cfb->blocklen) {
          if ((err = cipher_descriptor[cfb->cipher].ecb_encrypt(cfb->pad, cfb->IV, &cfb->key)) != CRYPT_OK) {
             return err;
          }
          cfb->padlen = 0;
       }
       cfb->pad[cfb->padlen] = *ct;
       *pt = *ct ^ cfb->IV[cfb->padlen];
       ++pt;
       ++ct;
       ++(cfb->padlen);
   }
   return CRYPT_OK;
}

#endif


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
