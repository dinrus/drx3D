/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/**
   @file gcm_mult_h.c
   GCM implementation, do the GF mult, by Tom St Denis
*/
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

#if defined(LTC_GCM_MODE)
/**
  GCM multiply by H
  @param gcm   The GCM state which holds the H value
  @param I     The value to multiply H by
 */
void gcm_mult_h(gcm_state *gcm, u8 *I)
{
   u8 T[16];
#ifdef LTC_GCM_TABLES
   i32 x;
#ifdef LTC_GCM_TABLES_SSE2
   asm("movdqa (%0),%%xmm0"::"r"(&gcm->PC[0][I[0]][0]));
   for (x = 1; x < 16; x++) {
      asm("pxor (%0),%%xmm0"::"r"(&gcm->PC[x][I[x]][0]));
   }
   asm("movdqa %%xmm0,(%0)"::"r"(&T));
#else
   i32 y;
   XMEMCPY(T, &gcm->PC[0][I[0]][0], 16);
   for (x = 1; x < 16; x++) {
#ifdef LTC_FAST
       for (y = 0; y < 16; y += sizeof(LTC_FAST_TYPE)) {
           *(LTC_FAST_TYPE_PTR_CAST(T + y)) ^= *(LTC_FAST_TYPE_PTR_CAST(&gcm->PC[x][I[x]][y]));
       }
#else
       for (y = 0; y < 16; y++) {
           T[y] ^= gcm->PC[x][I[x]][y];
       }
#endif /* LTC_FAST */
   }
#endif /* LTC_GCM_TABLES_SSE2 */
#else
   gcm_gf_mult(gcm->H, I, T);
#endif
   XMEMCPY(I, T, 16);
}
#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
