/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#ifdef LTC_HMAC
typedef struct Hmac_state {
     hash_state     md;
     i32            hash;
     hash_state     hashstate;
     u8  *key;
} hmac_state;

i32 hmac_init(hmac_state *hmac, i32 hash, u8k *key, u64 keylen);
i32 hmac_process(hmac_state *hmac, u8k *in, u64 inlen);
i32 hmac_done(hmac_state *hmac, u8 *out, u64 *outlen);
i32 hmac_test(void);
i32 hmac_memory(i32 hash,
                u8k *key, u64 keylen,
                u8k *in,  u64 inlen,
                      u8 *out, u64 *outlen);
i32 hmac_memory_multi(i32 hash,
                u8k *key,  u64 keylen,
                      u8 *out,  u64 *outlen,
                u8k *in,   u64 inlen, ...);
i32 hmac_file(i32 hash, tukk fname, u8k *key,
              u64 keylen,
              u8 *dst, u64 *dstlen);
#endif

#ifdef LTC_OMAC

typedef struct {
   i32             cipher_idx,
                   buflen,
                   blklen;
   u8   block[MAXBLOCKSIZE],
                   prev[MAXBLOCKSIZE],
                   Lu[2][MAXBLOCKSIZE];
   symmetric_key   key;
} omac_state;

i32 omac_init(omac_state *omac, i32 cipher, u8k *key, u64 keylen);
i32 omac_process(omac_state *omac, u8k *in, u64 inlen);
i32 omac_done(omac_state *omac, u8 *out, u64 *outlen);
i32 omac_memory(i32 cipher,
               u8k *key, u64 keylen,
               u8k *in,  u64 inlen,
                     u8 *out, u64 *outlen);
i32 omac_memory_multi(i32 cipher,
                u8k *key, u64 keylen,
                      u8 *out, u64 *outlen,
                u8k *in,  u64 inlen, ...);
i32 omac_file(i32 cipher,
              u8k *key, u64 keylen,
              const          char *filename,
                    u8 *out, u64 *outlen);
i32 omac_test(void);
#endif /* LTC_OMAC */

#ifdef LTC_PMAC

typedef struct {
   u8     Ls[32][MAXBLOCKSIZE],    /* L shifted by i bits to the left */
                     Li[MAXBLOCKSIZE],        /* value of Li [current value, we calc from previous recall] */
                     Lr[MAXBLOCKSIZE],        /* L * x^-1 */
                     block[MAXBLOCKSIZE],     /* currently accumulated block */
                     checksum[MAXBLOCKSIZE];  /* current checksum */

   symmetric_key     key;                     /* scheduled key for cipher */
   u64     block_index;             /* index # for current block */
   i32               cipher_idx,              /* cipher idx */
                     block_len,               /* length of block */
                     buflen;                  /* number of bytes in the buffer */
} pmac_state;

i32 pmac_init(pmac_state *pmac, i32 cipher, u8k *key, u64 keylen);
i32 pmac_process(pmac_state *pmac, u8k *in, u64 inlen);
i32 pmac_done(pmac_state *pmac, u8 *out, u64 *outlen);

i32 pmac_memory(i32 cipher,
               u8k *key, u64 keylen,
               u8k *msg, u64 msglen,
                     u8 *out, u64 *outlen);

i32 pmac_memory_multi(i32 cipher,
                u8k *key, u64 keylen,
                      u8 *out, u64 *outlen,
                u8k *in, u64 inlen, ...);

i32 pmac_file(i32 cipher,
             u8k *key, u64 keylen,
             const          char *filename,
                   u8 *out, u64 *outlen);

i32 pmac_test(void);

/* internal functions */
i32 pmac_ntz(u64 x);
void pmac_shift_xor(pmac_state *pmac);

#endif /* PMAC */

#ifdef LTC_POLY1305
typedef struct {
   ulong32 r[5];
   ulong32 h[5];
   ulong32 pad[4];
   u64 leftover;
   u8 buffer[16];
   i32 final;
} poly1305_state;

i32 poly1305_init(poly1305_state *st, u8k *key, u64 keylen);
i32 poly1305_process(poly1305_state *st, u8k *in, u64 inlen);
i32 poly1305_done(poly1305_state *st, u8 *mac, u64 *maclen);
i32 poly1305_memory(u8k *key, u64 keylen, u8k *in, u64 inlen, u8 *mac, u64 *maclen);
i32 poly1305_memory_multi(u8k *key, u64 keylen, u8 *mac, u64 *maclen, u8k *in,  u64 inlen, ...);
i32 poly1305_file(tukk fname, u8k *key, u64 keylen, u8 *mac, u64 *maclen);
i32 poly1305_test(void);
#endif /* LTC_POLY1305 */

