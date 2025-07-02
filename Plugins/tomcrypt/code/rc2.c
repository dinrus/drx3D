/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
/**********************************************************************\
* To commemorate the 1996 RSA Data Security Conference, the following  *
* code is released into the public domain by its author.  Prost!       *
*                                                                      *
* This cipher uses 16-bit words and little-endian byte ordering.       *
* I wonder which processor it was optimized for?                       *
*                                                                      *
* Thanks to CodeView, SoftIce, and D86 for helping bring this code to  *
* the public.                                                          *
\**********************************************************************/
#include <drx3D/Plugins/tomcrypt/tomcrypt.h>

/**
  @file rc2.c
  Implementation of RC2 with fixed effective key length of 64bits
*/

#ifdef LTC_RC2

const struct ltc_cipher_descriptor rc2_desc = {
   "rc2",
   12, 8, 128, 8, 16,
   &rc2_setup,
   &rc2_ecb_encrypt,
   &rc2_ecb_decrypt,
   &rc2_test,
   &rc2_done,
   &rc2_keysize,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

/* 256-entry permutation table, probably derived somehow from pi */
static u8k permute[256] = {
        217,120,249,196, 25,221,181,237, 40,233,253,121, 74,160,216,157,
        198,126, 55,131, 43,118, 83,142, 98, 76,100,136, 68,139,251,162,
         23,154, 89,245,135,179, 79, 19, 97, 69,109,141,  9,129,125, 50,
        189,143, 64,235,134,183,123, 11,240,149, 33, 34, 92,107, 78,130,
         84,214,101,147,206, 96,178, 28,115, 86,192, 20,167,140,241,220,
         18,117,202, 31, 59,190,228,209, 66, 61,212, 48,163, 60,182, 38,
        111,191, 14,218, 70,105,  7, 87, 39,242, 29,155,188,148, 67,  3,
        248, 17,199,246,144,239, 62,231,  6,195,213, 47,200,102, 30,215,
          8,232,234,222,128, 82,238,247,132,170,114,172, 53, 77,106, 42,
        150, 26,210,113, 90, 21, 73,116, 75,159,208, 94,  4, 24,164,236,
        194,224, 65,110, 15, 81,203,204, 36,145,175, 80,161,244,112, 57,
        153,124, 58,133, 35,184,180,122,252,  2, 54, 91, 37, 85,151, 49,
         45, 93,250,152,227,138,146,174,  5,223, 41, 16,103,108,186,201,
        211,  0,230,207,225,158,168, 44, 99, 22,  1, 63, 88,226,137,169,
         13, 56, 52, 27,171, 51,255,176,187, 72, 12, 95,185,177,205, 46,
        197,243,219, 71,229,165,156,119, 10,166, 32,104,254,127,193,173
};

 /**
    Initialize the RC2 block cipher
    @param key The symmetric key you wish to pass
    @param keylen The key length in bytes
    @param bits The effective key length in bits
    @param num_rounds The number of rounds desired (0 for default)
    @param skey The key in as scheduled by this function.
    @return CRYPT_OK if successful
 */
i32 rc2_setup_ex(u8k *key, i32 keylen, i32 bits, i32 num_rounds, symmetric_key *skey)
{
   unsigned *xkey = skey->rc2.xkey;
   u8 tmp[128];
   unsigned T8, TM;
   i32 i;

   LTC_ARGCHK(key  != NULL);
   LTC_ARGCHK(skey != NULL);

   if (keylen == 0 || keylen > 128 || bits > 1024) {
      return CRYPT_INVALID_KEYSIZE;
   }
   if (bits == 0) {
      bits = 1024;
   }

   if (num_rounds != 0 && num_rounds != 16) {
      return CRYPT_INVALID_ROUNDS;
   }

   for (i = 0; i < keylen; i++) {
      tmp[i] = key[i] & 255;
   }

   /* Phase 1: Expand input key to 128 bytes */
   if (keylen < 128) {
      for (i = keylen; i < 128; i++) {
         tmp[i] = permute[(tmp[i - 1] + tmp[i - keylen]) & 255];
      }
   }

   /* Phase 2 - reduce effective key size to "bits" */
   T8   = (unsigned)(bits+7)>>3;
   TM   = (255 >> (unsigned)(7 & -bits));
   tmp[128 - T8] = permute[tmp[128 - T8] & TM];
   for (i = 127 - T8; i >= 0; i--) {
      tmp[i] = permute[tmp[i + 1] ^ tmp[i + T8]];
   }

   /* Phase 3 - copy to xkey in little-endian order */
   for (i = 0; i < 64; i++) {
      xkey[i] =  (unsigned)tmp[2*i] + ((unsigned)tmp[2*i+1] << 8);
   }

#ifdef LTC_CLEAN_STACK
   zeromem(tmp, sizeof(tmp));
#endif

   return CRYPT_OK;
}

/**
   Initialize the RC2 block cipher

     The effective key length is here always keylen * 8

   @param key The symmetric key you wish to pass
   @param keylen The key length in bytes
   @param num_rounds The number of rounds desired (0 for default)
   @param skey The key in as scheduled by this function.
   @return CRYPT_OK if successful
*/
i32 rc2_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey)
{
   return rc2_setup_ex(key, keylen, keylen * 8, num_rounds, skey);
}

