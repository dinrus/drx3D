/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* ---- HASH FUNCTIONS ---- */
#ifdef LTC_SHA3
struct sha3_state {
    ulong64 saved;                  /* the portion of the input message that we didn't consume yet */
    ulong64 s[25];
    u8 sb[25 * 8];       /* used for storing `ulong64 s[25]` as little-endian bytes */
    unsigned short byte_index;      /* 0..7--the next byte after the set one (starts from 0; 0--none are buffered) */
    unsigned short word_index;      /* 0..24--the next word to integrate input (starts from 0) */
    unsigned short capacity_words;  /* the double size of the hash output in words (e.g. 16 for Keccak 512) */
    unsigned short xof_flag;
};
#endif

#ifdef LTC_SHA512
struct sha512_state {
    ulong64  length, state[8];
    u64 curlen;
    u8 buf[128];
};
#endif

#ifdef LTC_SHA256
struct sha256_state {
    ulong64 length;
    ulong32 state[8], curlen;
    u8 buf[64];
};
#endif

#ifdef LTC_SHA1
struct sha1_state {
    ulong64 length;
    ulong32 state[5], curlen;
    u8 buf[64];
};
#endif

#ifdef LTC_MD5
struct md5_state {
    ulong64 length;
    ulong32 state[4], curlen;
    u8 buf[64];
};
#endif

#ifdef LTC_MD4
struct md4_state {
    ulong64 length;
    ulong32 state[4], curlen;
    u8 buf[64];
};
#endif

#ifdef LTC_TIGER
struct tiger_state {
    ulong64 state[3], length;
    u64 curlen;
    u8 buf[64];
};
#endif

#ifdef LTC_MD2
struct md2_state {
    u8 chksum[16], X[48], buf[16];
    u64 curlen;
};
#endif

#ifdef LTC_RIPEMD128
struct rmd128_state {
    ulong64 length;
    u8 buf[64];
    ulong32 curlen, state[4];
};
#endif

#ifdef LTC_RIPEMD160
struct rmd160_state {
    ulong64 length;
    u8 buf[64];
    ulong32 curlen, state[5];
};
#endif

#ifdef LTC_RIPEMD256
struct rmd256_state {
    ulong64 length;
    u8 buf[64];
    ulong32 curlen, state[8];
};
#endif

#ifdef LTC_RIPEMD320
struct rmd320_state {
    ulong64 length;
    u8 buf[64];
    ulong32 curlen, state[10];
};
#endif

#ifdef LTC_WHIRLPOOL
struct whirlpool_state {
    ulong64 length, state[8];
    u8 buf[64];
    ulong32 curlen;
};
#endif

#ifdef LTC_CHC_HASH
struct chc_state {
    ulong64 length;
    u8 state[MAXBLOCKSIZE], buf[MAXBLOCKSIZE];
    ulong32 curlen;
};
#endif

#ifdef LTC_BLAKE2S
struct blake2s_state {
    ulong32 h[8];
    ulong32 t[2];
    ulong32 f[2];
    u8 buf[64];
    u64 curlen;
    u64 outlen;
    u8 last_node;
};
#endif

#ifdef LTC_BLAKE2B
struct blake2b_state {
    ulong64 h[8];
    ulong64 t[2];
    ulong64 f[2];
    u8 buf[128];
    u64 curlen;
    u64 outlen;
    u8 last_node;
};
#endif

typedef union Hash_state {
    char dummy[1];
#ifdef LTC_CHC_HASH
    struct chc_state chc;
#endif
#ifdef LTC_WHIRLPOOL
    struct whirlpool_state whirlpool;
#endif
#ifdef LTC_SHA3
    struct sha3_state sha3;
#endif
#ifdef LTC_SHA512
    struct sha512_state sha512;
#endif
#ifdef LTC_SHA256
    struct sha256_state sha256;
#endif
#ifdef LTC_SHA1
    struct sha1_state   sha1;
#endif
#ifdef LTC_MD5
    struct md5_state    md5;
#endif
#ifdef LTC_MD4
    struct md4_state    md4;
#endif
#ifdef LTC_MD2
    struct md2_state    md2;
#endif
#ifdef LTC_TIGER
    struct tiger_state  tiger;
#endif
#ifdef LTC_RIPEMD128
    struct rmd128_state rmd128;
#endif
#ifdef LTC_RIPEMD160
    struct rmd160_state rmd160;
#endif
#ifdef LTC_RIPEMD256
    struct rmd256_state rmd256;
#endif
#ifdef LTC_RIPEMD320
    struct rmd320_state rmd320;
#endif
#ifdef LTC_BLAKE2S
    struct blake2s_state blake2s;
#endif
#ifdef LTC_BLAKE2B
    struct blake2b_state blake2b;
#endif

    uk data;
} hash_state;

