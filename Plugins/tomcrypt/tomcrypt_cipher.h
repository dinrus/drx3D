/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* ---- SYMMETRIC KEY STUFF -----
 *
 * We put each of the ciphers scheduled keys in their own structs then we put all of
 * the key formats in one union.  This makes the function prototypes easier to use.
 */
#ifdef LTC_BLOWFISH
struct blowfish_key {
   ulong32 S[4][256];
   ulong32 K[18];
};
#endif

#ifdef LTC_RC5
struct rc5_key {
   i32 rounds;
   ulong32 K[50];
};
#endif

#ifdef LTC_RC6
struct rc6_key {
   ulong32 K[44];
};
#endif

#ifdef LTC_SAFERP
struct saferp_key {
   u8 K[33][16];
   long rounds;
};
#endif

#ifdef LTC_RIJNDAEL
struct rijndael_key {
   ulong32 eK[60], dK[60];
   i32 Nr;
};
#endif

#ifdef LTC_KSEED
struct kseed_key {
    ulong32 K[32], dK[32];
};
#endif

#ifdef LTC_KASUMI
struct kasumi_key {
    ulong32 KLi1[8], KLi2[8],
            KOi1[8], KOi2[8], KOi3[8],
            KIi1[8], KIi2[8], KIi3[8];
};
#endif

#ifdef LTC_XTEA
struct xtea_key {
   u64 A[32], B[32];
};
#endif

#ifdef LTC_TWOFISH
#ifndef LTC_TWOFISH_SMALL
   struct twofish_key {
      ulong32 S[4][256], K[40];
   };
#else
   struct twofish_key {
      ulong32 K[40];
      u8 S[32], start;
   };
#endif
#endif

#ifdef LTC_SAFER
#define LTC_SAFER_K64_DEFAULT_NOF_ROUNDS     6
#define LTC_SAFER_K128_DEFAULT_NOF_ROUNDS   10
#define LTC_SAFER_SK64_DEFAULT_NOF_ROUNDS    8
#define LTC_SAFER_SK128_DEFAULT_NOF_ROUNDS  10
#define LTC_SAFER_MAX_NOF_ROUNDS            13
#define LTC_SAFER_BLOCK_LEN                  8
#define LTC_SAFER_KEY_LEN     (1 + LTC_SAFER_BLOCK_LEN * (1 + 2 * LTC_SAFER_MAX_NOF_ROUNDS))
typedef u8 safer_block_t[LTC_SAFER_BLOCK_LEN];
typedef u8 safer_key_t[LTC_SAFER_KEY_LEN];
struct safer_key { safer_key_t key; };
#endif

#ifdef LTC_RC2
struct rc2_key { unsigned xkey[64]; };
#endif

#ifdef LTC_DES
struct des_key {
    ulong32 ek[32], dk[32];
};

struct des3_key {
    ulong32 ek[3][32], dk[3][32];
};
#endif

#ifdef LTC_CAST5
struct cast5_key {
    ulong32 K[32], keylen;
};
#endif

#ifdef LTC_NOEKEON
struct noekeon_key {
    ulong32 K[4], dK[4];
};
#endif

#ifdef LTC_SKIPJACK
struct skipjack_key {
    u8 key[10];
};
#endif

#ifdef LTC_KHAZAD
struct khazad_key {
   ulong64 roundKeyEnc[8 + 1];
   ulong64 roundKeyDec[8 + 1];
};
#endif

#ifdef LTC_ANUBIS
struct anubis_key {
   i32 keyBits;
   i32 R;
   ulong32 roundKeyEnc[18 + 1][4];
   ulong32 roundKeyDec[18 + 1][4];
};
#endif

#ifdef LTC_MULTI2
struct multi2_key {
    i32 N;
    ulong32 uk[8];
};
#endif

#ifdef LTC_CAMELLIA
struct camellia_key {
    i32 R;
    ulong64 kw[4], k[24], kl[6];
};
#endif

