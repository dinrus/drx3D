/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

/* ---- NUMBER THEORY ---- */

enum {
   PK_PUBLIC=0,
   PK_PRIVATE=1
};

/* Indicates standard output formats that can be read e.g. by OpenSSL or GnuTLS */
#define PK_STD          0x1000

i32 rand_prime(uk N, long len, prng_state *prng, i32 wprng);

#ifdef LTC_SOURCE
/* internal helper functions */
i32 rand_bn_bits(uk N, i32 bits, prng_state *prng, i32 wprng);
i32 rand_bn_upto(uk N, uk limit, prng_state *prng, i32 wprng);

enum public_key_algorithms {
   PKA_RSA,
   PKA_DSA
};

typedef struct Oid {
    u64 OID[16];
    /** Number of OID digits in use */
    u64 OIDlen;
} oid_st;

i32 pk_get_oid(i32 pk, oid_st *st);
#endif /* LTC_SOURCE */

/* ---- RSA ---- */
#ifdef LTC_MRSA

/** RSA PKCS style key */
typedef struct Rsa_key {
    /** Type of key, PK_PRIVATE or PK_PUBLIC */
    i32 type;
    /** The public exponent */
    uk e;
    /** The private exponent */
    uk d;
    /** The modulus */
    uk N;
    /** The p factor of N */
    uk p;
    /** The q factor of N */
    uk q;
    /** The 1/q mod p CRT param */
    uk qP;
    /** The d mod (p - 1) CRT param */
    uk dP;
    /** The d mod (q - 1) CRT param */
    uk dQ;
} rsa_key;

i32 rsa_make_key(prng_state *prng, i32 wprng, i32 size, long e, rsa_key *key);

i32 rsa_get_size(rsa_key *key);

i32 rsa_exptmod(u8k *in,   u64 inlen,
                      u8 *out,  u64 *outlen, i32 which,
                      rsa_key *key);

void rsa_free(rsa_key *key);

/* These use PKCS #1 v2.0 padding */
#define rsa_encrypt_key(_in, _inlen, _out, _outlen, _lparam, _lparamlen, _prng, _prng_idx, _hash_idx, _key) \
  rsa_encrypt_key_ex(_in, _inlen, _out, _outlen, _lparam, _lparamlen, _prng, _prng_idx, _hash_idx, LTC_PKCS_1_OAEP, _key)

#define rsa_decrypt_key(_in, _inlen, _out, _outlen, _lparam, _lparamlen, _hash_idx, _stat, _key) \
  rsa_decrypt_key_ex(_in, _inlen, _out, _outlen, _lparam, _lparamlen, _hash_idx, LTC_PKCS_1_OAEP, _stat, _key)

#define rsa_sign_hash(_in, _inlen, _out, _outlen, _prng, _prng_idx, _hash_idx, _saltlen, _key) \
  rsa_sign_hash_ex(_in, _inlen, _out, _outlen, LTC_PKCS_1_PSS, _prng, _prng_idx, _hash_idx, _saltlen, _key)

#define rsa_verify_hash(_sig, _siglen, _hash, _hashlen, _hash_idx, _saltlen, _stat, _key) \
  rsa_verify_hash_ex(_sig, _siglen, _hash, _hashlen, LTC_PKCS_1_PSS, _hash_idx, _saltlen, _stat, _key)

#define rsa_sign_saltlen_get_max(_hash_idx, _key) \
  rsa_sign_saltlen_get_max_ex(LTC_PKCS_1_PSS, _hash_idx, _key)

/* These can be switched between PKCS #1 v2.x and PKCS #1 v1.5 paddings */
i32 rsa_encrypt_key_ex(u8k *in,     u64 inlen,
                             u8 *out,    u64 *outlen,
                       u8k *lparam, u64 lparamlen,
                       prng_state *prng, i32 prng_idx, i32 hash_idx, i32 padding, rsa_key *key);

i32 rsa_decrypt_key_ex(u8k *in,       u64  inlen,
                             u8 *out,      u64 *outlen,
                       u8k *lparam,   u64  lparamlen,
                             i32            hash_idx, i32            padding,
                             i32           *stat,     rsa_key       *key);

