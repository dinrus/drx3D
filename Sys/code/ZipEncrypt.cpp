// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/System.h>

#include <drx3D/CoreX/smartptr.h>
#include <drx/Core/lib/z/zlib.h>
#include <drx3D/Sys/ZipFileFormat.h>
#include <drx3D/Sys/ZipDirStructures.h>
#include <drx3D/Sys/ZipEncrypt.h>

/////////////////////////////////////////////

//extern const ltc_math_descriptor ltm_desc;


//////////////////////////////////////////////////////////////////////////
//#ifdef INCLUDE_LIBTOMCRYPT

prng_state g_yarrow_prng_state;
// Main public RSA key used for verifying Drx Pak comments
rsa_key g_rsa_key_public_for_sign;

// Until we have attempted to initialize ZipEncrypt, there is no point in warning about possible initialization failure
bool g_using_zipencrypt = false;

	#define mp_count_bits(a)        ltc_mp.count_bits(a)
	#define mp_unsigned_bin_size(a) ltc_mp.unsigned_size(a)
	

namespace ZipEncrypt
{
	
////////////////////////////////////////////
void Init(u8k* pKeyData, u32 keyLen)
{
	LOADING_TIME_PROFILE_SECTION;

	g_using_zipencrypt = true;
	
	register_hash(&sha1_desc);
	register_hash(&sha256_desc);

	register_cipher(&twofish_desc);

	i32 prng_idx = register_prng(&yarrow_desc) != -1;
	assert(prng_idx != -1);
	rng_make_prng(128, find_prng("yarrow"), &g_yarrow_prng_state, NULL);

	i32 importReturn = rsa_import(pKeyData, (u64)keyLen, &g_rsa_key_public_for_sign);
	if (CRYPT_OK != importReturn)
	{
	#if !defined(_RELEASE)
		DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, "RSA Public Key failed to initialize. Returned %d", importReturn);
	#endif //_RELEASE
	}
}

bool StartStreamCipher(u8 key[16], u8 IV[16], symmetric_CTR* pCTR, u32k offset)
{
	i32 err;

	i32 cipher_idx = find_cipher(STREAM_CIPHER_NAME);
	if (cipher_idx < 0)
		return false;

	err = ctr_start(cipher_idx, IV, key, 16, 0, CTR_COUNTER_LITTLE_ENDIAN, pCTR);
	if (err != CRYPT_OK)
	{
		//printf("ctr_start error: %s\n",error_to_string(errno));
		return false;
	}

	// Seek forward into the stream cipher by offset bytes
	u32 offset_blocks = offset / pCTR->blocklen;
	u32 offset_remaining = offset - (offset_blocks * pCTR->blocklen);

	if (offset_blocks > 0)
	{
		SwapEndian((u32*)(&pCTR->ctr[0]), 4);
		*((u32*)(&pCTR->ctr[0])) += offset_blocks;
		SwapEndian((u32*)(&pCTR->ctr[0]), 4);

		ctr_setiv(pCTR->ctr, pCTR->ctrlen, pCTR);
	}

	// Seek into the last block to initialize the padding
	u32 bytesConsumed = 0;
	while (bytesConsumed < offset_remaining)
	{
		const static u32 bufSize = 1024;
		u8 buffer[bufSize] = { 0 };
		u32 bytesToConsume = min(bufSize, offset_remaining - bytesConsumed);
		ctr_decrypt(buffer, buffer, bytesToConsume, pCTR);
		bytesConsumed += bytesToConsume;
	}

	return true;
}

void FinishStreamCipher(symmetric_CTR* pCTR)
{
	ctr_done(pCTR);
}

bool DecryptBufferWithStreamCipher(u8* inBuffer, u8* outBuffer, size_t bufferSize, symmetric_CTR* pCTR)
{
	i32 err;
	err = ctr_decrypt(inBuffer, outBuffer, bufferSize, pCTR);
	if (err != CRYPT_OK)
	{
		//printf("ctr_encrypt error: %s\n", error_to_string(errno));
		return false;
	}
	return true;
}

bool DecryptBufferWithStreamCipher(u8* inBuffer, size_t bufferSize, u8 key[16], u8 IV[16])
{
	LOADING_TIME_PROFILE_SECTION

	symmetric_CTR ctr;

	if (!StartStreamCipher(key, IV, &ctr))
	{
		return false;
	}

	if (!DecryptBufferWithStreamCipher(inBuffer, inBuffer, bufferSize, &ctr))
	{
		return false;
	}

	ctr_done(&ctr);

	return true;
}