typedef union Symmetric_key {
#ifdef LTC_DES
   struct des_key des;
   struct des3_key des3;
#endif
#ifdef LTC_RC2
   struct rc2_key rc2;
#endif
#ifdef LTC_SAFER
   struct safer_key safer;
#endif
#ifdef LTC_TWOFISH
   struct twofish_key  twofish;
#endif
#ifdef LTC_BLOWFISH
   struct blowfish_key blowfish;
#endif
#ifdef LTC_RC5
   struct rc5_key      rc5;
#endif
#ifdef LTC_RC6
   struct rc6_key      rc6;
#endif
#ifdef LTC_SAFERP
   struct saferp_key   saferp;
#endif
#ifdef LTC_RIJNDAEL
   struct rijndael_key rijndael;
#endif
#ifdef LTC_XTEA
   struct xtea_key     xtea;
#endif
#ifdef LTC_CAST5
   struct cast5_key    cast5;
#endif
#ifdef LTC_NOEKEON
   struct noekeon_key  noekeon;
#endif
#ifdef LTC_SKIPJACK
   struct skipjack_key skipjack;
#endif
#ifdef LTC_KHAZAD
   struct khazad_key   khazad;
#endif
#ifdef LTC_ANUBIS
   struct anubis_key   anubis;
#endif
#ifdef LTC_KSEED
   struct kseed_key    kseed;
#endif
#ifdef LTC_KASUMI
   struct kasumi_key   kasumi;
#endif
#ifdef LTC_MULTI2
   struct multi2_key   multi2;
#endif
#ifdef LTC_CAMELLIA
   struct camellia_key camellia;
#endif
   void   *data;
} symmetric_key;

#ifdef LTC_ECB_MODE
/** A block cipher ECB structure */
typedef struct {
   /** The index of the cipher chosen */
   i32                 cipher,
   /** The block size of the given cipher */
                       blocklen;
   /** The scheduled key */
   symmetric_key       key;
} symmetric_ECB;
#endif

#ifdef LTC_CFB_MODE
/** A block cipher CFB structure */
typedef struct {
   /** The index of the cipher chosen */
   i32                 cipher,
   /** The block size of the given cipher */
                       blocklen,
   /** The padding offset */
                       padlen;
   /** The current IV */
   u8       IV[MAXBLOCKSIZE],
   /** The pad used to encrypt/decrypt */
                       pad[MAXBLOCKSIZE];
   /** The scheduled key */
   symmetric_key       key;
} symmetric_CFB;
#endif

#ifdef LTC_OFB_MODE
/** A block cipher OFB structure */
typedef struct {
   /** The index of the cipher chosen */
   i32                 cipher,
   /** The block size of the given cipher */
                       blocklen,
   /** The padding offset */
                       padlen;
   /** The current IV */
   u8       IV[MAXBLOCKSIZE];
   /** The scheduled key */
   symmetric_key       key;
} symmetric_OFB;
#endif

#ifdef LTC_CBC_MODE
/** A block cipher CBC structure */
typedef struct {
   /** The index of the cipher chosen */
   i32                 cipher,
   /** The block size of the given cipher */
                       blocklen;
   /** The current IV */
   u8       IV[MAXBLOCKSIZE];
   /** The scheduled key */
   symmetric_key       key;
} symmetric_CBC;
#endif


#ifdef LTC_CTR_MODE
/** A block cipher CTR structure */
typedef struct {
   /** The index of the cipher chosen */
   i32                 cipher,
   /** The block size of the given cipher */
                       blocklen,
   /** The padding offset */
                       padlen,
   /** The mode (endianess) of the CTR, 0==little, 1==big */
                       mode,
   /** counter width */
                       ctrlen;

   /** The counter */
   u8       ctr[MAXBLOCKSIZE],
   /** The pad used to encrypt/decrypt */
                       pad[MAXBLOCKSIZE];
   /** The scheduled key */
   symmetric_key       key;
} symmetric_CTR;
#endif


#ifdef LTC_LRW_MODE
/** A LRW structure */
typedef struct {
    /** The index of the cipher chosen (must be a 128-bit block cipher) */
    i32               cipher;

    /** The current IV */
    u8     IV[16],

    /** the tweak key */
                      tweak[16],

    /** The current pad, it's the product of the first 15 bytes against the tweak key */
                      pad[16];

    /** The scheduled symmetric key */
    symmetric_key     key;

#ifdef LTC_LRW_TABLES
    /** The pre-computed multiplication table */
    u8     PC[16][256][16];
#endif
} symmetric_LRW;
#endif

