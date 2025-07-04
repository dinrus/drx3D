/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#ifdef LTC_CCM_MODE

/**
  Add nonce data to the CCM state
  @param ccm       The CCM state
  @param nonce     The nonce data to add
  @param noncelen  The length of the nonce
  @return CRYPT_OK on success
 */
i32 ccm_add_nonce(ccm_state *ccm,
                  u8k *nonce,     u64 noncelen)
{
   u64 x, y, len;
   i32           err;

   LTC_ARGCHK(ccm   != NULL);
   LTC_ARGCHK(nonce != NULL);

   /* increase L to match the nonce len */
   ccm->noncelen = (noncelen > 13) ? 13 : noncelen;
   if ((15 - ccm->noncelen) > ccm->L) {
      ccm->L = 15 - ccm->noncelen;
   }

   /* decrease noncelen to match L */
   if ((ccm->noncelen + ccm->L) > 15) {
      ccm->noncelen = 15 - ccm->L;
   }

   /* form B_0 == flags | Nonce N | l(m) */
   x = 0;
   ccm->PAD[x++] = (u8)(((ccm->aadlen > 0) ? (1<<6) : 0) |
                   (((ccm->taglen - 2)>>1)<<3)        |
                   (ccm->L-1));

   /* nonce */
   for (y = 0; y < (16 - (ccm->L + 1)); y++) {
      ccm->PAD[x++] = nonce[y];
   }

   /* store len */
   len = ccm->ptlen;

   /* shift len so the upper bytes of len are the contents of the length */
   for (y = ccm->L; y < 4; y++) {
      len <<= 8;
   }

   /* store l(m) (only store 32-bits) */
   for (y = 0; ccm->L > 4 && (ccm->L-y)>4; y++) {
      ccm->PAD[x++] = 0;
   }
   for (; y < ccm->L; y++) {
      ccm->PAD[x++] = (u8)((len >> 24) & 255);
      len <<= 8;
   }

   /* encrypt PAD */
   if ((err = cipher_descriptor[ccm->cipher].ecb_encrypt(ccm->PAD, ccm->PAD, &ccm->K)) != CRYPT_OK) {
      return err;
   }

   /* handle header */
   ccm->x = 0;
   if (ccm->aadlen > 0) {
      /* store length */
      if (ccm->aadlen < ((1UL<<16) - (1UL<<8))) {
         ccm->PAD[ccm->x++] ^= (ccm->aadlen>>8) & 255;
         ccm->PAD[ccm->x++] ^= ccm->aadlen & 255;
      } else {
         ccm->PAD[ccm->x++] ^= 0xFF;
         ccm->PAD[ccm->x++] ^= 0xFE;
         ccm->PAD[ccm->x++] ^= (ccm->aadlen>>24) & 255;
         ccm->PAD[ccm->x++] ^= (ccm->aadlen>>16) & 255;
         ccm->PAD[ccm->x++] ^= (ccm->aadlen>>8) & 255;
         ccm->PAD[ccm->x++] ^= ccm->aadlen & 255;
      }
   }

   /* setup the ctr counter */
   x = 0;

   /* flags */
   ccm->ctr[x++] = (u8)ccm->L-1;

   /* nonce */
   for (y = 0; y < (16 - (ccm->L+1)); ++y) {
      ccm->ctr[x++] = nonce[y];
   }
   /* offset */
   while (x < 16) {
      ccm->ctr[x++] = 0;
   }

   ccm->CTRlen = 16;
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
