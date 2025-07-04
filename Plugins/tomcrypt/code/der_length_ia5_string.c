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
  @file der_length_ia5_string.c
  ASN.1 DER, get length of IA5 STRING, Tom St Denis
*/

#ifdef LTC_DER

static const struct {
   i32 code, value;
} ia5_table[] = {
{ '\0', 0 },
{ '\a', 7 },
{ '\b', 8 },
{ '\t', 9 },
{ '\n', 10 },
{ '\f', 12 },
{ '\r', 13 },
{ ' ', 32 },
{ '!', 33 },
{ '"', 34 },
{ '#', 35 },
{ '$', 36 },
{ '%', 37 },
{ '&', 38 },
{ '\'', 39 },
{ '(', 40 },
{ ')', 41 },
{ '*', 42 },
{ '+', 43 },
{ ',', 44 },
{ '-', 45 },
{ '.', 46 },
{ '/', 47 },
{ '0', 48 },
{ '1', 49 },
{ '2', 50 },
{ '3', 51 },
{ '4', 52 },
{ '5', 53 },
{ '6', 54 },
{ '7', 55 },
{ '8', 56 },
{ '9', 57 },
{ ':', 58 },
{ ';', 59 },
{ '<', 60 },
{ '=', 61 },
{ '>', 62 },
{ '?', 63 },
{ '@', 64 },
{ 'A', 65 },
{ 'B', 66 },
{ 'C', 67 },
{ 'D', 68 },
{ 'E', 69 },
{ 'F', 70 },
{ 'G', 71 },
{ 'H', 72 },
{ 'I', 73 },
{ 'J', 74 },
{ 'K', 75 },
{ 'L', 76 },
{ 'M', 77 },
{ 'N', 78 },
{ 'O', 79 },
{ 'P', 80 },
{ 'Q', 81 },
{ 'R', 82 },
{ 'S', 83 },
{ 'T', 84 },
{ 'U', 85 },
{ 'V', 86 },
{ 'W', 87 },
{ 'X', 88 },
{ 'Y', 89 },
{ 'Z', 90 },
{ '[', 91 },
{ '\\', 92 },
{ ']', 93 },
{ '^', 94 },
{ '_', 95 },
{ '`', 96 },
{ 'a', 97 },
{ 'b', 98 },
{ 'c', 99 },
{ 'd', 100 },
{ 'e', 101 },
{ 'f', 102 },
{ 'g', 103 },
{ 'h', 104 },
{ 'i', 105 },
{ 'j', 106 },
{ 'k', 107 },
{ 'l', 108 },
{ 'm', 109 },
{ 'n', 110 },
{ 'o', 111 },
{ 'p', 112 },
{ 'q', 113 },
{ 'r', 114 },
{ 's', 115 },
{ 't', 116 },
{ 'u', 117 },
{ 'v', 118 },
{ 'w', 119 },
{ 'x', 120 },
{ 'y', 121 },
{ 'z', 122 },
{ '{', 123 },
{ '|', 124 },
{ '}', 125 },
{ '~', 126 }
};

i32 der_ia5_char_encode(i32 c)
{
   i32 x;
   for (x = 0; x < (i32)(sizeof(ia5_table)/sizeof(ia5_table[0])); x++) {
       if (ia5_table[x].code == c) {
          return ia5_table[x].value;
       }
   }
   return -1;
}

i32 der_ia5_value_decode(i32 v)
{
   i32 x;
   for (x = 0; x < (i32)(sizeof(ia5_table)/sizeof(ia5_table[0])); x++) {
       if (ia5_table[x].value == v) {
          return ia5_table[x].code;
       }
   }
   return -1;
}

/**
  Gets length of DER encoding of IA5 STRING
  @param octets   The values you want to encode
  @param noctets  The number of octets in the string to encode
  @param outlen   [out] The length of the DER encoding for the given string
  @return CRYPT_OK if successful
*/
i32 der_length_ia5_string(u8k *octets, u64 noctets, u64 *outlen)
{
   u64 x;

   LTC_ARGCHK(outlen != NULL);
   LTC_ARGCHK(octets != NULL);

   /* scan string for validity */
   for (x = 0; x < noctets; x++) {
       if (der_ia5_char_encode(octets[x]) == -1) {
          return CRYPT_INVALID_ARG;
       }
   }

   if (noctets < 128) {
      /* 16 LL DD DD DD ... */
      *outlen = 2 + noctets;
   } else if (noctets < 256) {
      /* 16 81 LL DD DD DD ... */
      *outlen = 3 + noctets;
   } else if (noctets < 65536UL) {
      /* 16 82 LL LL DD DD DD ... */
      *outlen = 4 + noctets;
   } else if (noctets < 16777216UL) {
      /* 16 83 LL LL LL DD DD DD ... */
      *outlen = 5 + noctets;
   } else {
      return CRYPT_INVALID_ARG;
   }

   return CRYPT_OK;
}

#endif


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