#ifdef LTC_F8_MODE
/** A block cipher F8 structure */
typedef struct {
   /** The index of the cipher chosen */
   i32                 cipher,
   /** The block size of the given cipher */
                       blocklen,
   /** The padding offset */
                       padlen;
   /** The current IV */
   u8       IV[MAXBLOCKSIZE],
                       MIV[MAXBLOCKSIZE];
   /** Current block count */
   ulong32             blockcnt;
   /** The scheduled key */
   symmetric_key       key;
} symmetric_F8;
#endif


/** cipher descriptor table, last entry has "name == NULL" to mark the end of table */
extern struct ltc_cipher_descriptor {
   /** name of cipher */
   tukk name;
   /** internal ID */
   u8 ID;
   /** min keysize (octets) */
   i32  min_key_length,
   /** max keysize (octets) */
        max_key_length,
   /** block size (octets) */
        block_length,
   /** default number of rounds */
        default_rounds;
   /** Setup the cipher
      @param key         The input symmetric key
      @param keylen      The length of the input key (octets)
      @param num_rounds  The requested number of rounds (0==default)
      @param skey        [out] The destination of the scheduled key
      @return CRYPT_OK if successful
   */
   i32  (*setup)(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
   /** Encrypt a block
      @param pt      The plaintext
      @param ct      [out] The ciphertext
      @param skey    The scheduled key
      @return CRYPT_OK if successful
   */
   i32 (*ecb_encrypt)(u8k *pt, u8 *ct, symmetric_key *skey);
   /** Decrypt a block
      @param ct      The ciphertext
      @param pt      [out] The plaintext
      @param skey    The scheduled key
      @return CRYPT_OK if successful
   */
   i32 (*ecb_decrypt)(u8k *ct, u8 *pt, symmetric_key *skey);
   /** Test the block cipher
       @return CRYPT_OK if successful, CRYPT_NOP if self-testing has been disabled
   */
   i32 (*test)(void);

   /** Terminate the context
      @param skey    The scheduled key
   */
   void (*done)(symmetric_key *skey);

   /** Determine a key size
       @param keysize    [in/out] The size of the key desired and the suggested size
       @return CRYPT_OK if successful
   */
   i32  (*keysize)(i32 *keysize);

/** Accelerators **/
   /** Accelerated ECB encryption
       @param pt      Plaintext
       @param ct      Ciphertext
       @param blocks  The number of complete blocks to process
       @param skey    The scheduled key context
       @return CRYPT_OK if successful
   */
   i32 (*accel_ecb_encrypt)(u8k *pt, u8 *ct, u64 blocks, symmetric_key *skey);

   /** Accelerated ECB decryption
       @param pt      Plaintext
       @param ct      Ciphertext
       @param blocks  The number of complete blocks to process
       @param skey    The scheduled key context
       @return CRYPT_OK if successful
   */
   i32 (*accel_ecb_decrypt)(u8k *ct, u8 *pt, u64 blocks, symmetric_key *skey);

   /** Accelerated CBC encryption
       @param pt      Plaintext
       @param ct      Ciphertext
       @param blocks  The number of complete blocks to process
       @param IV      The initial value (input/output)
       @param skey    The scheduled key context
       @return CRYPT_OK if successful
   */
   i32 (*accel_cbc_encrypt)(u8k *pt, u8 *ct, u64 blocks, u8 *IV, symmetric_key *skey);

   /** Accelerated CBC decryption
       @param pt      Plaintext
       @param ct      Ciphertext
       @param blocks  The number of complete blocks to process
       @param IV      The initial value (input/output)
       @param skey    The scheduled key context
       @return CRYPT_OK if successful
   */
   i32 (*accel_cbc_decrypt)(u8k *ct, u8 *pt, u64 blocks, u8 *IV, symmetric_key *skey);

   /** Accelerated CTR encryption
       @param pt      Plaintext
       @param ct      Ciphertext
       @param blocks  The number of complete blocks to process
       @param IV      The initial value (input/output)
       @param mode    little or big endian counter (mode=0 or mode=1)
       @param skey    The scheduled key context
       @return CRYPT_OK if successful
   */
   i32 (*accel_ctr_encrypt)(u8k *pt, u8 *ct, u64 blocks, u8 *IV, i32 mode, symmetric_key *skey);

   /** Accelerated LRW
       @param pt      Plaintext
       @param ct      Ciphertext
       @param blocks  The number of complete blocks to process
       @param IV      The initial value (input/output)
       @param tweak   The LRW tweak
       @param skey    The scheduled key context
       @return CRYPT_OK if successful
   */
   i32 (*accel_lrw_encrypt)(u8k *pt, u8 *ct, u64 blocks, u8 *IV, u8k *tweak, symmetric_key *skey);

   /** Accelerated LRW
       @param ct      Ciphertext
       @param pt      Plaintext
       @param blocks  The number of complete blocks to process
       @param IV      The initial value (input/output)
       @param tweak   The LRW tweak
       @param skey    The scheduled key context
       @return CRYPT_OK if successful
   */
   i32 (*accel_lrw_decrypt)(u8k *ct, u8 *pt, u64 blocks, u8 *IV, u8k *tweak, symmetric_key *skey);

   /** Accelerated CCM packet (one-shot)
       @param key        The secret key to use
       @param keylen     The length of the secret key (octets)
       @param uskey      A previously scheduled key [optional can be NULL]
       @param nonce      The session nonce [use once]
       @param noncelen   The length of the nonce
       @param header     The header for the session
       @param headerlen  The length of the header (octets)
       @param pt         [out] The plaintext
       @param ptlen      The length of the plaintext (octets)
       @param ct         [out] The ciphertext
       @param tag        [out] The destination tag
       @param taglen     [in/out] The max size and resulting size of the authentication tag
       @param direction  Encrypt or Decrypt direction (0 or 1)
       @return CRYPT_OK if successful
   */
   i32 (*accel_ccm_memory)(
       u8k *key,    u64 keylen,
       symmetric_key       *uskey,
       u8k *nonce,  u64 noncelen,
       u8k *header, u64 headerlen,
             u8 *pt,     u64 ptlen,
             u8 *ct,
             u8 *tag,    u64 *taglen,
                       i32  direction);

   /** Accelerated GCM packet (one shot)
       @param key        The secret key
       @param keylen     The length of the secret key
       @param IV         The initialization vector
       @param IVlen      The length of the initialization vector
       @param adata      The additional authentication data (header)
       @param adatalen   The length of the adata
       @param pt         The plaintext
       @param ptlen      The length of the plaintext (ciphertext length is the same)
       @param ct         The ciphertext
       @param tag        [out] The MAC tag
       @param taglen     [in/out] The MAC tag length
       @param direction  Encrypt or Decrypt mode (GCM_ENCRYPT or GCM_DECRYPT)
       @return CRYPT_OK on success
   */
   i32 (*accel_gcm_memory)(
       u8k *key,    u64 keylen,
       u8k *IV,     u64 IVlen,
       u8k *adata,  u64 adatalen,
             u8 *pt,     u64 ptlen,
             u8 *ct,
             u8 *tag,    u64 *taglen,
                       i32 direction);

   /** Accelerated one shot LTC_OMAC
       @param key            The secret key
       @param keylen         The key length (octets)
       @param in             The message
       @param inlen          Length of message (octets)
       @param out            [out] Destination for tag
       @param outlen         [in/out] Initial and final size of out
       @return CRYPT_OK on success
   */
   i32 (*omac_memory)(
       u8k *key, u64 keylen,
       u8k *in,  u64 inlen,
             u8 *out, u64 *outlen);

   /** Accelerated one shot XCBC
       @param key            The secret key
       @param keylen         The key length (octets)
       @param in             The message
       @param inlen          Length of message (octets)
       @param out            [out] Destination for tag
       @param outlen         [in/out] Initial and final size of out
       @return CRYPT_OK on success
   */
   i32 (*xcbc_memory)(
       u8k *key, u64 keylen,
       u8k *in,  u64 inlen,
             u8 *out, u64 *outlen);

   /** Accelerated one shot F9
       @param key            The secret key
       @param keylen         The key length (octets)
       @param in             The message
       @param inlen          Length of message (octets)
       @param out            [out] Destination for tag
       @param outlen         [in/out] Initial and final size of out
       @return CRYPT_OK on success
       @remark Requires manual padding
   */
   i32 (*f9_memory)(
       u8k *key, u64 keylen,
       u8k *in,  u64 inlen,
             u8 *out, u64 *outlen);

   /** Accelerated XTS encryption
       @param pt      Plaintext
       @param ct      Ciphertext
       @param blocks  The number of complete blocks to process
       @param tweak   The 128-bit encryption tweak (input/output).
                      The tweak should not be encrypted on input, but
                      next tweak will be copied encrypted on output.
       @param skey1   The first scheduled key context
       @param skey2   The second scheduled key context
       @return CRYPT_OK if successful
    */
    i32 (*accel_xts_encrypt)(u8k *pt, u8 *ct,
        u64 blocks, u8 *tweak, symmetric_key *skey1,
        symmetric_key *skey2);

    /** Accelerated XTS decryption
        @param ct      Ciphertext
        @param pt      Plaintext
        @param blocks  The number of complete blocks to process
        @param tweak   The 128-bit encryption tweak (input/output).
                       The tweak should not be encrypted on input, but
                       next tweak will be copied encrypted on output.
        @param skey1   The first scheduled key context
        @param skey2   The second scheduled key context
        @return CRYPT_OK if successful
     */
     i32 (*accel_xts_decrypt)(u8k *ct, u8 *pt,
         u64 blocks, u8 *tweak, symmetric_key *skey1,
         symmetric_key *skey2);
} cipher_descriptor[];

#ifdef LTC_BLOWFISH
i32 blowfish_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 blowfish_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 blowfish_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 blowfish_test(void);
void blowfish_done(symmetric_key *skey);
i32 blowfish_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor blowfish_desc;
#endif

#ifdef LTC_RC5
i32 rc5_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 rc5_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 rc5_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 rc5_test(void);
void rc5_done(symmetric_key *skey);
i32 rc5_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor rc5_desc;
#endif

#ifdef LTC_RC6
i32 rc6_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 rc6_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 rc6_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 rc6_test(void);
void rc6_done(symmetric_key *skey);
i32 rc6_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor rc6_desc;
#endif

#ifdef LTC_RC2
i32 rc2_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 rc2_setup_ex(u8k *key, i32 keylen, i32 bits, i32 num_rounds, symmetric_key *skey);
i32 rc2_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 rc2_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 rc2_test(void);
void rc2_done(symmetric_key *skey);
i32 rc2_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor rc2_desc;
#endif

#ifdef LTC_SAFERP
i32 saferp_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 saferp_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 saferp_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 saferp_test(void);
void saferp_done(symmetric_key *skey);
i32 saferp_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor saferp_desc;
#endif

#ifdef LTC_SAFER
i32 safer_k64_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 safer_sk64_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 safer_k128_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 safer_sk128_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 safer_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *key);
i32 safer_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *key);
i32 safer_k64_test(void);
i32 safer_sk64_test(void);
i32 safer_sk128_test(void);
void safer_done(symmetric_key *skey);
i32 safer_64_keysize(i32 *keysize);
i32 safer_128_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor safer_k64_desc, safer_k128_desc, safer_sk64_desc, safer_sk128_desc;
#endif

