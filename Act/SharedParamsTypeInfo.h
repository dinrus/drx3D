// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Shared parameters type information.

   -------------------------------------------------------------------------
   История:
   - 15:07:2010: Created by Paul Slinger

*************************************************************************/

#ifndef __SHAREDPARAMSTYPEINFO_H__
#define __SHAREDPARAMSTYPEINFO_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/CoreX/String/StringUtils.h>

#ifdef _DEBUG
	#define DEBUG_SHARED_PARAMS_TYPE_INFO 1
#else
	#define DEBUG_SHARED_PARAMS_TYPE_INFO 0
#endif //_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////
// Shared parameters type information class.
////////////////////////////////////////////////////////////////////////////////////////////////////
class CSharedParamsTypeInfo
{
public:

	CSharedParamsTypeInfo(size_t size, tukk pName, tukk pFileName, u32 line) : m_size(size)
	{
		if (pName)
		{
			const size_t length = strlen(pName);
			size_t pos = 0;

			if (length > sizeof(m_name) - 1)
			{
				pos = length - (sizeof(m_name) - 1);
			}

			drx_strcpy(m_name, pName + pos);
		}
		else
		{
			m_name[0] = '\0';
		}

		if (pFileName)
		{
			const size_t length = strlen(pFileName);
			size_t pos = 0;

			if (length > sizeof(m_fileName) - 1)
			{
				pos = length - (sizeof(m_fileName) - 1);
			}

			drx_strcpy(m_fileName, pFileName + pos);
		}
		else
		{
			m_fileName[0] = '\0';
		}

		m_line = line;

		DrxFixedStringT<256> temp;

		temp.Format("%" PRISIZE_T "%s%s%u", size, m_name, m_fileName, m_line);

		m_uniqueId = DrxStringUtils::CalculateHash(temp.c_str());
	}

	inline size_t GetSize() const
	{
		return m_size;
	}

	inline tukk GetName() const
	{
		return m_name;
	}

	inline tukk GetFileName() const
	{
		return m_fileName;
	}

	inline u32 GetLine() const
	{
		return m_line;
	}

	inline u32 GetUniqueId() const
	{
		return m_uniqueId;
	}

	inline bool operator==(const CSharedParamsTypeInfo& right) const
	{
#if DEBUG_SHARED_PARAMS_TYPE_INFO
		if (m_uniqueId == right.m_uniqueId)
		{
			DRX_ASSERT(m_size == right.m_size);

			DRX_ASSERT(!strcmp(m_name, right.m_name));

			DRX_ASSERT(!strcmp(m_fileName, right.m_fileName));

			DRX_ASSERT(m_line == right.m_line);
		}
#endif //DEBUG_SHARED_PARAMS_TYPE_INFO

		return m_uniqueId == right.m_uniqueId;
	}

	inline bool operator!=(const CSharedParamsTypeInfo& right) const
	{
#if DEBUG_SHARED_PARAMS_TYPE_INFO
		if (m_uniqueId == right.m_uniqueId)
		{
			DRX_ASSERT(m_size == right.m_size);

			DRX_ASSERT(!strcmp(m_name, right.m_name));

			DRX_ASSERT(!strcmp(m_fileName, right.m_fileName));

			DRX_ASSERT(m_line == right.m_line);
		}
#endif //DEBUG_SHARED_PARAMS_TYPE_INFO

		return m_uniqueId != right.m_uniqueId;
	}

private:

	size_t m_size;

	char   m_name[64];
	char   m_fileName[64];

	u32 m_line;
	u32 m_uniqueId;
};

#undef DEBUG_SHARED_PARAMS_TYPE_INFO

#endif //__SHAREDPARAMSTYPEINFO_H__
