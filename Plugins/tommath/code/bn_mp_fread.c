#include "tommath_private.h"
#ifdef BN_MP_FREAD_C
/* LibTomMath, multiple-precision integer library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

#ifndef MP_NO_FILE
/* read a bigint from a file stream in ASCII */
mp_err mp_fread(mp_int *a, i32 radix, FILE *stream)
{
   mp_err err;
   mp_sign neg;

   /* if first digit is - then set negative */
   i32 ch = fgetc(stream);
   if (ch == (i32)'-') {
      neg = MP_NEG;
      ch = fgetc(stream);
   } else {
      neg = MP_ZPOS;
   }

   /* no digits, return error */
   if (ch == EOF) {
      return MP_ERR;
   }

   /* clear a */
   mp_zero(a);

   do {
      i32 y;
      unsigned pos = (unsigned)(ch - (i32)'(');
      if (mp_s_rmap_reverse_sz < pos) {
         break;
      }

      y = (i32)mp_s_rmap_reverse[pos];

      if ((y == 0xff) || (y >= radix)) {
         break;
      }

      /* shift up and add */
      if ((err = mp_mul_d(a, (mp_digit)radix, a)) != MP_OKAY) {
         return err;
      }
      if ((err = mp_add_d(a, (mp_digit)y, a)) != MP_OKAY) {
         return err;
      }
   } while ((ch = fgetc(stream)) != EOF);

   if (a->used != 0) {
      a->sign = neg;
   }

   return MP_OKAY;
}
#endif

#endif