#ifdef LTC_BLAKE2SMAC
typedef hash_state blake2smac_state;
i32 blake2smac_init(blake2smac_state *st, u64 outlen, u8k *key, u64 keylen);
i32 blake2smac_process(blake2smac_state *st, u8k *in, u64 inlen);
i32 blake2smac_done(blake2smac_state *st, u8 *mac, u64 *maclen);
i32 blake2smac_memory(u8k *key, u64 keylen, u8k *in, u64 inlen, u8 *mac, u64 *maclen);
i32 blake2smac_memory_multi(u8k *key, u64 keylen, u8 *mac, u64 *maclen, u8k *in,  u64 inlen, ...);
i32 blake2smac_file(tukk fname, u8k *key, u64 keylen, u8 *mac, u64 *maclen);
i32 blake2smac_test(void);
#endif /* LTC_BLAKE2SMAC */

#ifdef LTC_BLAKE2BMAC
typedef hash_state blake2bmac_state;
i32 blake2bmac_init(blake2bmac_state *st, u64 outlen, u8k *key, u64 keylen);
i32 blake2bmac_process(blake2bmac_state *st, u8k *in, u64 inlen);
i32 blake2bmac_done(blake2bmac_state *st, u8 *mac, u64 *maclen);
i32 blake2bmac_memory(u8k *key, u64 keylen, u8k *in, u64 inlen, u8 *mac, u64 *maclen);
i32 blake2bmac_memory_multi(u8k *key, u64 keylen, u8 *mac, u64 *maclen, u8k *in,  u64 inlen, ...);
i32 blake2bmac_file(tukk fname, u8k *key, u64 keylen, u8 *mac, u64 *maclen);
i32 blake2bmac_test(void);
#endif /* LTC_BLAKE2BMAC */

#ifdef LTC_EAX_MODE

#if !(defined(LTC_OMAC) && defined(LTC_CTR_MODE))
   #error LTC_EAX_MODE requires LTC_OMAC and CTR
#endif

typedef struct {
   u8 N[MAXBLOCKSIZE];
   symmetric_CTR ctr;
   omac_state    headeromac, ctomac;
} eax_state;

i32 eax_init(eax_state *eax, i32 cipher, u8k *key, u64 keylen,
             u8k *nonce, u64 noncelen,
             u8k *header, u64 headerlen);

i32 eax_encrypt(eax_state *eax, u8k *pt, u8 *ct, u64 length);
i32 eax_decrypt(eax_state *eax, u8k *ct, u8 *pt, u64 length);
i32 eax_addheader(eax_state *eax, u8k *header, u64 length);
i32 eax_done(eax_state *eax, u8 *tag, u64 *taglen);

i32 eax_encrypt_authenticate_memory(i32 cipher,
    u8k *key,    u64 keylen,
    u8k *nonce,  u64 noncelen,
    u8k *header, u64 headerlen,
    u8k *pt,     u64 ptlen,
          u8 *ct,
          u8 *tag,    u64 *taglen);

i32 eax_decrypt_verify_memory(i32 cipher,
    u8k *key,    u64 keylen,
    u8k *nonce,  u64 noncelen,
    u8k *header, u64 headerlen,
    u8k *ct,     u64 ctlen,
          u8 *pt,
          u8 *tag,    u64 taglen,
          i32           *stat);

 i32 eax_test(void);
#endif /* EAX MODE */

#ifdef LTC_OCB_MODE
typedef struct {
   u8     L[MAXBLOCKSIZE],         /* L value */
                     Ls[32][MAXBLOCKSIZE],    /* L shifted by i bits to the left */
                     Li[MAXBLOCKSIZE],        /* value of Li [current value, we calc from previous recall] */
                     Lr[MAXBLOCKSIZE],        /* L * x^-1 */
                     R[MAXBLOCKSIZE],         /* R value */
                     checksum[MAXBLOCKSIZE];  /* current checksum */

   symmetric_key     key;                     /* scheduled key for cipher */
   u64     block_index;             /* index # for current block */
   i32               cipher,                  /* cipher idx */
                     block_len;               /* length of block */
} ocb_state;

i32 ocb_init(ocb_state *ocb, i32 cipher,
             u8k *key, u64 keylen, u8k *nonce);

i32 ocb_encrypt(ocb_state *ocb, u8k *pt, u8 *ct);
i32 ocb_decrypt(ocb_state *ocb, u8k *ct, u8 *pt);