i32 GetEncryptionKeyIndex(const ZipDir::FileEntry* pFileEntry)
{
	return (~(pFileEntry->desc.lCRC32 >> 2)) & 0xF;
}

void GetEncryptionInitialVector(const ZipDir::FileEntry* pFileEntry, u8 IV[16])
{
	u32 intIV[4]; //16 byte

	intIV[0] = pFileEntry->desc.lSizeUncompressed ^ (pFileEntry->desc.lSizeCompressed << 12);
	intIV[1] = (!pFileEntry->desc.lSizeCompressed);
	intIV[2] = pFileEntry->desc.lCRC32 ^ (pFileEntry->desc.lSizeCompressed << 12);
	intIV[3] = !pFileEntry->desc.lSizeUncompressed ^ pFileEntry->desc.lSizeCompressed;

	memcpy(IV, intIV, sizeof(intIV));
}

//////////////////////////////////////////////////////////////////////////
bool RSA_VerifyData(uk inBuffer, i32 sizeIn, u8* signedHash, i32 signedHashSize, rsa_key& publicKey)
{
	// verify hash

	i32 sha256 = find_hash("sha256");
	if (sha256 == -1)
	{
	#if !defined(_RELEASE)
		if (g_using_zipencrypt)
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR_DBGBRK, "Hash program for RSA_VerifyData could not be found. LibTomCrypt has failed to start.");
	#endif
		return false;
	}

	i32 hashSize = 32; // 32 bytes for SHA 256

	u8 hash_digest[1024]; // 32 bytes should be enough

	hash_state md;
	hash_descriptor[sha256].init(&md);
	hash_descriptor[sha256].process(&md, (u8*)inBuffer, sizeIn);
	hash_descriptor[sha256].done(&md, hash_digest); // 32 bytes

	assert(hash_descriptor[sha256].hashsize == hashSize);

	i32 prng_idx = find_prng("yarrow");
	assert(prng_idx != -1);

	// Verify generated hash with RSA public key
	i32 statOut = 0;
	i32 res = rsa_verify_hash(signedHash, signedHashSize, hash_digest, hashSize, sha256, 0, &statOut, &publicKey);
	if (res != CRYPT_OK || statOut != 1)
	{
		return false;
	}
	return true;
}