/**********************************************************************\
* Encrypt an 8-byte block of plaintext using the given key.            *
\**********************************************************************/
/**
  Encrypts a block of text with RC2
  @param pt The input plaintext (8 bytes)
  @param ct The output ciphertext (8 bytes)
  @param skey The key as scheduled
  @return CRYPT_OK if successful
*/
#ifdef LTC_CLEAN_STACK
static i32 _rc2_ecb_encrypt( u8k *pt,
                            u8 *ct,
                            symmetric_key *skey)
#else
i32 rc2_ecb_encrypt( u8k *pt,
                            u8 *ct,
                            symmetric_key *skey)
#endif
{
    unsigned *xkey;
    unsigned x76, x54, x32, x10, i;

    LTC_ARGCHK(pt  != NULL);
    LTC_ARGCHK(ct != NULL);
    LTC_ARGCHK(skey   != NULL);

    xkey = skey->rc2.xkey;

    x76 = ((unsigned)pt[7] << 8) + (unsigned)pt[6];
    x54 = ((unsigned)pt[5] << 8) + (unsigned)pt[4];
    x32 = ((unsigned)pt[3] << 8) + (unsigned)pt[2];
    x10 = ((unsigned)pt[1] << 8) + (unsigned)pt[0];

    for (i = 0; i < 16; i++) {
        x10 = (x10 + (x32 & ~x76) + (x54 & x76) + xkey[4*i+0]) & 0xFFFF;
        x10 = ((x10 << 1) | (x10 >> 15));

        x32 = (x32 + (x54 & ~x10) + (x76 & x10) + xkey[4*i+1]) & 0xFFFF;
        x32 = ((x32 << 2) | (x32 >> 14));

        x54 = (x54 + (x76 & ~x32) + (x10 & x32) + xkey[4*i+2]) & 0xFFFF;
        x54 = ((x54 << 3) | (x54 >> 13));

        x76 = (x76 + (x10 & ~x54) + (x32 & x54) + xkey[4*i+3]) & 0xFFFF;
        x76 = ((x76 << 5) | (x76 >> 11));

        if (i == 4 || i == 10) {
            x10 = (x10 + xkey[x76 & 63]) & 0xFFFF;
            x32 = (x32 + xkey[x10 & 63]) & 0xFFFF;
            x54 = (x54 + xkey[x32 & 63]) & 0xFFFF;
            x76 = (x76 + xkey[x54 & 63]) & 0xFFFF;
        }
    }

    ct[0] = (u8)x10;
    ct[1] = (u8)(x10 >> 8);
    ct[2] = (u8)x32;
    ct[3] = (u8)(x32 >> 8);
    ct[4] = (u8)x54;
    ct[5] = (u8)(x54 >> 8);
    ct[6] = (u8)x76;
    ct[7] = (u8)(x76 >> 8);

    return CRYPT_OK;
}

