// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
SingleAllocTextBlock.cpp
Created December 2009 by Tim Furnish
*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Utility/SingleAllocTextBlock.h>
#include <drx3D/CoreX/String/StringUtils.h>

#if defined(_DEBUG)
	#define MORE_SINGLE_ALLOC_TEXT_BLOCK_CHECKS   1
#else
	#define MORE_SINGLE_ALLOC_TEXT_BLOCK_CHECKS   0
#endif

#if 0 && defined(USER_timf)
	#define SingleAllocTextBlockLog(...) DrxLog("[TEXTBLOCK] " __VA_ARGS__);
#else
	#define SingleAllocTextBlockLog(...)
#endif

//----------------------------------------------------------
CSingleAllocTextBlock::CSingleAllocTextBlock() :
	m_mem(NULL)
{
	Reset ();
}

//----------------------------------------------------------
CSingleAllocTextBlock::~CSingleAllocTextBlock()
{
	Reset ();
}

//----------------------------------------------------------
void CSingleAllocTextBlock::Reset()
{
	SAFE_DELETE_ARRAY(m_mem);
	m_sizeNeeded = 0;
	m_sizeNeededWithoutUsingDuplicates = 0;
	m_numBytesUsed = 0;
	m_reuseDuplicatedStringsArray = NULL;
	m_reuseDuplicatedStringsArraySize = 0;
	m_reuseDuplicatedStringsNumUsed = 0;
}

//----------------------------------------------------------
void CSingleAllocTextBlock::EmptyWithoutFreeing()
{
	m_numBytesUsed = 0;
	m_reuseDuplicatedStringsNumUsed = 0;
}

//----------------------------------------------------------
void CSingleAllocTextBlock::IncreaseSizeNeeded(size_t theSize)
{
	DRX_ASSERT_MESSAGE(m_mem == NULL, "Shouldn't try and increase size after memory allocation!");

	m_sizeNeeded += theSize;
	m_sizeNeededWithoutUsingDuplicates += theSize;
}

//----------------------------------------------------------
void CSingleAllocTextBlock::IncreaseSizeNeeded(tukk  textIn, bool doDuplicateCheck)
{
	DRX_ASSERT_MESSAGE(m_mem == NULL, "Shouldn't try and increase size after memory allocation!");

	if (textIn)
	{
		size_t lengthIncludingTerminator = strlen(textIn) + 1;
		if (doDuplicateCheck == false || FindDuplicate(textIn) == NULL)
		{
			if (doDuplicateCheck)
			{
				RememberPossibleDuplicate(textIn);
			}
			m_sizeNeeded += lengthIncludingTerminator;
		}
		m_sizeNeededWithoutUsingDuplicates += lengthIncludingTerminator;
	}
}

//----------------------------------------------------------
void CSingleAllocTextBlock::Allocate()
{
	DRX_ASSERT_TRACE(m_mem == NULL, ("Already allocated memory but something's trying to allocate %u bytes more", m_sizeNeeded));
	m_mem = new char[m_sizeNeeded + MORE_SINGLE_ALLOC_TEXT_BLOCK_CHECKS];
	DRX_ASSERT_TRACE(m_mem != NULL, ("Failed to allocate %u bytes of memory!", m_sizeNeeded));

	SingleAllocTextBlockLog ("Allocated %u bytes of memory (saved %u bytes by reusing strings; number of unique strings found is %d/%d)", m_sizeNeeded, m_sizeNeededWithoutUsingDuplicates - m_sizeNeeded, m_reuseDuplicatedStringsNumUsed, m_reuseDuplicatedStringsArraySize);

	m_reuseDuplicatedStringsNumUsed = 0;

#if MORE_SINGLE_ALLOC_TEXT_BLOCK_CHECKS
	m_mem[m_sizeNeeded] = '@';
#endif

	DRX_ASSERT_TRACE(m_numBytesUsed == 0, ("Allocating memory but have apparently already used %u bytes!", m_numBytesUsed));
}