i32 rsa_sign_hash_ex(u8k *in,       u64  inlen,
                           u8 *out,      u64 *outlen,
                           i32            padding,
                           prng_state    *prng,     i32            prng_idx,
                           i32            hash_idx, u64  saltlen,
                           rsa_key *key);

i32 rsa_verify_hash_ex(u8k *sig,      u64 siglen,
                       u8k *hash,     u64 hashlen,
                             i32            padding,
                             i32            hash_idx, u64 saltlen,
                             i32           *stat,     rsa_key      *key);

i32 rsa_sign_saltlen_get_max_ex(i32 padding, i32 hash_idx, rsa_key *key);

/* PKCS #1 import/export */
i32 rsa_export(u8 *out, u64 *outlen, i32 type, rsa_key *key);
i32 rsa_import(u8k *in, u64 inlen, rsa_key *key);

i32 rsa_import_x509(u8k *in, u64 inlen, rsa_key *key);
i32 rsa_import_pkcs8(u8k *in, u64 inlen,
                     ukk passwd, u64 passwdlen, rsa_key *key);

i32 rsa_set_key(u8k *N,  u64 Nlen,
                u8k *e,  u64 elen,
                u8k *d,  u64 dlen,
                rsa_key *key);
i32 rsa_set_factors(u8k *p,  u64 plen,
                    u8k *q,  u64 qlen,
                    rsa_key *key);
i32 rsa_set_crt_params(u8k *dP, u64 dPlen,
                       u8k *dQ, u64 dQlen,
                       u8k *qP, u64 qPlen,
                       rsa_key *key);
#endif

/* ---- Katja ---- */
#ifdef LTC_MKAT

/* Min and Max KAT key sizes (in bits) */
#define MIN_KAT_SIZE 1024
#define MAX_KAT_SIZE 4096

/** Katja PKCS style key */
typedef struct KAT_key {
    /** Type of key, PK_PRIVATE or PK_PUBLIC */
    i32 type;
    /** The private exponent */
    uk d;
    /** The modulus */
    uk N;
    /** The p factor of N */
    uk p;
    /** The q factor of N */
    uk q;
    /** The 1/q mod p CRT param */
    uk qP;
    /** The d mod (p - 1) CRT param */
    uk dP;
    /** The d mod (q - 1) CRT param */
    uk dQ;
    /** The pq param */
    uk pq;
} katja_key;

i32 katja_make_key(prng_state *prng, i32 wprng, i32 size, katja_key *key);

i32 katja_exptmod(u8k *in,   u64 inlen,
                        u8 *out,  u64 *outlen, i32 which,
                        katja_key *key);

void katja_free(katja_key *key);

/* These use PKCS #1 v2.0 padding */
i32 katja_encrypt_key(u8k *in,     u64 inlen,
                            u8 *out,    u64 *outlen,
                      u8k *lparam, u64 lparamlen,
                      prng_state *prng, i32 prng_idx, i32 hash_idx, katja_key *key);

i32 katja_decrypt_key(u8k *in,       u64 inlen,
                            u8 *out,      u64 *outlen,
                      u8k *lparam,   u64 lparamlen,
                            i32            hash_idx, i32 *stat,
                            katja_key       *key);

/* PKCS #1 import/export */
i32 katja_export(u8 *out, u64 *outlen, i32 type, katja_key *key);
i32 katja_import(u8k *in, u64 inlen, katja_key *key);

#endif

/* ---- DH Routines ---- */
#ifdef LTC_MDH

typedef struct {
    i32 type;
    uk x;
    uk y;
    uk base;
    uk prime;
} dh_key;

i32 dh_get_groupsize(dh_key *key);

i32 dh_export(u8 *out, u64 *outlen, i32 type, dh_key *key);
i32 dh_import(u8k *in, u64 inlen, dh_key *key);

i32 dh_set_pg(u8k *p, u64 plen,
              u8k *g, u64 glen,
              dh_key *key);
i32 dh_set_pg_dhparam(u8k *dhparam, u64 dhparamlen, dh_key *key);
i32 dh_set_pg_groupsize(i32 groupsize, dh_key *key);

i32 dh_set_key(u8k *in, u64 inlen, i32 type, dh_key *key);
i32 dh_generate_key(prng_state *prng, i32 wprng, dh_key *key);