i32 ocb_done_encrypt(ocb_state *ocb,
                     u8k *pt,  u64 ptlen,
                           u8 *ct,
                           u8 *tag, u64 *taglen);

i32 ocb_done_decrypt(ocb_state *ocb,
                     u8k *ct,  u64 ctlen,
                           u8 *pt,
                     u8k *tag, u64 taglen, i32 *stat);

i32 ocb_encrypt_authenticate_memory(i32 cipher,
    u8k *key,    u64 keylen,
    u8k *nonce,
    u8k *pt,     u64 ptlen,
          u8 *ct,
          u8 *tag,    u64 *taglen);

i32 ocb_decrypt_verify_memory(i32 cipher,
    u8k *key,    u64 keylen,
    u8k *nonce,
    u8k *ct,     u64 ctlen,
          u8 *pt,
    u8k *tag,    u64 taglen,
          i32           *stat);

i32 ocb_test(void);

/* internal functions */
void ocb_shift_xor(ocb_state *ocb, u8 *Z);
i32 ocb_ntz(u64 x);
i32 s_ocb_done(ocb_state *ocb, u8k *pt, u64 ptlen,
               u8 *ct, u8 *tag, u64 *taglen, i32 mode);

#endif /* LTC_OCB_MODE */

#ifdef LTC_OCB3_MODE
typedef struct {
   u8     Offset_0[MAXBLOCKSIZE],       /* Offset_0 value */
                     Offset_current[MAXBLOCKSIZE], /* Offset_{current_block_index} value */
                     L_dollar[MAXBLOCKSIZE],       /* L_$ value */
                     L_star[MAXBLOCKSIZE],         /* L_* value */
                     L_[32][MAXBLOCKSIZE],         /* L_{i} values */
                     tag_part[MAXBLOCKSIZE],       /* intermediate result of tag calculation */
                     checksum[MAXBLOCKSIZE];       /* current checksum */

   /* AAD related members */
   u8     aSum_current[MAXBLOCKSIZE],    /* AAD related helper variable */
                     aOffset_current[MAXBLOCKSIZE], /* AAD related helper variable */
                     adata_buffer[MAXBLOCKSIZE];    /* AAD buffer */
   i32               adata_buffer_bytes;            /* bytes in AAD buffer */
   u64     ablock_index;                  /* index # for current adata (AAD) block */

   symmetric_key     key;                     /* scheduled key for cipher */
   u64     block_index;             /* index # for current data block */
   i32               cipher,                  /* cipher idx */
                     tag_len,                 /* length of tag */
                     block_len;               /* length of block */
} ocb3_state;

i32 ocb3_init(ocb3_state *ocb, i32 cipher,
             u8k *key, u64 keylen,
             u8k *nonce, u64 noncelen,
             u64 taglen);

i32 ocb3_encrypt(ocb3_state *ocb, u8k *pt, u64 ptlen, u8 *ct);
i32 ocb3_decrypt(ocb3_state *ocb, u8k *ct, u64 ctlen, u8 *pt);
i32 ocb3_encrypt_last(ocb3_state *ocb, u8k *pt, u64 ptlen, u8 *ct);
i32 ocb3_decrypt_last(ocb3_state *ocb, u8k *ct, u64 ctlen, u8 *pt);
i32 ocb3_add_aad(ocb3_state *ocb, u8k *aad, u64 aadlen);
i32 ocb3_done(ocb3_state *ocb, u8 *tag, u64 *taglen);

i32 ocb3_encrypt_authenticate_memory(i32 cipher,
    u8k *key,    u64 keylen,
    u8k *nonce,  u64 noncelen,
    u8k *adata,  u64 adatalen,
    u8k *pt,     u64 ptlen,
          u8 *ct,
          u8 *tag,    u64 *taglen);

i32 ocb3_decrypt_verify_memory(i32 cipher,
    u8k *key,    u64 keylen,
    u8k *nonce,  u64 noncelen,
    u8k *adata,  u64 adatalen,
    u8k *ct,     u64 ctlen,
          u8 *pt,
    u8k *tag,    u64 taglen,
          i32           *stat);

i32 ocb3_test(void);

//#ifdef LTC_SOURCE
/* internal helper functions */
i32 ocb3_int_ntz(u64 x);
void ocb3_int_xor_blocks(u8 *out, u8k *block_a, u8k *block_b, u64 block_len);
//#endif /* LTC_SOURCE */

#endif /* LTC_OCB3_MODE */

