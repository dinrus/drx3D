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
  @file katja_encrypt_key.c
  Katja PKCS-style OAEP encryption, Tom St Denis
*/

#ifdef LTC_MKAT

/**
    (PKCS #1 v2.0) OAEP pad then encrypt
    @param in          The plaintext
    @param inlen       The length of the plaintext (octets)
    @param out         [out] The ciphertext
    @param outlen      [in/out] The max size and resulting size of the ciphertext
    @param lparam      The system "lparam" for the encryption
    @param lparamlen   The length of lparam (octets)
    @param prng        An active PRNG
    @param prng_idx    The index of the desired prng
    @param hash_idx    The index of the desired hash
    @param key         The Katja key to encrypt to
    @return CRYPT_OK if successful
*/
i32 katja_encrypt_key(u8k *in,     u64 inlen,
                          u8 *out,    u64 *outlen,
                    u8k *lparam, u64 lparamlen,
                    prng_state *prng, i32 prng_idx, i32 hash_idx, katja_key *key)
{
  u64 modulus_bitlen, modulus_bytelen, x;
  i32           err;

  LTC_ARGCHK(in     != NULL);
  LTC_ARGCHK(out    != NULL);
  LTC_ARGCHK(outlen != NULL);
  LTC_ARGCHK(key    != NULL);

  /* valid prng and hash ? */
  if ((err = prng_is_valid(prng_idx)) != CRYPT_OK) {
     return err;
  }
  if ((err = hash_is_valid(hash_idx)) != CRYPT_OK) {
     return err;
  }

  /* get modulus len in bits */
  modulus_bitlen = mp_count_bits((key->N));

  /* payload is upto pq, so we know q is 1/3rd the size of N and therefore pq is 2/3th the size */
  modulus_bitlen = ((modulus_bitlen << 1) / 3);

  /* round down to next byte */
  modulus_bitlen -= (modulus_bitlen & 7) + 8;

  /* outlen must be at least the size of the modulus */
  modulus_bytelen = mp_unsigned_bin_size((key->N));
  if (modulus_bytelen > *outlen) {
     *outlen = modulus_bytelen;
     return CRYPT_BUFFER_OVERFLOW;
  }

  /* OAEP pad the key */
  x = *outlen;
  if ((err = pkcs_1_oaep_encode(in, inlen, lparam,
                                lparamlen, modulus_bitlen, prng, prng_idx, hash_idx,
                                out, &x)) != CRYPT_OK) {
     return err;
  }

  /* Katja exptmod the OAEP pad */
  return katja_exptmod(out, x, out, outlen, PK_PUBLIC, key);
}

#endif /* LTC_MRSA */

/* ref:         HEAD -> master, tag: v1.18.2 */
/* git commit:  7e7eb695d581782f04b24dc444cbfde86af59853 */
/* commit time: 2018-07-01 22:49:01 +0200 */