i32 dh_shared_secret(dh_key        *private_key, dh_key        *public_key,
                     u8 *out,         u64 *outlen);

void dh_free(dh_key *key);

i32 dh_export_key(uk out, u64 *outlen, i32 type, dh_key *key);

#ifdef LTC_SOURCE
typedef struct {
  i32 size;
  tukk name, *base, *prime;
} ltc_dh_set_type;

extern const ltc_dh_set_type ltc_dh_sets[];

/* internal helper functions */
i32 dh_check_pubkey(dh_key *key);
#endif

#endif /* LTC_MDH */


/* ---- ECC Routines ---- */
#ifdef LTC_MECC

/* size of our temp buffers for exported keys */
#define ECC_BUF_SIZE 256

/* max private key size */
#define ECC_MAXSIZE  66

/** Structure defines a NIST GF(p) curve */
typedef struct {
   /** The size of the curve in octets */
   i32 size;

   /** name of curve */
   tukk name;

   /** The prime that defines the field the curve is in (encoded in hex) */
   tukk prime;

   /** The fields B param (hex) */
   tukk B;

   /** The order of the curve (hex) */
   tukk order;

   /** The x co-ordinate of the base point on the curve (hex) */
   tukk Gx;

   /** The y co-ordinate of the base point on the curve (hex) */
   tukk Gy;
} ltc_ecc_set_type;

/** A point on a ECC curve, stored in Jacbobian format such that (x,y,z) => (x/z^2, y/z^3, 1) when interpretted as affine */
typedef struct {
    /** The x co-ordinate */
    uk x;

    /** The y co-ordinate */
    uk y;

    /** The z co-ordinate */
    uk z;
} ecc_point;

/** An ECC key */
typedef struct {
    /** Type of key, PK_PRIVATE or PK_PUBLIC */
    i32 type;

    /** Index into the ltc_ecc_sets[] for the parameters of this curve; if -1, then this key is using user supplied curve in dp */
    i32 idx;

    /** pointer to domain parameters; either points to NIST curves (identified by idx >= 0) or user supplied curve */
    const ltc_ecc_set_type *dp;

    /** The public key */
    ecc_point pubkey;

    /** The private key */
    uk k;
} ecc_key;

/** the ECC params provided */
extern const ltc_ecc_set_type ltc_ecc_sets[];

i32  ecc_test(void);
void ecc_sizes(i32 *low, i32 *high);
i32  ecc_get_size(ecc_key *key);

i32  ecc_make_key(prng_state *prng, i32 wprng, i32 keysize, ecc_key *key);
i32  ecc_make_key_ex(prng_state *prng, i32 wprng, ecc_key *key, const ltc_ecc_set_type *dp);
void ecc_free(ecc_key *key);

i32  ecc_export(u8 *out, u64 *outlen, i32 type, ecc_key *key);
i32  ecc_import(u8k *in, u64 inlen, ecc_key *key);
i32  ecc_import_ex(u8k *in, u64 inlen, ecc_key *key, const ltc_ecc_set_type *dp);

i32 ecc_ansi_x963_export(ecc_key *key, u8 *out, u64 *outlen);
i32 ecc_ansi_x963_import(u8k *in, u64 inlen, ecc_key *key);
i32 ecc_ansi_x963_import_ex(u8k *in, u64 inlen, ecc_key *key, ltc_ecc_set_type *dp);

i32  ecc_shared_secret(ecc_key *private_key, ecc_key *public_key,
                       u8 *out, u64 *outlen);

i32  ecc_encrypt_key(u8k *in,   u64 inlen,
                           u8 *out,  u64 *outlen,
                           prng_state *prng, i32 wprng, i32 hash,
                           ecc_key *key);

i32  ecc_decrypt_key(u8k *in,  u64  inlen,
                           u8 *out, u64 *outlen,
                           ecc_key *key);

i32 ecc_sign_hash_rfc7518(u8k *in,  u64 inlen,
                                u8 *out, u64 *outlen,
                                prng_state *prng, i32 wprng, ecc_key *key);

i32  ecc_sign_hash(u8k *in,  u64 inlen,
                         u8 *out, u64 *outlen,
                         prng_state *prng, i32 wprng, ecc_key *key);