#ifdef LTC_RIJNDAEL

/* make aes an alias */
#define aes_setup           rijndael_setup
#define aes_ecb_encrypt     rijndael_ecb_encrypt
#define aes_ecb_decrypt     rijndael_ecb_decrypt
#define aes_test            rijndael_test
#define aes_done            rijndael_done
#define aes_keysize         rijndael_keysize

#define aes_enc_setup           rijndael_enc_setup
#define aes_enc_ecb_encrypt     rijndael_enc_ecb_encrypt
#define aes_enc_keysize         rijndael_enc_keysize

i32 rijndael_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 rijndael_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 rijndael_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 rijndael_test(void);
void rijndael_done(symmetric_key *skey);
i32 rijndael_keysize(i32 *keysize);
i32 rijndael_enc_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 rijndael_enc_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
void rijndael_enc_done(symmetric_key *skey);
i32 rijndael_enc_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor rijndael_desc, aes_desc;
extern const struct ltc_cipher_descriptor rijndael_enc_desc, aes_enc_desc;
#endif

#ifdef LTC_XTEA
i32 xtea_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 xtea_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 xtea_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 xtea_test(void);
void xtea_done(symmetric_key *skey);
i32 xtea_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor xtea_desc;
#endif

#ifdef LTC_TWOFISH
i32 twofish_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 twofish_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 twofish_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 twofish_test(void);
void twofish_done(symmetric_key *skey);
i32 twofish_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor twofish_desc;
#endif