bool RSA_VerifyData(u8k** inBuffers, u32* sizesIn, i32k numBuffers, u8* signedHash, i32 signedHashSize, rsa_key& publicKey)
{
	// verify hash from multiple buffers

	i32 sha256 = find_hash("sha256");
	if (sha256 == -1)
	{
	#if !defined(_RELEASE)
		if (g_using_zipencrypt)
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR_DBGBRK, "Hash program for RSA_VerifyData could not be found. LibTomCrypt has failed to start.");
	#endif
		return false;
	}

	i32 hashSize = 32; // 32 bytes for SHA 256

	u8 hash_digest[1024]; // 32 bytes should be enough

	hash_state md;
	hash_descriptor[sha256].init(&md);
	for (i32 i = 0; i < numBuffers; i++)
	{
		hash_descriptor[sha256].process(&md, inBuffers[i], sizesIn[i]);
	}
	hash_descriptor[sha256].done(&md, hash_digest); // 32 bytes

	assert(hash_descriptor[sha256].hashsize == hashSize);

	i32 prng_idx = find_prng("yarrow");
	assert(prng_idx != -1);

	// Verify generated hash with RSA public key
	i32 statOut = 0;
	i32 res = rsa_verify_hash(signedHash, signedHashSize, hash_digest, hashSize, sha256, 0, &statOut, &publicKey);
	if (res != CRYPT_OK || statOut != 1)
	{
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
i32 custom_rsa_encrypt_key_ex(u8k* in, u64 inlen,
                                          u8* out, u64* outlen,
                                          u8k* lparam, u64 lparamlen,
                                          prng_state* prng, i32 prng_idx, i32 hash_idx, i32 padding, rsa_key* key)
{
	u64 modulus_bitlen, modulus_bytelen, x;
	i32 err;

	DRX_ASSERT(in != NULL);
	DRX_ASSERT(out != NULL);
	DRX_ASSERT(outlen != NULL);
	DRX_ASSERT(key != NULL);

	/* valid padding? */
	if ((padding != LTC_PKCS_1_V1_5) &&
	    (padding != LTC_PKCS_1_OAEP))
	{
		return CRYPT_PK_INVALID_PADDING;
	}

	/* valid prng? */
	if ((err = prng_is_valid(prng_idx)) != CRYPT_OK)
	{
		return err;
	}

	if (padding == LTC_PKCS_1_OAEP)
	{
		/* valid hash? */
		if ((err = hash_is_valid(hash_idx)) != CRYPT_OK)
		{
			return err;
		}
	}

	/* get modulus len in bits */
	modulus_bitlen = mp_count_bits((key->N));

	/* outlen must be at least the size of the modulus */
	modulus_bytelen = mp_unsigned_bin_size((key->N));
	if (modulus_bytelen > *outlen)
	{
		*outlen = modulus_bytelen;
		return CRYPT_BUFFER_OVERFLOW;
	}

	if (padding == LTC_PKCS_1_OAEP)
	{
		/* OAEP pad the key */
		x = *outlen;
		if ((err = pkcs_1_oaep_encode(in, inlen, lparam,
		                              lparamlen, modulus_bitlen, prng, prng_idx, hash_idx,
		                              out, &x)) != CRYPT_OK)
		{
			return err;
		}
	}
	else
	{
		/* LTC_PKCS #1 v1.5 pad the key */
		x = *outlen;
		if ((err = pkcs_1_v1_5_encode(in, inlen, LTC_PKCS_1_EME,
		                              modulus_bitlen, prng, prng_idx,
		                              out, &x)) != CRYPT_OK)
		{
			return err;
		}
	}

	/* rsa exptmod the OAEP or LTC_PKCS #1 v1.5 pad */
	return ltc_mp.rsa_me(out, x, out, outlen, PK_PRIVATE, key);
}

//////////////////////////////////////////////////////////////////////////
i32 custom_rsa_decrypt_key_ex(u8k* in, u64 inlen,
                                          u8* out, u64* outlen,
                                          u8k* lparam, u64 lparamlen,
                                          i32 hash_idx, i32 padding,
                                          i32* stat, rsa_key* key)
{
	u64 modulus_bitlen, modulus_bytelen, x;
	i32 err;
	u8* tmp;

	DRX_ASSERT(out != NULL);
	DRX_ASSERT(outlen != NULL);
	DRX_ASSERT(key != NULL);
	DRX_ASSERT(stat != NULL);

	/* default to invalid */
	*stat = 0;

	/* valid padding? */

	if ((padding != LTC_PKCS_1_V1_5) &&
	    (padding != LTC_PKCS_1_OAEP))
	{
		return CRYPT_PK_INVALID_PADDING;
	}

	if (padding == LTC_PKCS_1_OAEP)
	{
		/* valid hash ? */
		if ((err = hash_is_valid(hash_idx)) != CRYPT_OK)
		{
			return err;
		}
	}

	/* get modulus len in bits */
	modulus_bitlen = mp_count_bits((key->N));

	/* outlen must be at least the size of the modulus */
	modulus_bytelen = mp_unsigned_bin_size((key->N));
	if (modulus_bytelen != inlen)
	{
		return CRYPT_INVALID_PACKET;
	}

	/* allocate ram */
	tmp = (u8*)XMALLOC(inlen);
	if (tmp == NULL)
	{
		return CRYPT_MEM;
	}

	/* rsa decode the packet */
	x = inlen;
	if ((err = ltc_mp.rsa_me(in, inlen, tmp, &x, PK_PUBLIC, key)) != CRYPT_OK)
	{
		XFREE(tmp);
		return err;
	}

	if (padding == LTC_PKCS_1_OAEP)
	{
		/* now OAEP decode the packet */
		err = pkcs_1_oaep_decode(tmp, x, lparam, lparamlen, modulus_bitlen, hash_idx,
		                         out, outlen, stat);
	}
	else
	{
		/* now LTC_PKCS #1 v1.5 depad the packet */
		err = pkcs_1_v1_5_decode(tmp, x, LTC_PKCS_1_EME, modulus_bitlen, out, outlen, stat);
	}

	XFREE(tmp);
	return err;
}

}//ns end

//#endif //INCLUDE_LIBTOMCRYPT
//////////////////////////////////////////////////////////////////////////
//#endif