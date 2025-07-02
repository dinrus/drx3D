/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* PKCS Header Info */

/* ===> PKCS #1 -- RSA Cryptography <=== */
#ifdef LTC_PKCS_1

enum ltc_pkcs_1_v1_5_blocks
{
  LTC_PKCS_1_EMSA   = 1,        /* Block type 1 (PKCS #1 v1.5 signature padding) */
  LTC_PKCS_1_EME    = 2         /* Block type 2 (PKCS #1 v1.5 encryption padding) */
};

enum ltc_pkcs_1_paddings
{
  LTC_PKCS_1_V1_5     = 1,        /* PKCS #1 v1.5 padding (\sa ltc_pkcs_1_v1_5_blocks) */
  LTC_PKCS_1_OAEP     = 2,        /* PKCS #1 v2.0 encryption padding */
  LTC_PKCS_1_PSS      = 3,        /* PKCS #1 v2.1 signature padding */
  LTC_PKCS_1_V1_5_NA1 = 4         /* PKCS #1 v1.5 padding - No ASN.1 (\sa ltc_pkcs_1_v1_5_blocks) */
};

i32 pkcs_1_mgf1(      i32            hash_idx,
                u8k *seed, u64 seedlen,
                      u8 *mask, u64 masklen);

i32 pkcs_1_i2osp(uk n, u64 modulus_len, u8 *out);
i32 pkcs_1_os2ip(uk n, u8 *in, u64 inlen);

/* *** v1.5 padding */
i32 pkcs_1_v1_5_encode(u8k *msg,
                             u64  msglen,
                             i32            block_type,
                             u64  modulus_bitlen,
                                prng_state *prng,
                                       i32  prng_idx,
                             u8 *out,
                             u64 *outlen);

i32 pkcs_1_v1_5_decode(u8k *msg,
                             u64  msglen,
                                       i32  block_type,
                             u64  modulus_bitlen,
                             u8 *out,
                             u64 *outlen,
                                       i32 *is_valid);

/* *** v2.1 padding */
i32 pkcs_1_oaep_encode(u8k *msg,    u64 msglen,
                       u8k *lparam, u64 lparamlen,
                             u64 modulus_bitlen, prng_state *prng,
                             i32           prng_idx,         i32  hash_idx,
                             u8 *out,    u64 *outlen);

i32 pkcs_1_oaep_decode(u8k *msg,    u64 msglen,
                       u8k *lparam, u64 lparamlen,
                             u64 modulus_bitlen, i32 hash_idx,
                             u8 *out,    u64 *outlen,
                             i32           *res);

i32 pkcs_1_pss_encode(u8k *msghash, u64 msghashlen,
                            u64 saltlen,  prng_state   *prng,
                            i32           prng_idx, i32           hash_idx,
                            u64 modulus_bitlen,
                            u8 *out,     u64 *outlen);

i32 pkcs_1_pss_decode(u8k *msghash, u64 msghashlen,
                      u8k *sig,     u64 siglen,
                            u64 saltlen,  i32           hash_idx,
                            u64 modulus_bitlen, i32    *res);

#endif /* LTC_PKCS_1 */

/* ===> PKCS #5 -- Password Based Cryptography <=== */
#ifdef LTC_PKCS_5

/* Algorithm #1 (PBKDF1) */
i32 pkcs_5_alg1(u8k *password, u64 password_len,
                u8k *salt,
                i32 iteration_count,  i32 hash_idx,
                u8 *out,   u64 *outlen);

/* Algorithm #1 (PBKDF1) - OpenSSL-compatible variant for arbitrarily-long keys.
   Compatible with EVP_BytesToKey() */
i32 pkcs_5_alg1_openssl(u8k *password,
                        u64 password_len,
                        u8k *salt,
                        i32 iteration_count,  i32 hash_idx,
                        u8 *out,   u64 *outlen);

/* Algorithm #2 (PBKDF2) */
i32 pkcs_5_alg2(u8k *password, u64 password_len,
                u8k *salt,     u64 salt_len,
                i32 iteration_count,           i32 hash_idx,
                u8 *out,            u64 *outlen);

i32 pkcs_5_test (void);
#endif  /* LTC_PKCS_5 */

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