#ifdef LTC_CCM_MODE

#define CCM_ENCRYPT LTC_ENCRYPT
#define CCM_DECRYPT LTC_DECRYPT

typedef struct {
   symmetric_key       K;
   i32                 cipher,               /* which cipher */
                       taglen,               /* length of the tag */
                       x;                    /* index in PAD */

   u64       L,                    /* L value */
                       ptlen,                /* length that will be enc / dec */
                       current_ptlen,        /* current processed length */
                       aadlen,               /* length of the aad */
                       current_aadlen,       /* length of the currently provided add */
                       noncelen;             /* length of the nonce */

   u8       PAD[16],
                       ctr[16],
                       CTRPAD[16],
                       CTRlen;
} ccm_state;

i32 ccm_init(ccm_state *ccm, i32 cipher,
             u8k *key, i32 keylen, i32 ptlen, i32 taglen, i32 aad_len);

i32 ccm_reset(ccm_state *ccm);

i32 ccm_add_nonce(ccm_state *ccm,
                  u8k *nonce,     u64 noncelen);

i32 ccm_add_aad(ccm_state *ccm,
                u8k *adata,  u64 adatalen);

i32 ccm_process(ccm_state *ccm,
                u8 *pt,     u64 ptlen,
                u8 *ct,
                i32 direction);

i32 ccm_done(ccm_state *ccm,
             u8 *tag,    u64 *taglen);

i32 ccm_memory(i32 cipher,
    u8k *key,    u64 keylen,
    symmetric_key       *uskey,
    u8k *nonce,  u64 noncelen,
    u8k *header, u64 headerlen,
          u8 *pt,     u64 ptlen,
          u8 *ct,
          u8 *tag,    u64 *taglen,
                    i32  direction);

i32 ccm_test(void);

#endif /* LTC_CCM_MODE */

#if defined(LRW_MODE) || defined(LTC_GCM_MODE)
void gcm_gf_mult(u8k *a, u8k *b, u8 *c);
#endif


/* table shared between GCM and LRW */
#if defined(LTC_GCM_TABLES) || defined(LTC_LRW_TABLES) || ((defined(LTC_GCM_MODE) || defined(LTC_GCM_MODE)) && defined(LTC_FAST))
extern u8k gcm_shift_table[];
#endif

#ifdef LTC_GCM_MODE

#define GCM_ENCRYPT LTC_ENCRYPT
#define GCM_DECRYPT LTC_DECRYPT

#define LTC_GCM_MODE_IV    0
#define LTC_GCM_MODE_AAD   1
#define LTC_GCM_MODE_TEXT  2

typedef struct {
   symmetric_key       K;
   u8       H[16],        /* multiplier */
                       X[16],        /* accumulator */
                       Y[16],        /* counter */
                       Y_0[16],      /* initial counter */
                       buf[16];      /* buffer for stuff */

   i32                 cipher,       /* which cipher */
                       ivmode,       /* Which mode is the IV in? */
                       mode,         /* mode the GCM code is in */
                       buflen;       /* length of data in buf */

   ulong64             totlen,       /* 64-bit counter used for IV and AAD */
                       pttotlen;     /* 64-bit counter for the PT */

#ifdef LTC_GCM_TABLES
   u8       PC[16][256][16]  /* 16 tables of 8x128 */
#ifdef LTC_GCM_TABLES_SSE2
__attribute__ ((aligned (16)))
#endif
;
#endif
} gcm_state;

void gcm_mult_h(gcm_state *gcm, u8 *I);

i32 gcm_init(gcm_state *gcm, i32 cipher,
             u8k *key, i32 keylen);

i32 gcm_reset(gcm_state *gcm);

i32 gcm_add_iv(gcm_state *gcm,
               u8k *IV,     u64 IVlen);

i32 gcm_add_aad(gcm_state *gcm,
               u8k *adata,  u64 adatalen);

i32 gcm_process(gcm_state *gcm,
                     u8 *pt,     u64 ptlen,
                     u8 *ct,
                     i32 direction);

i32 gcm_done(gcm_state *gcm,
                     u8 *tag,    u64 *taglen);

i32 gcm_memory(      i32           cipher,
               u8k *key,    u64 keylen,
               u8k *IV,     u64 IVlen,
               u8k *adata,  u64 adatalen,
                     u8 *pt,     u64 ptlen,
                     u8 *ct,
                     u8 *tag,    u64 *taglen,
                               i32 direction);
i32 gcm_test(void);

#endif /* LTC_GCM_MODE */

#ifdef LTC_PELICAN