/** hash descriptor */
extern  struct ltc_hash_descriptor {
    /** name of hash */
    tukk name;
    /** internal ID */
    u8 ID;
    /** Size of digest in octets */
    u64 hashsize;
    /** Input block size in octets */
    u64 blocksize;
    /** ASN.1 OID */
    u64 OID[16];
    /** Length of DER encoding */
    u64 OIDlen;

    /** Init a hash state
      @param hash   The hash to initialize
      @return CRYPT_OK if successful
    */
    i32 (*init)(hash_state *hash);
    /** Process a block of data
      @param hash   The hash state
      @param in     The data to hash
      @param inlen  The length of the data (octets)
      @return CRYPT_OK if successful
    */
    i32 (*process)(hash_state *hash, u8k *in, u64 inlen);
    /** Produce the digest and store it
      @param hash   The hash state
      @param out    [out] The destination of the digest
      @return CRYPT_OK if successful
    */
    i32 (*done)(hash_state *hash, u8 *out);
    /** Self-test
      @return CRYPT_OK if successful, CRYPT_NOP if self-tests have been disabled
    */
    i32 (*test)(void);

    /* accelerated hmac callback: if you need to-do multiple packets just use the generic hmac_memory and provide a hash callback */
    i32  (*hmac_block)(u8k *key, u64  keylen,
                       u8k *in,  u64  inlen,
                             u8 *out, u64 *outlen);

} hash_descriptor[];

#ifdef LTC_CHC_HASH
i32 chc_register(i32 cipher);
i32 chc_init(hash_state * md);
i32 chc_process(hash_state * md, u8k *in, u64 inlen);
i32 chc_done(hash_state * md, u8 *hash);
i32 chc_test(void);
extern const struct ltc_hash_descriptor chc_desc;
#endif

#ifdef LTC_WHIRLPOOL
i32 whirlpool_init(hash_state * md);
i32 whirlpool_process(hash_state * md, u8k *in, u64 inlen);
i32 whirlpool_done(hash_state * md, u8 *hash);
i32 whirlpool_test(void);
extern const struct ltc_hash_descriptor whirlpool_desc;
#endif

#ifdef LTC_SHA3
i32 sha3_512_init(hash_state * md);
i32 sha3_512_test(void);
extern const struct ltc_hash_descriptor sha3_512_desc;
i32 sha3_384_init(hash_state * md);
i32 sha3_384_test(void);
extern const struct ltc_hash_descriptor sha3_384_desc;
i32 sha3_256_init(hash_state * md);
i32 sha3_256_test(void);
extern const struct ltc_hash_descriptor sha3_256_desc;
i32 sha3_224_init(hash_state * md);
i32 sha3_224_test(void);
extern const struct ltc_hash_descriptor sha3_224_desc;
/* process + done are the same for all variants */
i32 sha3_process(hash_state * md, u8k *in, u64 inlen);
i32 sha3_done(hash_state *md, u8 *hash);
/* SHAKE128 + SHAKE256 */
i32 sha3_shake_init(hash_state *md, i32 num);
#define sha3_shake_process(a,b,c) sha3_process(a,b,c)
i32 sha3_shake_done(hash_state *md, u8 *out, u64 outlen);
i32 sha3_shake_test(void);
i32 sha3_shake_memory(i32 num, u8k *in, u64 inlen, u8 *out, u64 *outlen);
#endif

#ifdef LTC_SHA512
i32 sha512_init(hash_state * md);
i32 sha512_process(hash_state * md, u8k *in, u64 inlen);
i32 sha512_done(hash_state * md, u8 *hash);
i32 sha512_test(void);
extern const struct ltc_hash_descriptor sha512_desc;
#endif

#ifdef LTC_SHA384
#ifndef LTC_SHA512
   #error LTC_SHA512 is required for LTC_SHA384
#endif
i32 sha384_init(hash_state * md);
#define sha384_process sha512_process
i32 sha384_done(hash_state * md, u8 *hash);
i32 sha384_test(void);
extern const struct ltc_hash_descriptor sha384_desc;
#endif

#ifdef LTC_SHA512_256
#ifndef LTC_SHA512
   #error LTC_SHA512 is required for LTC_SHA512_256
