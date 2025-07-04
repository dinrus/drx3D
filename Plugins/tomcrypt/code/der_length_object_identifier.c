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
  @file der_length_object_identifier.c
  ASN.1 DER, get length of Object Identifier, Tom St Denis
*/

#ifdef LTC_DER

u64 der_object_identifier_bits(u64 x)
{
   u64 c;
   x &= 0xFFFFFFFF;
   c  = 0;
   while (x) {
     ++c;
     x >>= 1;
   }
   return c;
}


/**
  Gets length of DER encoding of Object Identifier
  @param nwords   The number of OID words
  @param words    The actual OID words to get the size of
  @param outlen   [out] The length of the DER encoding for the given string
  @return CRYPT_OK if successful
*/
i32 der_length_object_identifier(u64 *words, u64 nwords, u64 *outlen)
{
   u64 y, z, t, wordbuf;

   LTC_ARGCHK(words  != NULL);
   LTC_ARGCHK(outlen != NULL);


   /* must be >= 2 words */
   if (nwords < 2) {
      return CRYPT_INVALID_ARG;
   }

   /* word1 = 0,1,2,3 and word2 0..39 */
   if (words[0] > 3 || (words[0] < 2 && words[1] > 39)) {
      return CRYPT_INVALID_ARG;
   }

   /* leading word is the first two */
   z = 0;
   wordbuf = words[0] * 40 + words[1];
   for (y = 1; y < nwords; y++) {
       t = der_object_identifier_bits(wordbuf);
       z += t/7 + ((t%7) ? 1 : 0) + (wordbuf == 0 ? 1 : 0);
       if (y < nwords - 1) {
          /* grab next word */
          wordbuf = words[y+1];
       }
   }

   /* now depending on the length our length encoding changes */
   if (z < 128) {
      z += 2;
   } else if (z < 256) {
      z += 3;
   } else if (z < 65536UL) {
      z += 4;
   } else {
      return CRYPT_INVALID_ARG;
   }

   *outlen = z;
   return CRYPT_OK;
}

#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