i32 ecc_verify_hash_rfc7518(u8k *sig,  u64 siglen,
                            u8k *hash, u64 hashlen,
                            i32 *stat, ecc_key *key);

i32  ecc_verify_hash(u8k *sig,  u64 siglen,
                     u8k *hash, u64 hashlen,
                     i32 *stat, ecc_key *key);

/* low level functions */
ecc_point *ltc_ecc_new_point(void);
void       ltc_ecc_del_point(ecc_point *p);
i32        ltc_ecc_is_valid_idx(i32 n);

/* point ops (mp == montgomery digit) */
#if !defined(LTC_MECC_ACCEL) || defined(LTM_DESC) || defined(GMP_DESC)
/* R = 2P */
i32 ltc_ecc_projective_dbl_point(ecc_point *P, ecc_point *R, uk modulus, uk mp);

/* R = P + Q */
i32 ltc_ecc_projective_add_point(ecc_point *P, ecc_point *Q, ecc_point *R, uk modulus, uk mp);
#endif

#if defined(LTC_MECC_FP)
/* optimized point multiplication using fixed point cache (HAC algorithm 14.117) */
i32 ltc_ecc_fp_mulmod(uk k, ecc_point *G, ecc_point *R, uk modulus, i32 map);

/* functions for saving/loading/freeing/adding to fixed point cache */
i32 ltc_ecc_fp_save_state(u8 **out, u64 *outlen);
i32 ltc_ecc_fp_restore_state(u8 *in, u64 inlen);
void ltc_ecc_fp_free(void);
i32 ltc_ecc_fp_add_point(ecc_point *g, uk modulus, i32 lock);

/* lock/unlock all points currently in fixed point cache */
void ltc_ecc_fp_tablelock(i32 lock);
#endif

/* R = kG */
i32 ltc_ecc_mulmod(uk k, ecc_point *G, ecc_point *R, uk modulus, i32 map);

#ifdef LTC_ECC_SHAMIR
/* kA*A + kB*B = C */
i32 ltc_ecc_mul2add(ecc_point *A, uk kA,
                    ecc_point *B, uk kB,
                    ecc_point *C,
                         uk modulus);

#ifdef LTC_MECC_FP
/* Shamir's trick with optimized point multiplication using fixed point cache */
i32 ltc_ecc_fp_mul2add(ecc_point *A, uk kA,
                       ecc_point *B, uk kB,
                       ecc_point *C, uk modulus);
#endif

#endif


/* map P to affine from projective */
i32 ltc_ecc_map(ecc_point *P, uk modulus, uk mp);

#endif

#ifdef LTC_MDSA

/* Max diff between group and modulus size in bytes */
#define LTC_MDSA_DELTA     512

/* Max DSA group size in bytes (default allows 4k-bit groups) */
#define LTC_MDSA_MAX_GROUP 512

/** DSA key structure */
typedef struct {
   /** The key type, PK_PRIVATE or PK_PUBLIC */
   i32 type;

   /** The order of the sub-group used in octets */
   i32 qord;

   /** The generator  */
   uk g;

   /** The prime used to generate the sub-group */
   uk q;

   /** The large prime that generats the field the contains the sub-group */
   uk p;

   /** The private key */
   uk x;

   /** The public key */
   uk y;
} dsa_key;

i32 dsa_make_key(prng_state *prng, i32 wprng, i32 group_size, i32 modulus_size, dsa_key *key);

i32 dsa_set_pqg(u8k *p,  u64 plen,
                u8k *q,  u64 qlen,
                u8k *g,  u64 glen,
                dsa_key *key);
i32 dsa_set_pqg_dsaparam(u8k *dsaparam, u64 dsaparamlen, dsa_key *key);
i32 dsa_generate_pqg(prng_state *prng, i32 wprng, i32 group_size, i32 modulus_size, dsa_key *key);

i32 dsa_set_key(u8k *in, u64 inlen, i32 type, dsa_key *key);
i32 dsa_generate_key(prng_state *prng, i32 wprng, dsa_key *key);

void dsa_free(dsa_key *key);

i32 dsa_sign_hash_raw(u8k *in,  u64 inlen,
                                   uk r,   uk s,
                               prng_state *prng, i32 wprng, dsa_key *key);

