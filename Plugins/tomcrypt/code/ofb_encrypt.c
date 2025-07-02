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
  @file ofb_encrypt.c
  OFB implementation, encrypt data, Tom St Denis
*/

#ifdef LTC_OFB_MODE

/**
  OFB encrypt
  @param pt     Plaintext
  @param ct     [out] Ciphertext
  @param len    Length of plaintext (octets)
  @param ofb    OFB state
  @return CRYPT_OK if successful
*/
i32 ofb_encrypt(u8k *pt, u8 *ct, u64 len, symmetric_OFB *ofb)
{
   i32 err;
   LTC_ARGCHK(pt != NULL);
   LTC_ARGCHK(ct != NULL);
   LTC_ARGCHK(ofb != NULL);
   if ((err = cipher_is_valid(ofb->cipher)) != CRYPT_OK) {
       return err;
   }

   /* is blocklen/padlen valid? */
   if (ofb->blocklen < 0 || ofb->blocklen > (i32)sizeof(ofb->IV) ||
       ofb->padlen   < 0 || ofb->padlen   > (i32)sizeof(ofb->IV)) {
      return CRYPT_INVALID_ARG;
   }

   while (len-- > 0) {
       if (ofb->padlen == ofb->blocklen) {
          if ((err = cipher_descriptor[ofb->cipher].ecb_encrypt(ofb->IV, ofb->IV, &ofb->key)) != CRYPT_OK) {
             return err;
          }
          ofb->padlen = 0;
       }
       *ct++ = *pt++ ^ ofb->IV[(ofb->padlen)++];
   }
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