#ifdef LTC_DES
i32 des_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 des_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 des_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 des_test(void);
void des_done(symmetric_key *skey);
i32 des_keysize(i32 *keysize);
i32 des3_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 des3_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 des3_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 des3_test(void);
void des3_done(symmetric_key *skey);
i32 des3_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor des_desc, des3_desc;
#endif

#ifdef LTC_CAST5
i32 cast5_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 cast5_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 cast5_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 cast5_test(void);
void cast5_done(symmetric_key *skey);
i32 cast5_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor cast5_desc;
#endif

#ifdef LTC_NOEKEON
i32 noekeon_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 noekeon_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 noekeon_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 noekeon_test(void);
void noekeon_done(symmetric_key *skey);
i32 noekeon_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor noekeon_desc;
#endif

#ifdef LTC_SKIPJACK
i32 skipjack_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 skipjack_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 skipjack_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 skipjack_test(void);
void skipjack_done(symmetric_key *skey);
i32 skipjack_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor skipjack_desc;
#endif

#ifdef LTC_KHAZAD
i32 khazad_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 khazad_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 khazad_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 khazad_test(void);
void khazad_done(symmetric_key *skey);
i32 khazad_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor khazad_desc;
#endif

#ifdef LTC_ANUBIS
i32 anubis_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 anubis_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 anubis_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 anubis_test(void);
void anubis_done(symmetric_key *skey);
i32 anubis_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor anubis_desc;
#endif

