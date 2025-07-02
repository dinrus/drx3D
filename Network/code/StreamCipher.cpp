// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/StreamCipher.h>

CStreamCipher::CStreamCipher()
	: m_StartI(0)
	, m_I(0)
	, m_StartJ(0)
	, m_J(0)
{
	ZeroMemory(&m_StartS, sizeof(m_StartS));
	ZeroMemory(&m_S, sizeof(m_S));
}

CStreamCipher::~CStreamCipher()
{
}

void CStreamCipher::Init(u8k* key, i32 keyLen)
{
	i32 i, j;

	for (i = 0; i < 256; i++)
	{
		m_S[i] = i;
	}

	if (key)
	{
		for (i = j = 0; i < 256; i++)
		{
			u8 temp;

			j = (j + key[i % keyLen] + m_S[i]) & 255;
			temp = m_S[i];
			m_S[i] = m_S[j];
			m_S[j] = temp;
		}
	}

	m_I = m_J = 0;

	for (i = 0; i < 1024; i++)
	{
		GetNext();
	}

	memcpy(m_StartS, m_S, sizeof(m_StartS));

	m_StartI = m_I;
	m_StartJ = m_J;
}

u8 CStreamCipher::GetNext()
{
	u8 tmp;

	m_I = (m_I + 1) & 0xff;
	m_J = (m_J + m_S[m_I]) & 0xff;

	tmp = m_S[m_J];
	m_S[m_J] = m_S[m_I];
	m_S[m_I] = tmp;

	return m_S[(m_S[m_I] + m_S[m_J]) & 0xff];
}

void CStreamCipher::ProcessBuffer(u8k* input, i32 inputLen, u8* output, bool resetKey)
{
	if (resetKey)
	{
		memcpy(m_S, m_StartS, sizeof(m_S));
		m_I = m_StartI;
		m_J = m_StartJ;
	}

	for (i32 i = 0; i < inputLen; i++)
	{
		output[i] = input[i] ^ GetNext();
	}
}