#ifdef LTC_CLEAN_STACK
i32 rc2_ecb_encrypt( u8k *pt,
                            u8 *ct,
                            symmetric_key *skey)
{
    i32 err = _rc2_ecb_encrypt(pt, ct, skey);
    burn_stack(sizeof(unsigned *) + sizeof(unsigned) * 5);
    return err;
}
#endif

/**********************************************************************\
* Decrypt an 8-byte block of ciphertext using the given key.           *
\**********************************************************************/
/**
  Decrypts a block of text with RC2
  @param ct The input ciphertext (8 bytes)
  @param pt The output plaintext (8 bytes)
  @param skey The key as scheduled
  @return CRYPT_OK if successful
*/
#ifdef LTC_CLEAN_STACK
static i32 _rc2_ecb_decrypt( u8k *ct,
                            u8 *pt,
                            symmetric_key *skey)
#else
i32 rc2_ecb_decrypt( u8k *ct,
                            u8 *pt,
                            symmetric_key *skey)
#endif
{
    unsigned x76, x54, x32, x10;
    unsigned *xkey;
    i32 i;

    LTC_ARGCHK(pt  != NULL);
    LTC_ARGCHK(ct != NULL);
    LTC_ARGCHK(skey   != NULL);

    xkey = skey->rc2.xkey;

    x76 = ((unsigned)ct[7] << 8) + (unsigned)ct[6];
    x54 = ((unsigned)ct[5] << 8) + (unsigned)ct[4];
    x32 = ((unsigned)ct[3] << 8) + (unsigned)ct[2];
    x10 = ((unsigned)ct[1] << 8) + (unsigned)ct[0];

    for (i = 15; i >= 0; i--) {
        if (i == 4 || i == 10) {
            x76 = (x76 - xkey[x54 & 63]) & 0xFFFF;
            x54 = (x54 - xkey[x32 & 63]) & 0xFFFF;
            x32 = (x32 - xkey[x10 & 63]) & 0xFFFF;
            x10 = (x10 - xkey[x76 & 63]) & 0xFFFF;
        }

        x76 = ((x76 << 11) | (x76 >> 5));
        x76 = (x76 - ((x10 & ~x54) + (x32 & x54) + xkey[4*i+3])) & 0xFFFF;

        x54 = ((x54 << 13) | (x54 >> 3));
        x54 = (x54 - ((x76 & ~x32) + (x10 & x32) + xkey[4*i+2])) & 0xFFFF;

        x32 = ((x32 << 14) | (x32 >> 2));
        x32 = (x32 - ((x54 & ~x10) + (x76 & x10) + xkey[4*i+1])) & 0xFFFF;

        x10 = ((x10 << 15) | (x10 >> 1));
        x10 = (x10 - ((x32 & ~x76) + (x54 & x76) + xkey[4*i+0])) & 0xFFFF;
    }

    pt[0] = (u8)x10;
    pt[1] = (u8)(x10 >> 8);
    pt[2] = (u8)x32;
    pt[3] = (u8)(x32 >> 8);
    pt[4] = (u8)x54;
    pt[5] = (u8)(x54 >> 8);
    pt[6] = (u8)x76;
    pt[7] = (u8)(x76 >> 8);

    return CRYPT_OK;
}

#ifdef LTC_CLEAN_STACK
i32 rc2_ecb_decrypt( u8k *ct,
                            u8 *pt,
                            symmetric_key *skey)
{
    i32 err = _rc2_ecb_decrypt(ct, pt, skey);
    burn_stack(sizeof(unsigned *) + sizeof(unsigned) * 4 + sizeof(i32));
    return err;
}
#endif