#ifdef LTC_KSEED
i32 kseed_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 kseed_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 kseed_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 kseed_test(void);
void kseed_done(symmetric_key *skey);
i32 kseed_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor kseed_desc;
#endif

#ifdef LTC_KASUMI
i32 kasumi_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 kasumi_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 kasumi_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 kasumi_test(void);
void kasumi_done(symmetric_key *skey);
i32 kasumi_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor kasumi_desc;
#endif


#ifdef LTC_MULTI2
i32 multi2_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 multi2_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 multi2_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 multi2_test(void);
void multi2_done(symmetric_key *skey);
i32 multi2_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor multi2_desc;
#endif

#ifdef LTC_CAMELLIA
i32 camellia_setup(u8k *key, i32 keylen, i32 num_rounds, symmetric_key *skey);
i32 camellia_ecb_encrypt(u8k *pt, u8 *ct, symmetric_key *skey);
i32 camellia_ecb_decrypt(u8k *ct, u8 *pt, symmetric_key *skey);
i32 camellia_test(void);
void camellia_done(symmetric_key *skey);
i32 camellia_keysize(i32 *keysize);
extern const struct ltc_cipher_descriptor camellia_desc;
#endif

#ifdef LTC_ECB_MODE
i32 ecb_start(i32 cipher, u8k *key,
              i32 keylen, i32 num_rounds, symmetric_ECB *ecb);
i32 ecb_encrypt(u8k *pt, u8 *ct, u64 len, symmetric_ECB *ecb);
i32 ecb_decrypt(u8k *ct, u8 *pt, u64 len, symmetric_ECB *ecb);
i32 ecb_done(symmetric_ECB *ecb);
#endif

