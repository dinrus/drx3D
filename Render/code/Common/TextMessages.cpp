// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/TextMessages.h>

u32k g_dwTextMessageMaxSizeInKB = 128;

const CTextMessages& CTextMessages::operator=(const CTextMessages& rhs)
{
	this->m_TextMessageData = rhs.m_TextMessageData;
	this->m_dwCurrentReadPos = rhs.m_dwCurrentReadPos;

	return *this;
}

void CTextMessages::Merge(const CTextMessages& rhs)
{
	m_TextMessageData.insert(m_TextMessageData.end(), rhs.m_TextMessageData.begin(), rhs.m_TextMessageData.end());
}

void CTextMessages::PushEntry_Text(const Vec3& vPos, const ColorB col, IFFont* pFont, const Vec2& fFontSize, i32k nDrawFlags, tukk szText)
{
	AUTO_LOCK(m_TextMessageLock); // Not thread safe without this

	assert(szText);
	assert(!m_dwCurrentReadPos);    // iteration should not be started

	size_t texlen = strlen(szText);
	size_t size = sizeof(SText) + texlen + 1;

	if (size > 1020)
	{
		size = 1020;
		texlen = size - sizeof(SText) - 1;
	}

	size_t paddedSize = (size + 3) & ~3;
	u8* pData = PushData((u32) paddedSize);

	if (!pData)
		return;

	memcpy((tuk)pData + sizeof(SText), szText, texlen);
	pData[sizeof(SText) + texlen] = '\0';

	SText& rHeader = *(SText*)pData;

	rHeader.Init((u32) paddedSize);
	rHeader.m_vPos = vPos;
	rHeader.m_Color = col;
	rHeader.m_fFontSize = fFontSize;
	rHeader.m_nDrawFlags = nDrawFlags;
	rHeader.m_pFont = pFont;
}

void CTextMessages::Clear(bool posonly)
{
	if (!posonly)
	{
		m_TextMessageData.clear();
		stl::free_container(m_TextMessageData);
	}

	m_dwCurrentReadPos = 0;
}

const CTextMessages::CTextMessageHeader* CTextMessages::GetNextEntry()
{
	u32 dwSize = (u32)m_TextMessageData.size();

	if (m_dwCurrentReadPos >= dwSize)
		return 0;   // end reached

	const CTextMessageHeader* pHeader = (const CTextMessageHeader*)&m_TextMessageData[m_dwCurrentReadPos];

	m_dwCurrentReadPos += pHeader->GetSize();

	return pHeader;
}

u8* CTextMessages::PushData(u32k dwBytes)
{
	assert(dwBytes);
	assert(dwBytes % 4 == 0);

	u32 dwSize = (u32)m_TextMessageData.size();

	if (dwSize + dwBytes > g_dwTextMessageMaxSizeInKB * 1024)
		return 0;

	m_TextMessageData.resize(dwSize + dwBytes);

	return (u8*)&m_TextMessageData[dwSize];
}

u32 CTextMessages::ComputeSizeInMemory() const
{
	return (u32)(sizeof(*this) + m_TextMessageData.size());
}
