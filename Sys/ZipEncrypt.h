// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _ZIP_ENCRYPT_HDR_
#define _ZIP_ENCRYPT_HDR_
/*
#ifdef SUPPORT_RSA_AND_STREAMCIPHER_PAK_ENCRYPTION
#undef SUPPORT_RSA_AND_STREAMCIPHER_PAK_ENCRYPTION
#endif

#ifdef INCLUDE_LIBTOMCRYPT
#undef INCLUDE_LIBTOMCRYPT
#endif

#define SUPPORT_UNSIGNED_PAKS
//Временно аннулируем использование шифрованных зип-архивов,
//так как требуется tomcrypt...

//Далее код не должен сработать!
*/
//#define SUPPORT_RSA_AND_STREAMCIPHER_PAK_ENCRYPTION 1
//#define INCLUDE_LIBTOMCRYPT 1

//#ifdef SUPPORT_RSA_AND_STREAMCIPHER_PAK_ENCRYPTION

#include <drx3D/Sys/ZipFileFormat.h>
#include <drx3D/Sys/ZipDirStructures.h>

//#ifdef INCLUDE_LIBTOMCRYPT
	#include <drx3D/Plugins/tomcrypt/tomcrypt.h>
	#include <drx3D/CoreX/Assert/DrxAssert.h> // tomcrypt will re-define assert
	#undef byte                           // tomcrypt defines a byte macro which conflicts with out byte data type
	#define STREAM_CIPHER_NAME "twofish"
extern prng_state g_yarrow_prng_state;
extern rsa_key g_rsa_key_public_for_sign;
//#endif //INCLUDE_LIBTOMCRYPT

namespace ZipEncrypt
{
//#ifdef INCLUDE_LIBTOMCRYPT
void Init(u8k* pKeyData, u32 keyLen);
bool StartStreamCipher(u8 key[16], u8 IV[16], symmetric_CTR* pCTR, u32k offset = 0);
void FinishStreamCipher(symmetric_CTR* pCTR);
bool DecryptBufferWithStreamCipher(u8* inBuffer, u8* outBuffer, size_t bufferSize, symmetric_CTR* pCTR);
bool DecryptBufferWithStreamCipher(u8* inBuffer, size_t bufferSize, u8 key[16], u8 IV[16]);
i32  GetEncryptionKeyIndex(const ZipDir::FileEntry* pFileEntry);
void GetEncryptionInitialVector(const ZipDir::FileEntry* pFileEntry, u8 IV[16]);

bool RSA_VerifyData(uk inBuffer, i32 sizeIn, u8* signedHash, i32 signedHashSize, rsa_key& publicKey);
bool RSA_VerifyData(u8k** inBuffers, u32* sizesIn, i32k numBuffers, u8* signedHash, i32 signedHashSize, rsa_key& publicKey);

i32  custom_rsa_encrypt_key_ex(u8k* in, u64 inlen,
                               u8* out, u64* outlen,
                               u8k* lparam, u64 lparamlen,
                               prng_state* prng, i32 prng_idx, i32 hash_idx, i32 padding, rsa_key* key);

i32 custom_rsa_decrypt_key_ex(u8k* in, u64 inlen,
                              u8* out, u64* outlen,
                              u8k* lparam, u64 lparamlen,
                              i32 hash_idx, i32 padding,
                              i32* stat, rsa_key* key);
//#endif  //INCLUDE_LIBTOMCRYPT
};
//#endif
#endif //_ZIP_ENCRYPT_HDR_
