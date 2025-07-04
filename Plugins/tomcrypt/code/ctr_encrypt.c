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
  @file ctr_encrypt.c
  CTR implementation, encrypt data, Tom St Denis
*/


#ifdef LTC_CTR_MODE

/**
  CTR encrypt software implementation
  @param pt     Plaintext
  @param ct     [out] Ciphertext
  @param len    Length of plaintext (octets)
  @param ctr    CTR state
  @return CRYPT_OK if successful
*/
static i32 _ctr_encrypt(u8k *pt, u8 *ct, u64 len, symmetric_CTR *ctr)
{
   i32 x, err;

   while (len) {
      /* is the pad empty? */
      if (ctr->padlen == ctr->blocklen) {
         /* increment counter */
         if (ctr->mode == CTR_COUNTER_LITTLE_ENDIAN) {
            /* little-endian */
            for (x = 0; x < ctr->ctrlen; x++) {
               ctr->ctr[x] = (ctr->ctr[x] + (u8)1) & (u8)255;
               if (ctr->ctr[x] != (u8)0) {
                  break;
               }
            }
         } else {
            /* big-endian */
            for (x = ctr->blocklen-1; x >= ctr->ctrlen; x--) {
               ctr->ctr[x] = (ctr->ctr[x] + (u8)1) & (u8)255;
               if (ctr->ctr[x] != (u8)0) {
                  break;
               }
            }
         }

         /* encrypt it */
         if ((err = cipher_descriptor[ctr->cipher].ecb_encrypt(ctr->ctr, ctr->pad, &ctr->key)) != CRYPT_OK) {
            return err;
         }
         ctr->padlen = 0;
      }
#ifdef LTC_FAST
      if ((ctr->padlen == 0) && (len >= (u64)ctr->blocklen)) {
         for (x = 0; x < ctr->blocklen; x += sizeof(LTC_FAST_TYPE)) {
            *(LTC_FAST_TYPE_PTR_CAST((u8*)ct + x)) = *(LTC_FAST_TYPE_PTR_CAST((u8*)pt + x)) ^
                                                           *(LTC_FAST_TYPE_PTR_CAST((u8*)ctr->pad + x));
         }
       pt         += ctr->blocklen;
       ct         += ctr->blocklen;
       len        -= ctr->blocklen;
       ctr->padlen = ctr->blocklen;
       continue;
      }
#endif
      *ct++ = *pt++ ^ ctr->pad[ctr->padlen++];
      --len;
   }
   return CRYPT_OK;
}

/**
  CTR encrypt
  @param pt     Plaintext
  @param ct     [out] Ciphertext
  @param len    Length of plaintext (octets)
  @param ctr    CTR state
  @return CRYPT_OK if successful
*/
i32 ctr_encrypt(u8k *pt, u8 *ct, u64 len, symmetric_CTR *ctr)
{
   i32 err, fr;

   LTC_ARGCHK(pt != NULL);
   LTC_ARGCHK(ct != NULL);
   LTC_ARGCHK(ctr != NULL);

   if ((err = cipher_is_valid(ctr->cipher)) != CRYPT_OK) {
       return err;
   }

   /* is blocklen/padlen valid? */
   if ((ctr->blocklen < 1) || (ctr->blocklen > (i32)sizeof(ctr->ctr)) ||
       (ctr->padlen   < 0) || (ctr->padlen   > (i32)sizeof(ctr->pad))) {
      return CRYPT_INVALID_ARG;
   }

#ifdef LTC_FAST
   if (ctr->blocklen % sizeof(LTC_FAST_TYPE)) {
      return CRYPT_INVALID_ARG;
   }
#endif

   /* handle acceleration only if pad is empty, accelerator is present and length is >= a block size */
   if ((cipher_descriptor[ctr->cipher].accel_ctr_encrypt != NULL) && (len >= (u64)ctr->blocklen)) {
     if (ctr->padlen < ctr->blocklen) {
       fr = ctr->blocklen - ctr->padlen;
       if ((err = _ctr_encrypt(pt, ct, fr, ctr)) != CRYPT_OK) {
          return err;
       }
       pt += fr;
       ct += fr;
       len -= fr;
     }

     if (len >= (u64)ctr->blocklen) {
       if ((err = cipher_descriptor[ctr->cipher].accel_ctr_encrypt(pt, ct, len/ctr->blocklen, ctr->ctr, ctr->mode, &ctr->key)) != CRYPT_OK) {
          return err;
       }
       pt += (len / ctr->blocklen) * ctr->blocklen;
       ct += (len / ctr->blocklen) * ctr->blocklen;
       len %= ctr->blocklen;
     }
   }

   return _ctr_encrypt(pt, ct, len, ctr);
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
