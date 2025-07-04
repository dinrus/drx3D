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
  @file der_encode_utctime.c
  ASN.1 DER, encode a GeneralizedTime, Steffen Jaeckel
  Based on der_encode_utctime.c
*/

#ifdef LTC_DER

static tukk  const baseten = "0123456789";

#define STORE_V(y) do {\
    out[x++] = der_ia5_char_encode(baseten[(y/10) % 10]); \
    out[x++] = der_ia5_char_encode(baseten[y % 10]); \
} while(0)

#define STORE_V4(y) do {\
    out[x++] = der_ia5_char_encode(baseten[(y/1000) % 10]); \
    out[x++] = der_ia5_char_encode(baseten[(y/100) % 10]); \
    out[x++] = der_ia5_char_encode(baseten[(y/10) % 10]); \
    out[x++] = der_ia5_char_encode(baseten[y % 10]); \
} while(0)

/**
  Encodes a Generalized time structure in DER format
  @param gtime        The GeneralizedTime structure to encode
  @param out          The destination of the DER encoding of the GeneralizedTime structure
  @param outlen       [in/out] The length of the DER encoding
  @return CRYPT_OK if successful
*/
i32 der_encode_generalizedtime(ltc_generalizedtime *gtime,
                               u8       *out,   u64 *outlen)
{
    u64 x, tmplen;
    i32           err;

    LTC_ARGCHK(gtime != NULL);
    LTC_ARGCHK(out     != NULL);
    LTC_ARGCHK(outlen  != NULL);

    if ((err = der_length_generalizedtime(gtime, &tmplen)) != CRYPT_OK) {
       return err;
    }
    if (tmplen > *outlen) {
        *outlen = tmplen;
        return CRYPT_BUFFER_OVERFLOW;
    }

    /* store header */
    out[0] = 0x18;

    /* store values */
    x = 2;
    STORE_V4(gtime->YYYY);
    STORE_V(gtime->MM);
    STORE_V(gtime->DD);
    STORE_V(gtime->hh);
    STORE_V(gtime->mm);
    STORE_V(gtime->ss);

    if (gtime->fs) {
       u64 divisor;
       unsigned fs = gtime->fs;
       unsigned len = 0;
       out[x++] = der_ia5_char_encode('.');
       divisor = 1;
       do {
          fs /= 10;
          divisor *= 10;
          len++;
       } while(fs != 0);
       while (len-- > 1) {
          divisor /= 10;
          out[x++] = der_ia5_char_encode(baseten[(gtime->fs/divisor) % 10]);
       }
       out[x++] = der_ia5_char_encode(baseten[gtime->fs % 10]);
    }

    if (gtime->off_mm || gtime->off_hh) {
       out[x++] = der_ia5_char_encode(gtime->off_dir ? '-' : '+');
       STORE_V(gtime->off_hh);
       STORE_V(gtime->off_mm);
    } else {
       out[x++] = der_ia5_char_encode('Z');
    }

    /* store length */
    out[1] = (u8)(x - 2);

    /* all good let's return */
    *outlen = x;
    return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
