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
  @file der_decode_utctime.c
  ASN.1 DER, decode a  UTCTIME, Tom St Denis
*/

#ifdef LTC_DER

static i32 _char_to_int(u8 x)
{
   switch (x)  {
      case '0': return 0;
      case '1': return 1;
      case '2': return 2;
      case '3': return 3;
      case '4': return 4;
      case '5': return 5;
      case '6': return 6;
      case '7': return 7;
      case '8': return 8;
      case '9': return 9;
      default:  return 100;
   }
}

#define DECODE_V(y, max) \
   y  = _char_to_int(buf[x])*10 + _char_to_int(buf[x+1]); \
   if (y >= max) return CRYPT_INVALID_PACKET;           \
   x += 2;

/**
  Decodes a UTC time structure in DER format (reads all 6 valid encoding formats)
  @param in     Input buffer
  @param inlen  Length of input buffer in octets
  @param out    [out] Destination of UTC time structure
  @return CRYPT_OK   if successful
*/
i32 der_decode_utctime(u8k *in, u64 *inlen,
                             ltc_utctime   *out)
{
   u8 buf[32] = { 0 }; /* initialize as all zeroes */
   u64 x;
   i32           y;

   LTC_ARGCHK(in    != NULL);
   LTC_ARGCHK(inlen != NULL);
   LTC_ARGCHK(out   != NULL);

   /* check header */
   if (*inlen < 2UL || (in[1] >= sizeof(buf)) || ((in[1] + 2UL) > *inlen)) {
      return CRYPT_INVALID_PACKET;
   }

   /* decode the string */
   for (x = 0; x < in[1]; x++) {
       y = der_ia5_value_decode(in[x+2]);
       if (y == -1) {
          return CRYPT_INVALID_PACKET;
       }
       buf[x] = y;
   }
   *inlen = 2 + x;


   /* possible encodings are
YYMMDDhhmmZ
YYMMDDhhmm+hh'mm'
YYMMDDhhmm-hh'mm'
YYMMDDhhmmssZ
YYMMDDhhmmss+hh'mm'
YYMMDDhhmmss-hh'mm'

    So let's do a trivial decode upto [including] mm
   */

    x = 0;
    DECODE_V(out->YY, 100);
    DECODE_V(out->MM, 13);
    DECODE_V(out->DD, 32);
    DECODE_V(out->hh, 24);
    DECODE_V(out->mm, 60);

    /* clear timezone and seconds info */
    out->off_dir = out->off_hh = out->off_mm = out->ss = 0;

    /* now is it Z, +, - or 0-9 */
    if (buf[x] == 'Z') {
       return CRYPT_OK;
    } else if (buf[x] == '+' || buf[x] == '-') {
       out->off_dir = (buf[x++] == '+') ? 0 : 1;
       DECODE_V(out->off_hh, 24);
       DECODE_V(out->off_mm, 60);
       return CRYPT_OK;
    }

    /* decode seconds */
    DECODE_V(out->ss, 60);

    /* now is it Z, +, - */
    if (buf[x] == 'Z') {
       return CRYPT_OK;
    } else if (buf[x] == '+' || buf[x] == '-') {
       out->off_dir = (buf[x++] == '+') ? 0 : 1;
       DECODE_V(out->off_hh, 24);
       DECODE_V(out->off_mm, 60);
       return CRYPT_OK;
    } else {
       return CRYPT_INVALID_PACKET;
    }
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