#endif
i32 sha512_256_init(hash_state * md);
#define sha512_256_process sha512_process
i32 sha512_256_done(hash_state * md, u8 *hash);
i32 sha512_256_test(void);
extern const struct ltc_hash_descriptor sha512_256_desc;
#endif

#ifdef LTC_SHA512_224
#ifndef LTC_SHA512
   #error LTC_SHA512 is required for LTC_SHA512_224
#endif
i32 sha512_224_init(hash_state * md);
#define sha512_224_process sha512_process
i32 sha512_224_done(hash_state * md, u8 *hash);
i32 sha512_224_test(void);
extern const struct ltc_hash_descriptor sha512_224_desc;
#endif

#ifdef LTC_SHA256
i32 sha256_init(hash_state * md);
i32 sha256_process(hash_state * md, u8k *in, u64 inlen);
i32 sha256_done(hash_state * md, u8 *hash);
i32 sha256_test(void);
extern const struct ltc_hash_descriptor sha256_desc;

#ifdef LTC_SHA224
#ifndef LTC_SHA256
   #error LTC_SHA256 is required for LTC_SHA224
#endif
i32 sha224_init(hash_state * md);
#define sha224_process sha256_process
i32 sha224_done(hash_state * md, u8 *hash);
i32 sha224_test(void);
extern const struct ltc_hash_descriptor sha224_desc;
#endif
#endif

#ifdef LTC_SHA1
i32 sha1_init(hash_state * md);
i32 sha1_process(hash_state * md, u8k *in, u64 inlen);
i32 sha1_done(hash_state * md, u8 *hash);
i32 sha1_test(void);
extern const struct ltc_hash_descriptor sha1_desc;
#endif

#ifdef LTC_BLAKE2S
extern const struct ltc_hash_descriptor blake2s_256_desc;
i32 blake2s_256_init(hash_state * md);
i32 blake2s_256_test(void);

extern const struct ltc_hash_descriptor blake2s_224_desc;
i32 blake2s_224_init(hash_state * md);
i32 blake2s_224_test(void);

extern const struct ltc_hash_descriptor blake2s_160_desc;
i32 blake2s_160_init(hash_state * md);
i32 blake2s_160_test(void);

extern const struct ltc_hash_descriptor blake2s_128_desc;
i32 blake2s_128_init(hash_state * md);
i32 blake2s_128_test(void);

i32 blake2s_init(hash_state * md, u64 outlen, u8k *key, u64 keylen);
i32 blake2s_process(hash_state * md, u8k *in, u64 inlen);
i32 blake2s_done(hash_state * md, u8 *hash);
#endif

#ifdef LTC_BLAKE2B
extern const struct ltc_hash_descriptor blake2b_512_desc;
i32 blake2b_512_init(hash_state * md);
i32 blake2b_512_test(void);

extern const struct ltc_hash_descriptor blake2b_384_desc;
i32 blake2b_384_init(hash_state * md);
i32 blake2b_384_test(void);

extern const struct ltc_hash_descriptor blake2b_256_desc;
i32 blake2b_256_init(hash_state * md);
i32 blake2b_256_test(void);

extern const struct ltc_hash_descriptor blake2b_160_desc;
i32 blake2b_160_init(hash_state * md);
i32 blake2b_160_test(void);

i32 blake2b_init(hash_state * md, u64 outlen, u8k *key, u64 keylen);
i32 blake2b_process(hash_state * md, u8k *in, u64 inlen);
i32 blake2b_done(hash_state * md, u8 *hash);
#endif

#ifdef LTC_MD5
i32 md5_init(hash_state * md);
i32 md5_process(hash_state * md, u8k *in, u64 inlen);
i32 md5_done(hash_state * md, u8 *hash);
i32 md5_test(void);
extern const struct ltc_hash_descriptor md5_desc;
#endif

#ifdef LTC_MD4
i32 md4_init(hash_state * md);
i32 md4_process(hash_state * md, u8k *in, u64 inlen);
i32 md4_done(hash_state * md, u8 *hash);
i32 md4_test(void);
extern const struct ltc_hash_descriptor md4_desc;
#endif

#ifdef LTC_MD2
i32 md2_init(hash_state * md);
i32 md2_process(hash_state * md, u8k *in, u64 inlen);
i32 md2_done(hash_state * md, u8 *hash);
i32 md2_test(void);
extern const struct ltc_hash_descriptor md2_desc;
#endif