i32 dsa_sign_hash(u8k *in,  u64 inlen,
                        u8 *out, u64 *outlen,
                        prng_state *prng, i32 wprng, dsa_key *key);

i32 dsa_verify_hash_raw(         uk r,          uk s,
                    u8k *hash, u64 hashlen,
                                    i32 *stat,      dsa_key *key);

i32 dsa_verify_hash(u8k *sig,  u64 siglen,
                    u8k *hash, u64 hashlen,
                          i32           *stat, dsa_key       *key);

i32 dsa_encrypt_key(u8k *in,   u64 inlen,
                          u8 *out,  u64 *outlen,
                          prng_state *prng, i32 wprng, i32 hash,
                          dsa_key *key);

i32 dsa_decrypt_key(u8k *in,  u64  inlen,
                          u8 *out, u64 *outlen,
                          dsa_key *key);

i32 dsa_import(u8k *in, u64 inlen, dsa_key *key);
i32 dsa_export(u8 *out, u64 *outlen, i32 type, dsa_key *key);
i32 dsa_verify_key(dsa_key *key, i32 *stat);
#ifdef LTC_SOURCE
/* internal helper functions */
i32 dsa_int_validate_xy(dsa_key *key, i32 *stat);
i32 dsa_int_validate_pqg(dsa_key *key, i32 *stat);
i32 dsa_int_validate_primes(dsa_key *key, i32 *stat);
#endif
i32 dsa_shared_secret(void          *private_key, uk base,
                      dsa_key       *public_key,
                      u8 *out,         u64 *outlen);
#endif

#ifdef LTC_DER
/* DER handling */

typedef enum ltc_asn1_type_ {
 /*  0 */
 LTC_ASN1_EOL,
 LTC_ASN1_BOOLEAN,
 LTC_ASN1_INTEGER,
 LTC_ASN1_SHORT_INTEGER,
 LTC_ASN1_BIT_STRING,
 /*  5 */
 LTC_ASN1_OCTET_STRING,
 LTC_ASN1_NULL,
 LTC_ASN1_OBJECT_IDENTIFIER,
 LTC_ASN1_IA5_STRING,
 LTC_ASN1_PRINTABLE_STRING,
 /* 10 */
 LTC_ASN1_UTF8_STRING,
 LTC_ASN1_UTCTIME,
 LTC_ASN1_CHOICE,
 LTC_ASN1_SEQUENCE,
 LTC_ASN1_SET,
 /* 15 */
 LTC_ASN1_SETOF,
 LTC_ASN1_RAW_BIT_STRING,
 LTC_ASN1_TELETEX_STRING,
 LTC_ASN1_CONSTRUCTED,
 LTC_ASN1_CONTEXT_SPECIFIC,
 /* 20 */
 LTC_ASN1_GENERALIZEDTIME,
} ltc_asn1_type;

/** A LTC ASN.1 list type */
typedef struct ltc_asn1_list_ {
   /** The LTC ASN.1 enumerated type identifier */
   ltc_asn1_type type;
   /** The data to encode or place for decoding */
   void         *data;
   /** The size of the input or resulting output */
   u64 size;
   /** The used flag, this is used by the CHOICE ASN.1 type to indicate which choice was made */
   i32           used;
   /** prev/next entry in the list */
   struct ltc_asn1_list_ *prev, *next, *child, *parent;
} ltc_asn1_list;

#define LTC_SET_ASN1(list, index, Type, Data, Size)  \
   do {                                              \
      i32 LTC_MACRO_temp            = (index);       \
      ltc_asn1_list *LTC_MACRO_list = (list);        \
      LTC_MACRO_list[LTC_MACRO_temp].type = (Type);  \
      LTC_MACRO_list[LTC_MACRO_temp].data = (uk )(Data);  \
      LTC_MACRO_list[LTC_MACRO_temp].size = (Size);  \
      LTC_MACRO_list[LTC_MACRO_temp].used = 0;       \
   } while (0)

/* SEQUENCE */
i32 der_encode_sequence_ex(ltc_asn1_list *list, u64 inlen,
                           u8 *out,  u64 *outlen, i32 type_of);

#define der_encode_sequence(list, inlen, out, outlen) der_encode_sequence_ex(list, inlen, out, outlen, LTC_ASN1_SEQUENCE)