//----------------------------------------------------------
tukk  CSingleAllocTextBlock::FindDuplicate(tukk  textIn)
{
	assert (m_reuseDuplicatedStringsNumUsed <= m_reuseDuplicatedStringsArraySize);

	for (i32 i = 0; i < m_reuseDuplicatedStringsNumUsed; ++ i)
	{
		if (0 == strcmp (textIn, m_reuseDuplicatedStringsArray[i].m_charPtr))
		{
			SingleAllocTextBlockLog("Stored '%s' already! Reusing pointer!", textIn);
			return m_reuseDuplicatedStringsArray[i].m_charPtr;
		}
	}

	return NULL;
}


//----------------------------------------------------------
void CSingleAllocTextBlock::RememberPossibleDuplicate(tukk  textIn)
{
	if (m_reuseDuplicatedStringsArray)
	{
		if (m_reuseDuplicatedStringsNumUsed < m_reuseDuplicatedStringsArraySize)
		{
			m_reuseDuplicatedStringsArray[m_reuseDuplicatedStringsNumUsed].m_charPtr = textIn;
			++ m_reuseDuplicatedStringsNumUsed;
		}
		else
		{
			DRX_ASSERT_MESSAGE(m_mem == NULL, "Only expected to find too many unique strings before memory has been allocated, not afterwards!");
			SingleAllocTextBlockLog("Too many unique strings - workspace only big enough to cope with %u! The string '%s' was one too many. Not consolidating duplicated strings as a precaution...", m_reuseDuplicatedStringsArraySize, textIn);
			m_reuseDuplicatedStringsArray = NULL;
			m_reuseDuplicatedStringsArraySize = 0;
			m_reuseDuplicatedStringsNumUsed = 0;
			m_sizeNeeded = m_sizeNeededWithoutUsingDuplicates;
		}
	}
}

//----------------------------------------------------------
tukk  CSingleAllocTextBlock::StoreText(tukk  textIn, bool doDuplicateCheck)
{
	tukk  reply = NULL;

	if (textIn)
	{
		reply = doDuplicateCheck ? FindDuplicate(textIn) : NULL;
		if (reply == NULL)
		{
			DRX_ASSERT_MESSAGE(m_mem != NULL, "No memory has been allocated!");
			if (drx_strcpy(m_mem + m_numBytesUsed, m_sizeNeeded - m_numBytesUsed, textIn))
			{
				reply = m_mem + m_numBytesUsed;
				m_numBytesUsed += strlen(reply) + 1;
				if (doDuplicateCheck)
				{
					RememberPossibleDuplicate(reply);
				}
			}
#ifndef _RELEASE
			else
			{
				GameWarning("Tried to store too much text in a single-alloc text block of size %" PRISIZE_T " (%" PRISIZE_T " bytes have already been used, no room for '%s')", m_sizeNeeded, m_numBytesUsed, textIn);
			}
#endif
		}
		SingleAllocTextBlockLog ("Storing a copy of '%s', now used %u/%u bytes, %u bytes left", textIn, m_numBytesUsed, m_sizeNeeded, m_sizeNeeded - m_numBytesUsed);
	}

	DRX_ASSERT_TRACE (m_numBytesUsed <= m_sizeNeeded, ("Counters have been set to invalid values! Apparently used %d/%d bytes!", m_numBytesUsed, m_sizeNeeded));

	return reply;
}

//----------------------------------------------------------
void CSingleAllocTextBlock::Lock()
{
	DRX_ASSERT_MESSAGE(m_numBytesUsed == m_sizeNeeded, string().Format("Didn't fill entire block of reserved memory: allocated %d bytes, used %d", m_sizeNeeded, m_numBytesUsed));

#if MORE_SINGLE_ALLOC_TEXT_BLOCK_CHECKS
	DRX_ASSERT_MESSAGE(m_mem[m_sizeNeeded] == '@', "Memory overwrite");
#endif

	m_reuseDuplicatedStringsArray = NULL;
	m_reuseDuplicatedStringsArraySize = 0;
	m_reuseDuplicatedStringsNumUsed = 0;
}