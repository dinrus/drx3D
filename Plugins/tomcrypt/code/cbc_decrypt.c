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
   @file cbc_decrypt.c
   CBC implementation, encrypt block, Tom St Denis
*/


#ifdef LTC_CBC_MODE

/**
  CBC decrypt
  @param ct     Ciphertext
  @param pt     [out] Plaintext
  @param len    The number of bytes to process (must be multiple of block length)
  @param cbc    CBC state
  @return CRYPT_OK if successful
*/
i32 cbc_decrypt(u8k *ct, u8 *pt, u64 len, symmetric_CBC *cbc)
{
   i32 x, err;
   u8 tmp[16];
#ifdef LTC_FAST
   LTC_FAST_TYPE tmpy;
#else
   u8 tmpy;
#endif

   LTC_ARGCHK(pt  != NULL);
   LTC_ARGCHK(ct  != NULL);
   LTC_ARGCHK(cbc != NULL);

   if ((err = cipher_is_valid(cbc->cipher)) != CRYPT_OK) {
       return err;
   }

   /* is blocklen valid? */
   if (cbc->blocklen < 1 || cbc->blocklen > (i32)sizeof(cbc->IV) || cbc->blocklen > (i32)sizeof(tmp)) {
      return CRYPT_INVALID_ARG;
   }

   if (len % cbc->blocklen) {
      return CRYPT_INVALID_ARG;
   }
#ifdef LTC_FAST
   if (cbc->blocklen % sizeof(LTC_FAST_TYPE)) {
      return CRYPT_INVALID_ARG;
   }
#endif

   if (cipher_descriptor[cbc->cipher].accel_cbc_decrypt != NULL) {
      return cipher_descriptor[cbc->cipher].accel_cbc_decrypt(ct, pt, len / cbc->blocklen, cbc->IV, &cbc->key);
   } else {
      while (len) {
         /* decrypt */
         if ((err = cipher_descriptor[cbc->cipher].ecb_decrypt(ct, tmp, &cbc->key)) != CRYPT_OK) {
            return err;
         }

         /* xor IV against plaintext */
         #if defined(LTC_FAST)
         for (x = 0; x < cbc->blocklen; x += sizeof(LTC_FAST_TYPE)) {
            tmpy = *(LTC_FAST_TYPE_PTR_CAST((u8*)cbc->IV + x)) ^ *(LTC_FAST_TYPE_PTR_CAST((u8*)tmp + x));
            *(LTC_FAST_TYPE_PTR_CAST((u8*)cbc->IV + x)) = *(LTC_FAST_TYPE_PTR_CAST((u8*)ct + x));
            *(LTC_FAST_TYPE_PTR_CAST((u8*)pt + x)) = tmpy;
         }
    #else
         for (x = 0; x < cbc->blocklen; x++) {
            tmpy       = tmp[x] ^ cbc->IV[x];
            cbc->IV[x] = ct[x];
            pt[x]      = tmpy;
         }
    #endif

         ct  += cbc->blocklen;
         pt  += cbc->blocklen;
         len -= cbc->blocklen;
      }
   }
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