i32 der_decode_sequence_ex(u8k *in, u64  inlen,
                           ltc_asn1_list *list,     u64  outlen, i32 ordered);

#define der_decode_sequence(in, inlen, list, outlen) der_decode_sequence_ex(in, inlen, list, outlen, 1)

i32 der_length_sequence(ltc_asn1_list *list, u64 inlen,
                        u64 *outlen);


#ifdef LTC_SOURCE
/* internal helper functions */
i32 der_length_sequence_ex(ltc_asn1_list *list, u64 inlen,
                           u64 *outlen, u64 *payloadlen);
/* SUBJECT PUBLIC KEY INFO */
i32 der_encode_subject_public_key_info(u8 *out, u64 *outlen,
        u32 algorithm, uk public_key, u64 public_key_len,
        u64 parameters_type, uk parameters, u64 parameters_len);

i32 der_decode_subject_public_key_info(u8k *in, u64 inlen,
        u32 algorithm, uk public_key, u64* public_key_len,
        u64 parameters_type, ltc_asn1_list* parameters, u64 parameters_len);
#endif /* LTC_SOURCE */

/* SET */
#define der_decode_set(in, inlen, list, outlen) der_decode_sequence_ex(in, inlen, list, outlen, 0)
#define der_length_set der_length_sequence
i32 der_encode_set(ltc_asn1_list *list, u64 inlen,
                   u8 *out,  u64 *outlen);

i32 der_encode_setof(ltc_asn1_list *list, u64 inlen,
                     u8 *out,  u64 *outlen);

/* VA list handy helpers with triplets of <type, size, data> */
i32 der_encode_sequence_multi(u8 *out, u64 *outlen, ...);
i32 der_decode_sequence_multi(u8k *in, u64 inlen, ...);

/* FLEXI DECODER handle unknown list decoder */
i32  der_decode_sequence_flexi(u8k *in, u64 *inlen, ltc_asn1_list **out);
#define der_free_sequence_flexi         der_sequence_free
void der_sequence_free(ltc_asn1_list *in);
void der_sequence_shrink(ltc_asn1_list *in);

/* BOOLEAN */
i32 der_length_boolean(u64 *outlen);
i32 der_encode_boolean(i32 in,
                       u8 *out, u64 *outlen);
i32 der_decode_boolean(u8k *in, u64 inlen,
                                       i32 *out);
/* INTEGER */
i32 der_encode_integer(uk num, u8 *out, u64 *outlen);
i32 der_decode_integer(u8k *in, u64 inlen, uk num);
i32 der_length_integer(uk num, u64 *len);

/* INTEGER -- handy for 0..2^32-1 values */
i32 der_decode_short_integer(u8k *in, u64 inlen, u64 *num);
i32 der_encode_short_integer(u64 num, u8 *out, u64 *outlen);
i32 der_length_short_integer(u64 num, u64 *outlen);

/* BIT STRING */
i32 der_encode_bit_string(u8k *in, u64 inlen,
                                u8 *out, u64 *outlen);
i32 der_decode_bit_string(u8k *in, u64 inlen,
                                u8 *out, u64 *outlen);
i32 der_encode_raw_bit_string(u8k *in, u64 inlen,
                                u8 *out, u64 *outlen);
i32 der_decode_raw_bit_string(u8k *in, u64 inlen,
                                u8 *out, u64 *outlen);
i32 der_length_bit_string(u64 nbits, u64 *outlen);

/* OCTET STRING */
i32 der_encode_octet_string(u8k *in, u64 inlen,
                                  u8 *out, u64 *outlen);
i32 der_decode_octet_string(u8k *in, u64 inlen,
                                  u8 *out, u64 *outlen);
i32 der_length_octet_string(u64 noctets, u64 *outlen);

/* OBJECT IDENTIFIER */
i32 der_encode_object_identifier(u64 *words, u64  nwords,
                                 u8 *out,   u64 *outlen);
i32 der_decode_object_identifier(u8k *in,    u64  inlen,
                                       u64 *words, u64 *outlen);
i32 der_length_object_identifier(u64 *words, u64 nwords, u64 *outlen);
u64 der_object_identifier_bits(u64 x);

