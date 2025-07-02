// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/****************************************************
   A simple stream cipher based on RC4
****************************************************/

#ifndef __STREAMCIPHER_H__
#define __STREAMCIPHER_H__

class CStreamCipher
{
public:
	CStreamCipher();
	~CStreamCipher();

	void Init(u8k* key, i32 keyLen);
	void Encrypt(u8k* input, i32 inputLen, u8* output)       { ProcessBuffer(input, inputLen, output, true); }
	void Decrypt(u8k* input, i32 inputLen, u8* output)       { ProcessBuffer(input, inputLen, output, true); }
	void EncryptStream(u8k* input, i32 inputLen, u8* output) { ProcessBuffer(input, inputLen, output, false); }
	void DecryptStream(u8k* input, i32 inputLen, u8* output) { ProcessBuffer(input, inputLen, output, false); }

private:
	u8 GetNext();
	void  ProcessBuffer(u8k* input, i32 inputLen, u8* output, bool resetKey);

	u8 m_StartS[256];
	u8 m_S[256];
	i32   m_StartI;
	i32   m_I;
	i32   m_StartJ;
	i32   m_J;
};

#endif