typedef struct pelican_state
{
    symmetric_key K;
    u8 state[16];
    i32           buflen;
} pelican_state;

i32 pelican_init(pelican_state *pelmac, u8k *key, u64 keylen);
i32 pelican_process(pelican_state *pelmac, u8k *in, u64 inlen);
i32 pelican_done(pelican_state *pelmac, u8 *out);
i32 pelican_test(void);

i32 pelican_memory(u8k *key, u64 keylen,
                   u8k *in, u64 inlen,
                         u8 *out);

#endif

#ifdef LTC_XCBC

/* add this to "keylen" to xcbc_init to use a pure three-key XCBC MAC */
#define LTC_XCBC_PURE  0x8000UL

typedef struct {
   u8 K[3][MAXBLOCKSIZE],
                 IV[MAXBLOCKSIZE];

   symmetric_key key;

             i32 cipher,
                 buflen,
                 blocksize;
} xcbc_state;

i32 xcbc_init(xcbc_state *xcbc, i32 cipher, u8k *key, u64 keylen);
i32 xcbc_process(xcbc_state *xcbc, u8k *in, u64 inlen);
i32 xcbc_done(xcbc_state *xcbc, u8 *out, u64 *outlen);
i32 xcbc_memory(i32 cipher,
               u8k *key, u64 keylen,
               u8k *in,  u64 inlen,
                     u8 *out, u64 *outlen);
i32 xcbc_memory_multi(i32 cipher,
                u8k *key, u64 keylen,
                      u8 *out, u64 *outlen,
                u8k *in,  u64 inlen, ...);
i32 xcbc_file(i32 cipher,
              u8k *key, u64 keylen,
              const          char *filename,
                    u8 *out, u64 *outlen);
i32 xcbc_test(void);

#endif

#ifdef LTC_F9_MODE

typedef struct {
   u8 akey[MAXBLOCKSIZE],
                 ACC[MAXBLOCKSIZE],
                 IV[MAXBLOCKSIZE];

   symmetric_key key;

             i32 cipher,
                 buflen,
                 keylen,
                 blocksize;
} f9_state;

i32 f9_init(f9_state *f9, i32 cipher, u8k *key, u64 keylen);
i32 f9_process(f9_state *f9, u8k *in, u64 inlen);
i32 f9_done(f9_state *f9, u8 *out, u64 *outlen);
i32 f9_memory(i32 cipher,
               u8k *key, u64 keylen,
               u8k *in,  u64 inlen,
                     u8 *out, u64 *outlen);
i32 f9_memory_multi(i32 cipher,
                u8k *key, u64 keylen,
                      u8 *out, u64 *outlen,
                u8k *in,  u64 inlen, ...);
i32 f9_file(i32 cipher,
              u8k *key, u64 keylen,
              const          char *filename,
                    u8 *out, u64 *outlen);
i32 f9_test(void);

#endif

#ifdef LTC_CHACHA20POLY1305_MODE

typedef struct {
   poly1305_state poly;
   chacha_state chacha;
   ulong64 aadlen;
   ulong64 ctlen;
   i32 aadflg;
} chacha20poly1305_state;

#define CHACHA20POLY1305_ENCRYPT LTC_ENCRYPT
#define CHACHA20POLY1305_DECRYPT LTC_DECRYPT

i32 chacha20poly1305_init(chacha20poly1305_state *st, u8k *key, u64 keylen);
i32 chacha20poly1305_setiv(chacha20poly1305_state *st, u8k *iv, u64 ivlen);
i32 chacha20poly1305_setiv_rfc7905(chacha20poly1305_state *st, u8k *iv, u64 ivlen, ulong64 sequence_number);
i32 chacha20poly1305_add_aad(chacha20poly1305_state *st, u8k *in, u64 inlen);
i32 chacha20poly1305_encrypt(chacha20poly1305_state *st, u8k *in, u64 inlen, u8 *out);
i32 chacha20poly1305_decrypt(chacha20poly1305_state *st, u8k *in, u64 inlen, u8 *out);
i32 chacha20poly1305_done(chacha20poly1305_state *st, u8 *tag, u64 *taglen);
i32 chacha20poly1305_memory(u8k *key, u64 keylen,
                            u8k *iv,  u64 ivlen,
                            u8k *aad, u64 aadlen,
                            u8k *in,  u64 inlen,
                                  u8 *out,
                                  u8 *tag, u64 *taglen,
                            i32 direction);
i32 chacha20poly1305_test(void);

#endif /* LTC_CHACHA20POLY1305_MODE */

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
