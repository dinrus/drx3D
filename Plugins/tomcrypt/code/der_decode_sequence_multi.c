/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>
#include <stdarg.h>


/**
  @file der_decode_sequence_multi.c
  ASN.1 DER, decode a SEQUENCE, Tom St Denis
*/

#ifdef LTC_DER

/**
  Decode a SEQUENCE type using a VA list
  @param in    Input buffer
  @param inlen Length of input in octets
  @remark <...> is of the form <type, size, data> (i32, u64, uk )
  @return CRYPT_OK on success
*/
i32 der_decode_sequence_multi(u8k *in, u64 inlen, ...)
{
   i32           err;
   ltc_asn1_type type;
   u64 size, x;
   void          *data;
   va_list       args;
   ltc_asn1_list *list;

   LTC_ARGCHK(in    != NULL);

   /* get size of output that will be required */
   va_start(args, inlen);
   x = 0;
   for (;;) {
       type = (ltc_asn1_type)va_arg(args, i32);
       size = va_arg(args, u64);
       data = va_arg(args, uk );
       LTC_UNUSED_PARAM(size);
       LTC_UNUSED_PARAM(data);

       if (type == LTC_ASN1_EOL) {
          break;
       }

       switch (type) {
           case LTC_ASN1_BOOLEAN:
           case LTC_ASN1_INTEGER:
           case LTC_ASN1_SHORT_INTEGER:
           case LTC_ASN1_BIT_STRING:
           case LTC_ASN1_OCTET_STRING:
           case LTC_ASN1_NULL:
           case LTC_ASN1_OBJECT_IDENTIFIER:
           case LTC_ASN1_IA5_STRING:
           case LTC_ASN1_PRINTABLE_STRING:
           case LTC_ASN1_UTF8_STRING:
           case LTC_ASN1_UTCTIME:
           case LTC_ASN1_SET:
           case LTC_ASN1_SETOF:
           case LTC_ASN1_SEQUENCE:
           case LTC_ASN1_CHOICE:
           case LTC_ASN1_RAW_BIT_STRING:
           case LTC_ASN1_TELETEX_STRING:
           case LTC_ASN1_GENERALIZEDTIME:
                ++x;
                break;

           case LTC_ASN1_EOL:
           case LTC_ASN1_CONSTRUCTED:
           case LTC_ASN1_CONTEXT_SPECIFIC:
               va_end(args);
               return CRYPT_INVALID_ARG;
       }
   }
   va_end(args);

   /* allocate structure for x elements */
   if (x == 0) {
      return CRYPT_NOP;
   }

   list = XCALLOC(sizeof(*list), x);
   if (list == NULL) {
      return CRYPT_MEM;
   }

   /* fill in the structure */
   va_start(args, inlen);
   x = 0;
   for (;;) {
       type = (ltc_asn1_type)va_arg(args, i32);
       size = va_arg(args, u64);
       data = va_arg(args, uk );

       if (type == LTC_ASN1_EOL) {
          break;
       }

       switch (type) {
           case LTC_ASN1_BOOLEAN:
           case LTC_ASN1_INTEGER:
           case LTC_ASN1_SHORT_INTEGER:
           case LTC_ASN1_BIT_STRING:
           case LTC_ASN1_OCTET_STRING:
           case LTC_ASN1_NULL:
           case LTC_ASN1_OBJECT_IDENTIFIER:
           case LTC_ASN1_IA5_STRING:
           case LTC_ASN1_PRINTABLE_STRING:
           case LTC_ASN1_UTF8_STRING:
           case LTC_ASN1_UTCTIME:
           case LTC_ASN1_SEQUENCE:
           case LTC_ASN1_SET:
           case LTC_ASN1_SETOF:
           case LTC_ASN1_CHOICE:
           case LTC_ASN1_RAW_BIT_STRING:
           case LTC_ASN1_TELETEX_STRING:
           case LTC_ASN1_GENERALIZEDTIME:
                LTC_SET_ASN1(list, x++, type, data, size);
                break;
           /* coverity[dead_error_line] */
           case LTC_ASN1_EOL:
           case LTC_ASN1_CONSTRUCTED:
           case LTC_ASN1_CONTEXT_SPECIFIC:
                break;
       }
   }
   va_end(args);

   err = der_decode_sequence(in, inlen, list, x);
   XFREE(list);
   return err;
}

#endif


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
