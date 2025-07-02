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
  @file base64_encode.c
  Compliant base64 encoder donated by Wayne Scott (wscott@bitmover.com)
  base64 URL Safe variant (RFC 4648 section 5) by Karel Miko
*/


#if defined(LTC_BASE64) || defined (LTC_BASE64_URL)

#if defined(LTC_BASE64)
static tukk  const codes_base64 =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#endif /* LTC_BASE64 */

#if defined(LTC_BASE64_URL)
static tukk  const codes_base64url =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
#endif /* LTC_BASE64_URL */

static i32 _base64_encode_internal(u8k *in,  u64 inlen,
                                 u8 *out, u64 *outlen,
                                 tukk codes, i32 pad)
{
   u64 i, len2, leven;
   u8 *p;

   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   /* valid output size ? */
   len2 = 4 * ((inlen + 2) / 3);
   if (*outlen < len2 + 1) {
      *outlen = len2 + 1;
      return CRYPT_BUFFER_OVERFLOW;
   }
   p = out;
   leven = 3*(inlen / 3);
   for (i = 0; i < leven; i += 3) {
       *p++ = codes[(in[0] >> 2) & 0x3F];
       *p++ = codes[(((in[0] & 3) << 4) + (in[1] >> 4)) & 0x3F];
       *p++ = codes[(((in[1] & 0xf) << 2) + (in[2] >> 6)) & 0x3F];
       *p++ = codes[in[2] & 0x3F];
       in += 3;
   }
   /* Pad it if necessary...  */
   if (i < inlen) {
       unsigned a = in[0];
       unsigned b = (i+1 < inlen) ? in[1] : 0;

       *p++ = codes[(a >> 2) & 0x3F];
       *p++ = codes[(((a & 3) << 4) + (b >> 4)) & 0x3F];
       if (pad) {
         *p++ = (i+1 < inlen) ? codes[(((b & 0xf) << 2)) & 0x3F] : '=';
         *p++ = '=';
       }
       else {
         if (i+1 < inlen) *p++ = codes[(((b & 0xf) << 2)) & 0x3F];
       }
   }

   /* append a NULL byte */
   *p = '\0';

   /* return ok */
   *outlen = (u64)(p - out);
   return CRYPT_OK;
}

#if defined(LTC_BASE64)
/**
   base64 Encode a buffer (NUL terminated)
   @param in      The input buffer to encode
   @param inlen   The length of the input buffer
   @param out     [out] The destination of the base64 encoded data
   @param outlen  [in/out] The max size and resulting size
   @return CRYPT_OK if successful
*/
i32 base64_encode(u8k *in,  u64 inlen,
                        u8 *out, u64 *outlen)
{
    return _base64_encode_internal(in, inlen, out, outlen, codes_base64, 1);
}
#endif /* LTC_BASE64 */


#if defined(LTC_BASE64_URL)
/**
   base64 (URL Safe, RFC 4648 section 5) Encode a buffer (NUL terminated)
   @param in      The input buffer to encode
   @param inlen   The length of the input buffer
   @param out     [out] The destination of the base64 encoded data
   @param outlen  [in/out] The max size and resulting size
   @return CRYPT_OK if successful
*/
i32 base64url_encode(u8k *in,  u64 inlen,
                           u8 *out, u64 *outlen)
{
    return _base64_encode_internal(in, inlen, out, outlen, codes_base64url, 0);
}

i32 base64url_strict_encode(u8k *in,  u64 inlen,
                           u8 *out, u64 *outlen)
{
    return _base64_encode_internal(in, inlen, out, outlen, codes_base64url, 1);
}
#endif /* LTC_BASE64_URL */

#endif


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