#ifdef LTC_CFB_MODE
i32 cfb_start(i32 cipher, u8k *IV, u8k *key,
              i32 keylen, i32 num_rounds, symmetric_CFB *cfb);
i32 cfb_encrypt(u8k *pt, u8 *ct, u64 len, symmetric_CFB *cfb);
i32 cfb_decrypt(u8k *ct, u8 *pt, u64 len, symmetric_CFB *cfb);
i32 cfb_getiv(u8 *IV, u64 *len, symmetric_CFB *cfb);
i32 cfb_setiv(u8k *IV, u64 len, symmetric_CFB *cfb);
i32 cfb_done(symmetric_CFB *cfb);
#endif

#ifdef LTC_OFB_MODE
i32 ofb_start(i32 cipher, u8k *IV, u8k *key,
              i32 keylen, i32 num_rounds, symmetric_OFB *ofb);
i32 ofb_encrypt(u8k *pt, u8 *ct, u64 len, symmetric_OFB *ofb);
i32 ofb_decrypt(u8k *ct, u8 *pt, u64 len, symmetric_OFB *ofb);
i32 ofb_getiv(u8 *IV, u64 *len, symmetric_OFB *ofb);
i32 ofb_setiv(u8k *IV, u64 len, symmetric_OFB *ofb);
i32 ofb_done(symmetric_OFB *ofb);
#endif

#ifdef LTC_CBC_MODE
i32 cbc_start(i32 cipher, u8k *IV, u8k *key,
               i32 keylen, i32 num_rounds, symmetric_CBC *cbc);
i32 cbc_encrypt(u8k *pt, u8 *ct, u64 len, symmetric_CBC *cbc);
i32 cbc_decrypt(u8k *ct, u8 *pt, u64 len, symmetric_CBC *cbc);
i32 cbc_getiv(u8 *IV, u64 *len, symmetric_CBC *cbc);
i32 cbc_setiv(u8k *IV, u64 len, symmetric_CBC *cbc);
i32 cbc_done(symmetric_CBC *cbc);
#endif

#ifdef LTC_CTR_MODE

#define CTR_COUNTER_LITTLE_ENDIAN    0x0000
#define CTR_COUNTER_BIG_ENDIAN       0x1000
#define LTC_CTR_RFC3686              0x2000

i32 ctr_start(               i32   cipher,
              u8k *IV,
              u8k *key,       i32 keylen,
                             i32  num_rounds, i32 ctr_mode,
                   symmetric_CTR *ctr);
i32 ctr_encrypt(u8k *pt, u8 *ct, u64 len, symmetric_CTR *ctr);
i32 ctr_decrypt(u8k *ct, u8 *pt, u64 len, symmetric_CTR *ctr);
i32 ctr_getiv(u8 *IV, u64 *len, symmetric_CTR *ctr);
i32 ctr_setiv(u8k *IV, u64 len, symmetric_CTR *ctr);
i32 ctr_done(symmetric_CTR *ctr);
i32 ctr_test(void);
#endif

#ifdef LTC_LRW_MODE

#define LRW_ENCRYPT LTC_ENCRYPT
#define LRW_DECRYPT LTC_DECRYPT

i32 lrw_start(               i32   cipher,
              u8k *IV,
              u8k *key,       i32 keylen,
              u8k *tweak,
                             i32  num_rounds,
                   symmetric_LRW *lrw);
i32 lrw_encrypt(u8k *pt, u8 *ct, u64 len, symmetric_LRW *lrw);
i32 lrw_decrypt(u8k *ct, u8 *pt, u64 len, symmetric_LRW *lrw);
i32 lrw_getiv(u8 *IV, u64 *len, symmetric_LRW *lrw);
i32 lrw_setiv(u8k *IV, u64 len, symmetric_LRW *lrw);
i32 lrw_done(symmetric_LRW *lrw);
i32 lrw_test(void);

/* don't call */
i32 lrw_process(u8k *pt, u8 *ct, u64 len, i32 mode, symmetric_LRW *lrw);
#endif

#ifdef LTC_F8_MODE
i32 f8_start(                i32  cipher, u8k *IV,
             u8k *key,                    i32  keylen,
             u8k *salt_key,               i32  skeylen,
                             i32  num_rounds,   symmetric_F8  *f8);