/**
  Performs a self-test of the RC2 block cipher
  @return CRYPT_OK if functional, CRYPT_NOP if self-test has been disabled
*/
i32 rc2_test(void)
{
 #ifndef LTC_TEST
    return CRYPT_NOP;
 #else
   static const struct {
        i32 keylen, bits;
        u8 key[16], pt[8], ct[8];
   } tests[] = {

   { 8, 63,
     { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
     { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
     { 0xeb, 0xb7, 0x73, 0xf9, 0x93, 0x27, 0x8e, 0xff }
   },
   { 8, 64,
     { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
     { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
     { 0x27, 0x8b, 0x27, 0xe4, 0x2e, 0x2f, 0x0d, 0x49 }
   },
   { 8, 64,
     { 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
     { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 },
     { 0x30, 0x64, 0x9e, 0xdf, 0x9b, 0xe7, 0xd2, 0xc2 }
   },
   { 1, 64,
     { 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
     { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
     { 0x61, 0xa8, 0xa2, 0x44, 0xad, 0xac, 0xcc, 0xf0 }
   },
   { 7, 64,
     { 0x88, 0xbc, 0xa9, 0x0e, 0x90, 0x87, 0x5a, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
     { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
     { 0x6c, 0xcf, 0x43, 0x08, 0x97, 0x4c, 0x26, 0x7f }
   },
   { 16, 64,
     { 0x88, 0xbc, 0xa9, 0x0e, 0x90, 0x87, 0x5a, 0x7f,
       0x0f, 0x79, 0xc3, 0x84, 0x62, 0x7b, 0xaf, 0xb2 },
     { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
     { 0x1a, 0x80, 0x7d, 0x27, 0x2b, 0xbe, 0x5d, 0xb1 }
   },
   { 16, 128,
     { 0x88, 0xbc, 0xa9, 0x0e, 0x90, 0x87, 0x5a, 0x7f,
       0x0f, 0x79, 0xc3, 0x84, 0x62, 0x7b, 0xaf, 0xb2 },
     { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
     { 0x22, 0x69, 0x55, 0x2a, 0xb0, 0xf8, 0x5c, 0xa6 }
   }
  };
    i32 x, y, err;
    symmetric_key skey;
    u8 tmp[2][8];

    for (x = 0; x < (i32)(sizeof(tests) / sizeof(tests[0])); x++) {
        zeromem(tmp, sizeof(tmp));
        if (tests[x].bits == (tests[x].keylen * 8)) {
           if ((err = rc2_setup(tests[x].key, tests[x].keylen, 0, &skey)) != CRYPT_OK) {
              return err;
           }
        }
        else {
           if ((err = rc2_setup_ex(tests[x].key, tests[x].keylen, tests[x].bits, 0, &skey)) != CRYPT_OK) {
              return err;
           }
        }

        rc2_ecb_encrypt(tests[x].pt, tmp[0], &skey);
        rc2_ecb_decrypt(tmp[0], tmp[1], &skey);

        if (compare_testvector(tmp[0], 8, tests[x].ct, 8, "RC2 CT", x) ||
              compare_testvector(tmp[1], 8, tests[x].pt, 8, "RC2 PT", x)) {
           return CRYPT_FAIL_TESTVECTOR;
        }

      /* now see if we can encrypt all zero bytes 1000 times, decrypt and come back where we started */
      for (y = 0; y < 8; y++) tmp[0][y] = 0;
      for (y = 0; y < 1000; y++) rc2_ecb_encrypt(tmp[0], tmp[0], &skey);
      for (y = 0; y < 1000; y++) rc2_ecb_decrypt(tmp[0], tmp[0], &skey);
      for (y = 0; y < 8; y++) if (tmp[0][y] != 0) return CRYPT_FAIL_TESTVECTOR;
    }
    return CRYPT_OK;
   #endif
}

/** Terminate the context
   @param skey    The scheduled key
*/
void rc2_done(symmetric_key *skey)
{
  LTC_UNUSED_PARAM(skey);
}

/**
  Gets suitable key size
  @param keysize [in/out] The length of the recommended key (in bytes).  This function will store the suitable size back in this variable.
  @return CRYPT_OK if the input key size is acceptable.
*/
i32 rc2_keysize(i32 *keysize)
{
   LTC_ARGCHK(keysize != NULL);
   if (*keysize < 1) {
       return CRYPT_INVALID_KEYSIZE;
   } else if (*keysize > 128) {
       *keysize = 128;
   }
   return CRYPT_OK;
}

#endif




/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
