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
  Source donated by Elliptic Semiconductor Inc (www.ellipticsemi.com) to the LibTom Projects
*/

#ifdef LTC_XTS_MODE

/** multiply by x
  @param I      The value to multiply by x (LFSR shift)
*/
void xts_mult_x(u8 *I)
{
   i32 x;
   u8 t, tt;

   for (x = t = 0; x < 16; x++) {
      tt = I[x] >> 7;
      I[x] = ((I[x] << 1) | t) & 0xFF;
      t = tt;
   }
   if (tt) {
      I[0] ^= 0x87;
   }
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
