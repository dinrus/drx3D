// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __TINY_FILE_ENUM_H__
#define __TINY_FILE_ENUM_H__

#pragma once

#include <io.h>

class _TinyFileEnum
{
public:
	_TinyFileEnum() { m_hEnumFile = -1L; };

	virtual ~_TinyFileEnum()
	{
		if (m_hEnumFile != -1L)
		{
			_findclose(m_hEnumFile);
			m_hEnumFile = -1L;
		}
	};

	bool GetNextFile(struct __finddata64_t* pFile)
	{
		if (_findnext64(m_hEnumFile, pFile) == -1L)
		{
			_findclose(m_hEnumFile);
			m_hEnumFile = -1L;
			return false;
		}
		;
		return true;
	}

	bool StartEnumeration(tukk pszEnumPathAndPattern, __finddata64_t* pFile)
	{
		if (m_hEnumFile != -1L)
		{
			_findclose(m_hEnumFile);
			m_hEnumFile = -1L;
		}
		if ((m_hEnumFile = _findfirst64(pszEnumPathAndPattern, pFile)) == -1L)
		{
			_findclose(m_hEnumFile);
			m_hEnumFile = -1L;
			return false;
		}
		return true;
	}

	bool StartEnumeration(tuk pszEnumPath, tuk pszEnumPattern, __finddata64_t* pFile)
	{
		char szPath[_MAX_PATH];
		drx_strcpy(szPath, pszEnumPath);
		if (szPath[strlen(szPath)] != '\\' &&
		    szPath[strlen(szPath)] != '/')
		{
			drx_strcat(szPath, "\\");
		}
		drx_strcat(szPath, pszEnumPattern);
		m_hEnumFile = _findfirst64(szPath, pFile);
		return m_hEnumFile != -1;
	}

protected:
	intptr_t m_hEnumFile;
};

#endif
