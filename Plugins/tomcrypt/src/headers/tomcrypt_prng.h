/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* ---- PRNG Stuff ---- */
#ifdef LTC_YARROW
struct yarrow_prng {
    i32                   cipher, hash;
    u8         pool[MAXBLOCKSIZE];
    symmetric_CTR         ctr;
};
#endif

#ifdef LTC_RC4
struct rc4_prng {
    rc4_state s;
};
#endif

#ifdef LTC_CHACHA20_PRNG
struct chacha20_prng {
    chacha_state s;        /* chacha state */
    u8 ent[40]; /* entropy buffer */
    u64 idx;     /* entropy counter */
};
#endif

#ifdef LTC_FORTUNA
struct fortuna_prng {
    hash_state pool[LTC_FORTUNA_POOLS];     /* the  pools */

    symmetric_key skey;

    u8 K[32],      /* the current key */
                  IV[16];     /* IV for CTR mode */

    u64 pool_idx,   /* current pool we will add to */
                  pool0_len,  /* length of 0'th pool */
                  wd;

    ulong64       reset_cnt;  /* number of times we have reset */
};
#endif

#ifdef LTC_SOBER128
struct sober128_prng {
    sober128_state s;      /* sober128 state */
    u8 ent[40]; /* entropy buffer */
    u64 idx;     /* entropy counter */
};
#endif

typedef struct {
   union {
      char dummy[1];
#ifdef LTC_YARROW
      struct yarrow_prng    yarrow;
#endif
#ifdef LTC_RC4
      struct rc4_prng       rc4;
#endif
#ifdef LTC_CHACHA20_PRNG
      struct chacha20_prng  chacha;
#endif
#ifdef LTC_FORTUNA
      struct fortuna_prng   fortuna;
#endif
#ifdef LTC_SOBER128
      struct sober128_prng  sober128;
#endif
   };
   short ready;            /* ready flag 0-1 */
   LTC_MUTEX_TYPE(lock)    /* lock */
} prng_state;

/** PRNG descriptor */
extern struct ltc_prng_descriptor {
    /** Name of the PRNG */
    tukk name;
    /** size in bytes of exported state */
    i32  export_size;
    /** Start a PRNG state
        @param prng   [out] The state to initialize
        @return CRYPT_OK if successful
    */
    i32 (*start)(prng_state *prng);
    /** Add entropy to the PRNG
        @param in         The entropy
        @param inlen      Length of the entropy (octets)\
        @param prng       The PRNG state
        @return CRYPT_OK if successful
    */
    i32 (*add_entropy)(u8k *in, u64 inlen, prng_state *prng);
    /** Ready a PRNG state to read from
        @param prng       The PRNG state to ready
        @return CRYPT_OK if successful
    */
    i32 (*ready)(prng_state *prng);
    /** Read from the PRNG
        @param out     [out] Where to store the data
        @param outlen  Length of data desired (octets)
        @param prng    The PRNG state to read from
        @return Number of octets read
    */
    u64 (*read)(u8 *out, u64 outlen, prng_state *prng);
    /** Terminate a PRNG state
        @param prng   The PRNG state to terminate
        @return CRYPT_OK if successful
    */
    i32 (*done)(prng_state *prng);
    /** Export a PRNG state
        @param out     [out] The destination for the state
        @param outlen  [in/out] The max size and resulting size of the PRNG state
        @param prng    The PRNG to export
        @return CRYPT_OK if successful
    */
    i32 (*pexport)(u8 *out, u64 *outlen, prng_state *prng);
    /** Import a PRNG state
        @param in      The data to import
        @param inlen   The length of the data to import (octets)
        @param prng    The PRNG to initialize/import
        @return CRYPT_OK if successful
    */
    i32 (*pimport)(u8k *in, u64 inlen, prng_state *prng);
    /** Self-test the PRNG
        @return CRYPT_OK if successful, CRYPT_NOP if self-testing has been disabled
    */
    i32 (*test)(void);
} prng_descriptor[];

#ifdef LTC_YARROW
i32 yarrow_start(prng_state *prng);
i32 yarrow_add_entropy(u8k *in, u64 inlen, prng_state *prng);
i32 yarrow_ready(prng_state *prng);
u64 yarrow_read(u8 *out, u64 outlen, prng_state *prng);
i32 yarrow_done(prng_state *prng);
i32  yarrow_export(u8 *out, u64 *outlen, prng_state *prng);
i32  yarrow_import(u8k *in, u64 inlen, prng_state *prng);
i32  yarrow_test(void);
extern const struct ltc_prng_descriptor yarrow_desc;
#endif

