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

static i32 _tweak_uncrypt(u8k *C, u8 *P, u8 *T, symmetric_xts *xts)
{
   u64 x;
   i32 err;

   /* tweak encrypt block i */
#ifdef LTC_FAST
   for (x = 0; x < 16; x += sizeof(LTC_FAST_TYPE)) {
      *(LTC_FAST_TYPE_PTR_CAST(&P[x])) = *(LTC_FAST_TYPE_PTR_CAST(&C[x])) ^ *(LTC_FAST_TYPE_PTR_CAST(&T[x]));
   }
#else
   for (x = 0; x < 16; x++) {
      P[x] = C[x] ^ T[x];
   }
#endif

   err = cipher_descriptor[xts->cipher].ecb_decrypt(P, P, &xts->key1);

#ifdef LTC_FAST
   for (x = 0; x < 16; x += sizeof(LTC_FAST_TYPE)) {
      *(LTC_FAST_TYPE_PTR_CAST(&P[x])) ^= *(LTC_FAST_TYPE_PTR_CAST(&T[x]));
   }
#else
   for (x = 0; x < 16; x++) {
      P[x] = P[x] ^ T[x];
   }
#endif

   /* LFSR the tweak */
   xts_mult_x(T);

   return err;
}

/** XTS Decryption
 @param ct     [in] Ciphertext
 @param ptlen  Length of plaintext (and ciphertext)
 @param pt     [out]  Plaintext
 @param tweak  [in] The 128--bit encryption tweak (e.g. sector number)
 @param xts    The XTS structure
 Returns CRYPT_OK upon success
 */
i32 xts_decrypt(u8k *ct, u64 ptlen, u8 *pt, u8 *tweak,
                symmetric_xts *xts)
{
   u8 PP[16], CC[16], T[16];
   u64 i, m, mo, lim;
   i32 err;

   /* check inputs */
   LTC_ARGCHK(pt != NULL);
   LTC_ARGCHK(ct != NULL);
   LTC_ARGCHK(tweak != NULL);
   LTC_ARGCHK(xts != NULL);

   /* check if valid */
   if ((err = cipher_is_valid(xts->cipher)) != CRYPT_OK) {
      return err;
   }

   /* get number of blocks */
   m = ptlen >> 4;
   mo = ptlen & 15;

   /* must have at least one full block */
   if (m == 0) {
      return CRYPT_INVALID_ARG;
   }

   if (mo == 0) {
      lim = m;
   } else {
      lim = m - 1;
   }

   if (cipher_descriptor[xts->cipher].accel_xts_decrypt && lim > 0) {

      /* use accelerated decryption for whole blocks */
      if ((err = cipher_descriptor[xts->cipher].accel_xts_decrypt(ct, pt, lim, tweak, &xts->key1, &xts->key2)) !=
                 CRYPT_OK) {
         return err;
      }
      ct += lim * 16;
      pt += lim * 16;

      /* tweak is encrypted on output */
      XMEMCPY(T, tweak, sizeof(T));
   } else {
      /* encrypt the tweak */
      if ((err = cipher_descriptor[xts->cipher].ecb_encrypt(tweak, T, &xts->key2)) != CRYPT_OK) {
         return err;
      }

      for (i = 0; i < lim; i++) {
         if ((err = _tweak_uncrypt(ct, pt, T, xts)) != CRYPT_OK) {
            return err;
         }
         ct += 16;
         pt += 16;
      }
   }

   /* if ptlen not divide 16 then */
   if (mo > 0) {
      XMEMCPY(CC, T, 16);
      xts_mult_x(CC);

      /* PP = tweak decrypt block m-1 */
      if ((err = _tweak_uncrypt(ct, PP, CC, xts)) != CRYPT_OK) {
         return err;
      }

      /* Pm = first ptlen % 16 bytes of PP */
      for (i = 0; i < mo; i++) {
         CC[i] = ct[16 + i];
         pt[16 + i] = PP[i];
      }
      for (; i < 16; i++) {
         CC[i] = PP[i];
      }

      /* Pm-1 = Tweak uncrypt CC */
      if ((err = _tweak_uncrypt(CC, pt, T, xts)) != CRYPT_OK) {
         return err;
      }
   }

   /* Decrypt the tweak back */
   if ((err = cipher_descriptor[xts->cipher].ecb_decrypt(T, tweak, &xts->key2)) != CRYPT_OK) {
      return err;
   }

   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
