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
  @file der_encode_integer.c
  ASN.1 DER, encode an integer, Tom St Denis
*/


#ifdef LTC_DER

/* Exports a positive bignum as DER format (upto 2^32 bytes in size) */
/**
  Store a mp_int integer
  @param num      The first mp_int to encode
  @param out      [out] The destination for the DER encoded integers
  @param outlen   [in/out] The max size and resulting size of the DER encoded integers
  @return CRYPT_OK if successful
*/
i32 der_encode_integer(uk num, u8 *out, u64 *outlen)
{
   u64 tmplen, y;
   i32           err, leading_zero;

   LTC_ARGCHK(num    != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* find out how big this will be */
   if ((err = der_length_integer(num, &tmplen)) != CRYPT_OK) {
      return err;
   }

   if (*outlen < tmplen) {
      *outlen = tmplen;
      return CRYPT_BUFFER_OVERFLOW;
   }

   if (mp_cmp_d(num, 0) != LTC_MP_LT) {
      /* we only need a leading zero if the msb of the first byte is one */
      if ((mp_count_bits(num) & 7) == 0 || mp_iszero(num) == LTC_MP_YES) {
         leading_zero = 1;
      } else {
         leading_zero = 0;
      }

      /* get length of num in bytes (plus 1 since we force the msbyte to zero) */
      y = mp_unsigned_bin_size(num) + leading_zero;
   } else {
      leading_zero = 0;
      y            = mp_count_bits(num);
      y            = y + (8 - (y & 7));
      y            = y >> 3;
      if (((mp_cnt_lsb(num)+1)==mp_count_bits(num)) && ((mp_count_bits(num)&7)==0)) --y;
   }

   /* now store initial data */
   *out++ = 0x02;
   if (y < 128) {
      /* short form */
      *out++ = (u8)y;
   } else if (y < 256) {
      *out++ = 0x81;
      *out++ = (u8)y;
   } else if (y < 65536UL) {
      *out++ = 0x82;
      *out++ = (u8)((y>>8)&255);
      *out++ = (u8)y;
   } else if (y < 16777216UL) {
      *out++ = 0x83;
      *out++ = (u8)((y>>16)&255);
      *out++ = (u8)((y>>8)&255);
      *out++ = (u8)y;
   } else {
      return CRYPT_INVALID_ARG;
   }

   /* now store msbyte of zero if num is non-zero */
   if (leading_zero) {
      *out++ = 0x00;
   }

   /* if it's not zero store it as big endian */
   if (mp_cmp_d(num, 0) == LTC_MP_GT) {
      /* now store the mpint */
      if ((err = mp_to_unsigned_bin(num, out)) != CRYPT_OK) {
          return err;
      }
   } else if (mp_iszero(num) != LTC_MP_YES) {
      uk tmp;

      /* negative */
      if (mp_init(&tmp) != CRYPT_OK) {
         return CRYPT_MEM;
      }

      /* 2^roundup and subtract */
      y = mp_count_bits(num);
      y = y + (8 - (y & 7));
      if (((mp_cnt_lsb(num)+1)==mp_count_bits(num)) && ((mp_count_bits(num)&7)==0)) y -= 8;
      if (mp_2expt(tmp, y) != CRYPT_OK || mp_add(tmp, num, tmp) != CRYPT_OK) {
         mp_clear(tmp);
         return CRYPT_MEM;
      }
      if ((err = mp_to_unsigned_bin(tmp, out)) != CRYPT_OK) {
         mp_clear(tmp);
         return err;
      }
      mp_clear(tmp);
   }

   /* we good */
   *outlen = tmplen;
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
