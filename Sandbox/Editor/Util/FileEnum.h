// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
//////////////////////////////////////////////////////////////////////////
//  DrxENGINE Source File
//  Copyright (C) 2000-2012, DinrusPro 3D GmbH, All rights reserved
//////////////////////////////////////////////////////////////////////////

class CFileEnum
{
public:
	CFileEnum();
	virtual ~CFileEnum();
	bool        GetNextFile(struct __finddata64_t* pFile);
	bool        StartEnumeration(tukk szEnumPathAndPattern, __finddata64_t* pFile);
	bool        StartEnumeration(tukk szEnumPath, tukk szEnumPattern, __finddata64_t* pFile);
	static bool ScanDirectory(
	  const string& path,
	  const string& file,
	  std::vector<string>& files,
	  bool bRecursive = true,
	  bool bSkipPaks = false);

protected:
	intptr_t m_hEnumFile;
};