#ifdef LTC_FORTUNA
i32 fortuna_start(prng_state *prng);
i32 fortuna_add_entropy(u8k *in, u64 inlen, prng_state *prng);
i32 fortuna_ready(prng_state *prng);
u64 fortuna_read(u8 *out, u64 outlen, prng_state *prng);
i32 fortuna_done(prng_state *prng);
i32  fortuna_export(u8 *out, u64 *outlen, prng_state *prng);
i32  fortuna_import(u8k *in, u64 inlen, prng_state *prng);
i32  fortuna_test(void);
extern const struct ltc_prng_descriptor fortuna_desc;
#endif

#ifdef LTC_RC4
i32 rc4_start(prng_state *prng);
i32 rc4_add_entropy(u8k *in, u64 inlen, prng_state *prng);
i32 rc4_ready(prng_state *prng);
u64 rc4_read(u8 *out, u64 outlen, prng_state *prng);
i32  rc4_done(prng_state *prng);
i32  rc4_export(u8 *out, u64 *outlen, prng_state *prng);
i32  rc4_import(u8k *in, u64 inlen, prng_state *prng);
i32  rc4_test(void);
extern const struct ltc_prng_descriptor rc4_desc;
#endif

#ifdef LTC_CHACHA20_PRNG
i32 chacha20_prng_start(prng_state *prng);
i32 chacha20_prng_add_entropy(u8k *in, u64 inlen, prng_state *prng);
i32 chacha20_prng_ready(prng_state *prng);
u64 chacha20_prng_read(u8 *out, u64 outlen, prng_state *prng);
i32  chacha20_prng_done(prng_state *prng);
i32  chacha20_prng_export(u8 *out, u64 *outlen, prng_state *prng);
i32  chacha20_prng_import(u8k *in, u64 inlen, prng_state *prng);
i32  chacha20_prng_test(void);
extern const struct ltc_prng_descriptor chacha20_prng_desc;
#endif

#ifdef LTC_SPRNG
i32 sprng_start(prng_state *prng);
i32 sprng_add_entropy(u8k *in, u64 inlen, prng_state *prng);
i32 sprng_ready(prng_state *prng);
u64 sprng_read(u8 *out, u64 outlen, prng_state *prng);
i32 sprng_done(prng_state *prng);
i32  sprng_export(u8 *out, u64 *outlen, prng_state *prng);
i32  sprng_import(u8k *in, u64 inlen, prng_state *prng);
i32  sprng_test(void);
extern const struct ltc_prng_descriptor sprng_desc;
#endif

#ifdef LTC_SOBER128
i32 sober128_start(prng_state *prng);
i32 sober128_add_entropy(u8k *in, u64 inlen, prng_state *prng);
i32 sober128_ready(prng_state *prng);
u64 sober128_read(u8 *out, u64 outlen, prng_state *prng);
i32 sober128_done(prng_state *prng);
i32  sober128_export(u8 *out, u64 *outlen, prng_state *prng);
i32  sober128_import(u8k *in, u64 inlen, prng_state *prng);
i32  sober128_test(void);
extern const struct ltc_prng_descriptor sober128_desc;
#endif

i32 find_prng(tukk name);
i32 register_prng(const struct ltc_prng_descriptor *prng);
i32 unregister_prng(const struct ltc_prng_descriptor *prng);
i32 register_all_prngs(void);
i32 prng_is_valid(i32 idx);
LTC_MUTEX_PROTO(ltc_prng_mutex)

/* Slow RNG you **might** be able to use to seed a PRNG with.  Be careful as this
 * might not work on all platforms as planned
 */
u64 rng_get_bytes(u8 *out,
                            u64 outlen,
                            void (*callback)(void));

i32 rng_make_prng(i32 bits, i32 wprng, prng_state *prng, void (*callback)(void));

#ifdef LTC_PRNG_ENABLE_LTC_RNG
extern u64 (*ltc_rng)(u8 *out, u64 outlen,
      void (*callback)(void));
#endif


/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