i32 f8_encrypt(u8k *pt, u8 *ct, u64 len, symmetric_F8 *f8);
i32 f8_decrypt(u8k *ct, u8 *pt, u64 len, symmetric_F8 *f8);
i32 f8_getiv(u8 *IV, u64 *len, symmetric_F8 *f8);
i32 f8_setiv(u8k *IV, u64 len, symmetric_F8 *f8);
i32 f8_done(symmetric_F8 *f8);
i32 f8_test_mode(void);
#endif

#ifdef LTC_XTS_MODE
typedef struct {
   symmetric_key  key1, key2;
   i32            cipher;
} symmetric_xts;

i32 xts_start(                i32  cipher,
              u8k *key1,
              u8k *key2,
                    u64  keylen,
                              i32  num_rounds,
                    symmetric_xts *xts);

i32 xts_encrypt(
   u8k *pt, u64 ptlen,
         u8 *ct,
         u8 *tweak,
         symmetric_xts *xts);
i32 xts_decrypt(
   u8k *ct, u64 ptlen,
         u8 *pt,
         u8 *tweak,
         symmetric_xts *xts);

void xts_done(symmetric_xts *xts);
i32  xts_test(void);
void xts_mult_x(u8 *I);
#endif

i32 find_cipher(tukk name);
i32 find_cipher_any(tukk name, i32 blocklen, i32 keylen);
i32 find_cipher_id(u8 ID);
i32 register_cipher(const struct ltc_cipher_descriptor *cipher);
i32 unregister_cipher(const struct ltc_cipher_descriptor *cipher);
i32 register_all_ciphers(void);
i32 cipher_is_valid(i32 idx);

LTC_MUTEX_PROTO(ltc_cipher_mutex)

/* ---- stream ciphers ---- */

#ifdef LTC_CHACHA

typedef struct {
   ulong32 input[16];
   u8 kstream[64];
   u64 ksleft;
   u64 ivlen;
   i32 rounds;
} chacha_state;

i32 chacha_setup(chacha_state *st, u8k *key, u64 keylen, i32 rounds);
i32 chacha_ivctr32(chacha_state *st, u8k *iv, u64 ivlen, ulong32 counter);
i32 chacha_ivctr64(chacha_state *st, u8k *iv, u64 ivlen, ulong64 counter);
i32 chacha_crypt(chacha_state *st, u8k *in, u64 inlen, u8 *out);
i32 chacha_keystream(chacha_state *st, u8 *out, u64 outlen);
i32 chacha_done(chacha_state *st);
i32 chacha_test(void);

#endif /* LTC_CHACHA */

#ifdef LTC_RC4_STREAM

typedef struct {
   u32 x, y;
   u8 buf[256];
} rc4_state;

i32 rc4_stream_setup(rc4_state *st, u8k *key, u64 keylen);
i32 rc4_stream_crypt(rc4_state *st, u8k *in, u64 inlen, u8 *out);
i32 rc4_stream_keystream(rc4_state *st, u8 *out, u64 outlen);
i32 rc4_stream_done(rc4_state *st);
i32 rc4_stream_test(void);

#endif /* LTC_RC4_STREAM */

#ifdef LTC_SOBER128_STREAM

typedef struct {
   ulong32 R[17],       /* Working storage for the shift register */
           initR[17],   /* saved register contents */
           konst,       /* key dependent constant */
           sbuf;        /* partial word encryption buffer */
   i32     nbuf;        /* number of part-word stream bits buffered */
} sober128_state;

i32 sober128_stream_setup(sober128_state *st, u8k *key, u64 keylen);
i32 sober128_stream_setiv(sober128_state *st, u8k *iv, u64 ivlen);
i32 sober128_stream_crypt(sober128_state *st, u8k *in, u64 inlen, u8 *out);
i32 sober128_stream_keystream(sober128_state *st, u8 *out, u64 outlen);
i32 sober128_stream_done(sober128_state *st);
i32 sober128_stream_test(void);

#endif /* LTC_SOBER128_STREAM */

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
