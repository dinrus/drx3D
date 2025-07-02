// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "MemoryBlock.h"
#include <zlib.h>
#include "Controls/QuestionDialog.h"

//////////////////////////////////////////////////////////////////////////
CMemoryBlock::CMemoryBlock()
{
	m_buffer = 0;
	m_size = 0;
	m_uncompressedSize = 0;
	m_owns = false;
}

//////////////////////////////////////////////////////////////////////////
CMemoryBlock::CMemoryBlock(const CMemoryBlock& mem)
{
	*this = mem;
}

//////////////////////////////////////////////////////////////////////////
CMemoryBlock::~CMemoryBlock()
{
	Free();
}

//////////////////////////////////////////////////////////////////////////
CMemoryBlock& CMemoryBlock::operator=(const CMemoryBlock& mem)
{
	if (mem.GetSize() > 0)
	{
		// Do not reallocate.
		if (mem.GetSize() > GetSize())
		{
			if (!Allocate(mem.GetSize()))
				return *this;
		}
		Copy(mem.GetBuffer(), mem.GetSize());
	}
	else
	{
		m_buffer = 0;
		m_size = 0;
		m_owns = false;
	}
	m_uncompressedSize = mem.m_uncompressedSize;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
bool CMemoryBlock::Allocate(i32 size, i32 uncompressedSize)
{
	assert(size > 0);
	if (m_buffer)
	{
		m_buffer = realloc(m_buffer, size);
	}
	else
	{
		m_buffer = malloc(size);
	}
	if (!m_buffer)
	{
		string str;
		str.Format("CMemoryBlock::Allocate failed to allocate %dMb of Memory", size / (1024 * 1024));
		DrxLogAlways(str);

		CQuestionDialog::SWarning(QObject::tr(""), QObject::tr(str + string("\r\nSandbox will try to reduce its working memory set to free memory for this allocation.")));
		GetIEditor()->ReduceMemory();
		if (m_buffer)
		{
			m_buffer = realloc(m_buffer, size);
		}
		else
		{
			m_buffer = malloc(size);
		}
		if (!m_buffer)
		{
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, "Reducing working memory set failed, Sandbox must quit");
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, "Reducing working memory set succeeded\r\nSandbox may become unstable, it is advised to save the level and restart editor.");
		}
	}

	m_owns = true;
	m_size = size;
	m_uncompressedSize = uncompressedSize;
	// Check if allocation failed.
	if (m_buffer == 0)
		return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CMemoryBlock::Free()
{
	if (m_buffer && m_owns)
		free(m_buffer);
	m_buffer = 0;
	m_owns = false;
	m_size = 0;
	m_uncompressedSize = 0;
}

//////////////////////////////////////////////////////////////////////////
void CMemoryBlock::Copy(uk src, i32 size)
{
	assert(size <= m_size);
	memcpy(m_buffer, src, size);
}

//////////////////////////////////////////////////////////////////////////
void CMemoryBlock::Attach(uk buffer, i32 size, i32 uncompressedSize)
{
	Free();
	m_owns = false;
	m_buffer = buffer;
	m_size = size;
	m_uncompressedSize = uncompressedSize;
}

//////////////////////////////////////////////////////////////////////////
void CMemoryBlock::Detach()
{
	Free();
}

//////////////////////////////////////////////////////////////////////////
void CMemoryBlock::Compress(CMemoryBlock& toBlock) const
{
	// Cannot compress to itself.
	assert(this != &toBlock);
	u64 destSize = m_size * 2 + 128;
	CMemoryBlock temp;
	temp.Allocate(destSize);

	compress((u8*)temp.GetBuffer(), &destSize, (u8*)GetBuffer(), m_size);

	toBlock.Allocate(destSize);
	toBlock.Copy(temp.GetBuffer(), destSize);
	toBlock.m_uncompressedSize = GetSize();
}

//////////////////////////////////////////////////////////////////////////
void CMemoryBlock::Uncompress(CMemoryBlock& toBlock) const
{
	assert(this != &toBlock);
	toBlock.Allocate(m_uncompressedSize);
	toBlock.m_uncompressedSize = 0;
	u64 destSize = m_uncompressedSize;
	i32 result = uncompress((u8*)toBlock.GetBuffer(), &destSize, (u8*)GetBuffer(), GetSize());
	assert(result == Z_OK);
	assert(destSize == m_uncompressedSize);
}

//////////////////////////////////////////////////////////////////////////
void CMemoryBlock::Serialize(CArchive& ar)
{
	if (ar.IsLoading())
	{
		i32 size;
		// Loading.
		ar >> size;
		if (size != m_size)
			Allocate(size);
		m_size = size;
		ar >> m_uncompressedSize;
		ar.Read(m_buffer, m_size);
	}
	else
	{
		// Saving.
		ar << m_size;
		ar << m_uncompressedSize;
		ar.Write(m_buffer, m_size);
	}
}