/* IA5 STRING */
i32 der_encode_ia5_string(u8k *in, u64 inlen,
                                u8 *out, u64 *outlen);
i32 der_decode_ia5_string(u8k *in, u64 inlen,
                                u8 *out, u64 *outlen);
i32 der_length_ia5_string(u8k *octets, u64 noctets, u64 *outlen);

i32 der_ia5_char_encode(i32 c);
i32 der_ia5_value_decode(i32 v);

/* TELETEX STRING */
i32 der_decode_teletex_string(u8k *in, u64 inlen,
                                u8 *out, u64 *outlen);
i32 der_length_teletex_string(u8k *octets, u64 noctets, u64 *outlen);

#ifdef LTC_SOURCE
/* internal helper functions */
i32 der_teletex_char_encode(i32 c);
i32 der_teletex_value_decode(i32 v);
#endif /* LTC_SOURCE */


/* PRINTABLE STRING */
i32 der_encode_printable_string(u8k *in, u64 inlen,
                                u8 *out, u64 *outlen);
i32 der_decode_printable_string(u8k *in, u64 inlen,
                                u8 *out, u64 *outlen);
i32 der_length_printable_string(u8k *octets, u64 noctets, u64 *outlen);

i32 der_printable_char_encode(i32 c);
i32 der_printable_value_decode(i32 v);

/* UTF-8 */
#if (defined(SIZE_MAX) || __STDC_VERSION__ >= 199901L || defined(WCHAR_MAX) || defined(__WCHAR_MAX__) || defined(_WCHAR_T) || defined(_WCHAR_T_DEFINED) || defined (__WCHAR_TYPE__)) && !defined(LTC_NO_WCHAR)
   #if defined(__WCHAR_MAX__)
      #define LTC_WCHAR_MAX __WCHAR_MAX__
   #else
      #include <wchar.h>
      #define LTC_WCHAR_MAX WCHAR_MAX
   #endif
/* please note that it might happen that LTC_WCHAR_MAX is undefined */
#else
   typedef ulong32 wchar_t;
   #define LTC_WCHAR_MAX 0xFFFFFFFF
#endif

i32 der_encode_utf8_string(const wchar_t *in,  u64 inlen,
                           u8 *out, u64 *outlen);

i32 der_decode_utf8_string(u8k *in,  u64 inlen,
                                       wchar_t *out, u64 *outlen);
u64 der_utf8_charsize(const wchar_t c);
#ifdef LTC_SOURCE
/* internal helper functions */
i32 der_utf8_valid_char(const wchar_t c);
#endif /* LTC_SOURCE */
i32 der_length_utf8_string(const wchar_t *in, u64 noctets, u64 *outlen);


/* CHOICE */
i32 der_decode_choice(u8k *in,   u64 *inlen,
                            ltc_asn1_list *list, u64  outlen);

/* UTCTime */
typedef struct {
   unsigned YY, /* year */
            MM, /* month */
            DD, /* day */
            hh, /* hour */
            mm, /* minute */
            ss, /* second */
            off_dir, /* timezone offset direction 0 == +, 1 == - */
            off_hh, /* timezone offset hours */
            off_mm; /* timezone offset minutes */
} ltc_utctime;

i32 der_encode_utctime(ltc_utctime *utctime,
                       u8 *out,   u64 *outlen);

i32 der_decode_utctime(u8k *in, u64 *inlen,
                             ltc_utctime   *out);

i32 der_length_utctime(ltc_utctime *utctime, u64 *outlen);

/* GeneralizedTime */
typedef struct {
   unsigned YYYY, /* year */
            MM, /* month */
            DD, /* day */
            hh, /* hour */
            mm, /* minute */
            ss, /* second */
            fs, /* fractional seconds */
            off_dir, /* timezone offset direction 0 == +, 1 == - */
            off_hh, /* timezone offset hours */
            off_mm; /* timezone offset minutes */
} ltc_generalizedtime;

i32 der_encode_generalizedtime(ltc_generalizedtime *gtime,
                               u8       *out, u64 *outlen);

i32 der_decode_generalizedtime(u8k *in, u64 *inlen,
                               ltc_generalizedtime *out);

i32 der_length_generalizedtime(ltc_generalizedtime *gtime, u64 *outlen);


#endif

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
