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
  @file der_length_sequence.c
  ASN.1 DER, length a SEQUENCE, Tom St Denis
*/

#ifdef LTC_DER

/**
   Get the length of a DER sequence
   @param list   The sequences of items in the SEQUENCE
   @param inlen  The number of items
   @param outlen [out] The length required in octets to store it
   @return CRYPT_OK on success
*/
i32 der_length_sequence(ltc_asn1_list *list, u64 inlen,
                        u64 *outlen)
{
   return der_length_sequence_ex(list, inlen, outlen, NULL);
}

i32 der_length_sequence_ex(ltc_asn1_list *list, u64 inlen,
                           u64 *outlen, u64 *payloadlen)
{
   i32           err;
   ltc_asn1_type type;
   u64 size, x, y, i, z;
   void          *data;

   LTC_ARGCHK(list    != NULL);
   LTC_ARGCHK(outlen  != NULL);

   /* get size of output that will be required */
   y = 0;
   for (i = 0; i < inlen; i++) {
       type = list[i].type;
       size = list[i].size;
       data = list[i].data;

       if (type == LTC_ASN1_EOL) {
          break;
       }

       switch (type) {
           case LTC_ASN1_BOOLEAN:
              if ((err = der_length_boolean(&x)) != CRYPT_OK) {
                 goto LBL_ERR;
              }
              y += x;
              break;

           case LTC_ASN1_INTEGER:
               if ((err = der_length_integer(data, &x)) != CRYPT_OK) {
                  goto LBL_ERR;
               }
               y += x;
               break;

           case LTC_ASN1_SHORT_INTEGER:
               if ((err = der_length_short_integer(*((u64 *)data), &x)) != CRYPT_OK) {
                  goto LBL_ERR;
               }
               y += x;
               break;

           case LTC_ASN1_BIT_STRING:
           case LTC_ASN1_RAW_BIT_STRING:
               if ((err = der_length_bit_string(size, &x)) != CRYPT_OK) {
                  goto LBL_ERR;
               }
               y += x;
               break;

           case LTC_ASN1_OCTET_STRING:
               if ((err = der_length_octet_string(size, &x)) != CRYPT_OK) {
                  goto LBL_ERR;
               }
               y += x;
               break;

           case LTC_ASN1_NULL:
               y += 2;
               break;

           case LTC_ASN1_OBJECT_IDENTIFIER:
               if ((err = der_length_object_identifier(data, size, &x)) != CRYPT_OK) {
                  goto LBL_ERR;
               }
               y += x;
               break;

           case LTC_ASN1_IA5_STRING:
               if ((err = der_length_ia5_string(data, size, &x)) != CRYPT_OK) {
                  goto LBL_ERR;
               }
               y += x;
               break;

           case LTC_ASN1_TELETEX_STRING:
               if ((err = der_length_teletex_string(data, size, &x)) != CRYPT_OK) {
                  goto LBL_ERR;
               }
               y += x;
               break;

           case LTC_ASN1_PRINTABLE_STRING:
               if ((err = der_length_printable_string(data, size, &x)) != CRYPT_OK) {
                  goto LBL_ERR;
               }
               y += x;
               break;

           case LTC_ASN1_UTCTIME:
               if ((err = der_length_utctime(data, &x)) != CRYPT_OK) {
                  goto LBL_ERR;
               }
               y += x;
               break;

           case LTC_ASN1_GENERALIZEDTIME:
               if ((err = der_length_generalizedtime(data, &x)) != CRYPT_OK) {
                  goto LBL_ERR;
               }
               y += x;
               break;

           case LTC_ASN1_UTF8_STRING:
               if ((err = der_length_utf8_string(data, size, &x)) != CRYPT_OK) {
                  goto LBL_ERR;
               }
               y += x;
               break;

           case LTC_ASN1_SET:
           case LTC_ASN1_SETOF:
           case LTC_ASN1_SEQUENCE:
               if ((err = der_length_sequence(data, size, &x)) != CRYPT_OK) {
                  goto LBL_ERR;
               }
               y += x;
               break;


           case LTC_ASN1_CHOICE:
           case LTC_ASN1_CONSTRUCTED:
           case LTC_ASN1_CONTEXT_SPECIFIC:
           case LTC_ASN1_EOL:
               err = CRYPT_INVALID_ARG;
               goto LBL_ERR;
       }
   }

   /* calc header size */
   z = y;
   if (y < 128) {
      y += 2;
   } else if (y < 256) {
      /* 0x30 0x81 LL */
      y += 3;
   } else if (y < 65536UL) {
      /* 0x30 0x82 LL LL */
      y += 4;
   } else if (y < 16777216UL) {
      /* 0x30 0x83 LL LL LL */
      y += 5;
   } else {
      err = CRYPT_INVALID_ARG;
      goto LBL_ERR;
   }

   /* store size */
   if (payloadlen) *payloadlen = z;
   *outlen = y;
   err     = CRYPT_OK;

LBL_ERR:
   return err;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
