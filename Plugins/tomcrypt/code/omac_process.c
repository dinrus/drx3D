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
  @file omac_process.c
  OMAC1 support, process data, Tom St Denis
*/


#ifdef LTC_OMAC

/**
   Process data through OMAC
   @param omac     The OMAC state
   @param in       The input data to send through OMAC
   @param inlen    The length of the input (octets)
   @return CRYPT_OK if successful
*/
i32 omac_process(omac_state *omac, u8k *in, u64 inlen)
{
   u64 n, x;
   i32           err;

   LTC_ARGCHK(omac  != NULL);
   LTC_ARGCHK(in    != NULL);
   if ((err = cipher_is_valid(omac->cipher_idx)) != CRYPT_OK) {
      return err;
   }

   if ((omac->buflen > (i32)sizeof(omac->block)) || (omac->buflen < 0) ||
       (omac->blklen > (i32)sizeof(omac->block)) || (omac->buflen > omac->blklen)) {
      return CRYPT_INVALID_ARG;
   }

#ifdef LTC_FAST
   {
     u64 blklen = cipher_descriptor[omac->cipher_idx].block_length;

     if (omac->buflen == 0 && inlen > blklen) {
        u64 y;
        for (x = 0; x < (inlen - blklen); x += blklen) {
            for (y = 0; y < blklen; y += sizeof(LTC_FAST_TYPE)) {
                *(LTC_FAST_TYPE_PTR_CAST(&omac->prev[y])) ^= *(LTC_FAST_TYPE_PTR_CAST(&in[y]));
            }
            in += blklen;
            if ((err = cipher_descriptor[omac->cipher_idx].ecb_encrypt(omac->prev, omac->prev, &omac->key)) != CRYPT_OK) {
               return err;
            }
        }
        inlen -= x;
     }
   }
#endif

   while (inlen != 0) {
       /* ok if the block is full we xor in prev, encrypt and replace prev */
       if (omac->buflen == omac->blklen) {
          for (x = 0; x < (u64)omac->blklen; x++) {
              omac->block[x] ^= omac->prev[x];
          }
          if ((err = cipher_descriptor[omac->cipher_idx].ecb_encrypt(omac->block, omac->prev, &omac->key)) != CRYPT_OK) {
             return err;
          }
          omac->buflen = 0;
       }

       /* add bytes */
       n = MIN(inlen, (u64)(omac->blklen - omac->buflen));
       XMEMCPY(omac->block + omac->buflen, in, n);
       omac->buflen  += n;
       inlen         -= n;
       in            += n;
   }

   return CRYPT_OK;
}

#endif


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