#ifdef LTC_TIGER
i32 tiger_init(hash_state * md);
i32 tiger_process(hash_state * md, u8k *in, u64 inlen);
i32 tiger_done(hash_state * md, u8 *hash);
i32 tiger_test(void);
extern const struct ltc_hash_descriptor tiger_desc;
#endif

#ifdef LTC_RIPEMD128
i32 rmd128_init(hash_state * md);
i32 rmd128_process(hash_state * md, u8k *in, u64 inlen);
i32 rmd128_done(hash_state * md, u8 *hash);
i32 rmd128_test(void);
extern const struct ltc_hash_descriptor rmd128_desc;
#endif

#ifdef LTC_RIPEMD160
i32 rmd160_init(hash_state * md);
i32 rmd160_process(hash_state * md, u8k *in, u64 inlen);
i32 rmd160_done(hash_state * md, u8 *hash);
i32 rmd160_test(void);
extern const struct ltc_hash_descriptor rmd160_desc;
#endif

#ifdef LTC_RIPEMD256
i32 rmd256_init(hash_state * md);
i32 rmd256_process(hash_state * md, u8k *in, u64 inlen);
i32 rmd256_done(hash_state * md, u8 *hash);
i32 rmd256_test(void);
extern const struct ltc_hash_descriptor rmd256_desc;
#endif

#ifdef LTC_RIPEMD320
i32 rmd320_init(hash_state * md);
i32 rmd320_process(hash_state * md, u8k *in, u64 inlen);
i32 rmd320_done(hash_state * md, u8 *hash);
i32 rmd320_test(void);
extern const struct ltc_hash_descriptor rmd320_desc;
#endif


i32 find_hash(tukk name);
i32 find_hash_id(u8 ID);
i32 find_hash_oid(const u64 *ID, u64 IDlen);
i32 find_hash_any(tukk name, i32 digestlen);
i32 register_hash(const struct ltc_hash_descriptor *hash);
i32 unregister_hash(const struct ltc_hash_descriptor *hash);
i32 register_all_hashes(void);
i32 hash_is_valid(i32 idx);

LTC_MUTEX_PROTO(ltc_hash_mutex)

i32 hash_memory(i32 hash,
                u8k *in,  u64 inlen,
                      u8 *out, u64 *outlen);
i32 hash_memory_multi(i32 hash, u8 *out, u64 *outlen,
                      u8k *in, u64 inlen, ...);

#ifndef LTC_NO_FILE
i32 hash_filehandle(i32 hash, FILE *in, u8 *out, u64 *outlen);
i32 hash_file(i32 hash, tukk fname, u8 *out, u64 *outlen);
#endif

/* a simple macro for making hash "process" functions */
#define HASH_PROCESS(func_name, compress_name, state_var, block_size)                       \
i32 func_name (hash_state * md, u8k *in, u64 inlen)               \
{                                                                                           \
    u64 n;                                                                        \
    i32           err;                                                                      \
    LTC_ARGCHK(md != NULL);                                                                 \
    LTC_ARGCHK(in != NULL);                                                                 \
    if (md-> state_var .curlen > sizeof(md-> state_var .buf)) {                             \
       return CRYPT_INVALID_ARG;                                                            \
    }                                                                                       \
    if ((md-> state_var .length + inlen) < md-> state_var .length) {                        \
      return CRYPT_HASH_OVERFLOW;                                                           \
    }                                                                                       \
    while (inlen > 0) {                                                                     \
        if (md-> state_var .curlen == 0 && inlen >= block_size) {                           \
           if ((err = compress_name (md, (u8*)in)) != CRYPT_OK) {               \
              return err;                                                                   \
           }                                                                                \
           md-> state_var .length += block_size * 8;                                        \
           in             += block_size;                                                    \
           inlen          -= block_size;                                                    \
        } else {                                                                            \
           n = MIN(inlen, (block_size - md-> state_var .curlen));                           \
           XMEMCPY(md-> state_var .buf + md-> state_var.curlen, in, (size_t)n);             \
           md-> state_var .curlen += n;                                                     \
           in             += n;                                                             \
           inlen          -= n;                                                             \
           if (md-> state_var .curlen == block_size) {                                      \
              if ((err = compress_name (md, md-> state_var .buf)) != CRYPT_OK) {            \
                 return err;                                                                \
              }                                                                             \
              md-> state_var .length += 8*block_size;                                       \
              md-> state_var .curlen = 0;                                                   \
           }                                                                                \
       }                                                                                    \
    }                                                                                       \
    return CRYPT_OK;                                                                        \
}

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
