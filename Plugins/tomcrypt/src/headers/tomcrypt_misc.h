/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* ---- LTC_BASE64 Routines ---- */
#ifdef LTC_BASE64
i32 base64_encode(u8k *in,  u64 len,
                        u8 *out, u64 *outlen);

i32 base64_decode(u8k *in,  u64 len,
                        u8 *out, u64 *outlen);
i32 base64_strict_decode(u8k *in,  u64 len,
                        u8 *out, u64 *outlen);
#endif

#ifdef LTC_BASE64_URL
i32 base64url_encode(u8k *in,  u64 len,
                        u8 *out, u64 *outlen);
i32 base64url_strict_encode(u8k *in,  u64 inlen,
                        u8 *out, u64 *outlen);

i32 base64url_decode(u8k *in,  u64 len,
                        u8 *out, u64 *outlen);
i32 base64url_strict_decode(u8k *in,  u64 len,
                        u8 *out, u64 *outlen);
#endif

/* ===> LTC_HKDF -- RFC5869 HMAC-based Key Derivation Function <=== */
#ifdef LTC_HKDF

i32 hkdf_test(void);

i32 hkdf_extract(i32 hash_idx,
                 u8k *salt, u64 saltlen,
                 u8k *in,   u64 inlen,
                       u8 *out,  u64 *outlen);

i32 hkdf_expand(i32 hash_idx,
                u8k *info, u64 infolen,
                u8k *in,   u64 inlen,
                      u8 *out,  u64 outlen);

i32 hkdf(i32 hash_idx,
         u8k *salt, u64 saltlen,
         u8k *info, u64 infolen,
         u8k *in,   u64 inlen,
               u8 *out,  u64 outlen);

#endif  /* LTC_HKDF */

/* ---- MEM routines ---- */
i32 mem_neq(ukk a, ukk b, size_t len);
void zeromem( uk dst, size_t len);
void burn_stack(u64 len);

tukk error_to_string(i32 err);

extern tukk crypt_build_settings;

/* ---- HMM ---- */
i32 crypt_fsa(uk mp, ...);

/* ---- Dynamic language support ---- */
i32 crypt_get_constant(tukk namein, i32 *valueout);
i32 crypt_list_all_constants(char *names_list, u32 *names_list_size);

i32 crypt_get_size(tukk namein, u32 *sizeout);
i32 crypt_list_all_sizes(char *names_list, u32 *names_list_size);

#ifdef LTM_DESC
void init_LTM(void);
#endif
#ifdef TFM_DESC
void init_TFM(void);
#endif
#ifdef GMP_DESC
void init_GMP(void);
#endif

#ifdef LTC_ADLER32
typedef struct adler32_state_s
{
   unsigned short s[2];
} adler32_state;

void adler32_init(adler32_state *ctx);
void adler32_update(adler32_state *ctx, u8k *input, u64 length);
void adler32_finish(adler32_state *ctx, uk hash, u64 size);
i32 adler32_test(void);
#endif

#ifdef LTC_CRC32
typedef struct crc32_state_s
{
   ulong32 crc;
} crc32_state;

void crc32_init(crc32_state *ctx);
void crc32_update(crc32_state *ctx, u8k *input, u64 length);
void crc32_finish(crc32_state *ctx, uk hash, u64 size);
i32 crc32_test(void);
#endif

i32 compare_testvector(ukk is, const u64 is_len, ukk should, const u64 should_len, tukk what, i32 which);

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
