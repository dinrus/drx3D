// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/ResFileLookupDataMan.h>
#include <drx3D/Render/ResFile.h>

#include <drx3D/Render/Shaders/Shader.h>

//////////////////////////////////////////////////////////////////////////

SResFileLookupDataDisk::SResFileLookupDataDisk(const struct SResFileLookupData& inLookup)
{
	m_NumOfFilesUnique = inLookup.m_NumOfFilesUnique;
	m_OffsetDir = inLookup.m_OffsetDir;
	m_CRC32 = inLookup.m_CRC32;
	m_CacheMajorVer = inLookup.m_CacheMajorVer;
	m_CacheMinorVer = inLookup.m_CacheMinorVer;
}

#ifdef USE_PARTIAL_ACTIVATION
u32 SResFileLookupData::GetDirOffset(
  const CDrxNameTSCRC& dirEntryName) const
{
	if (m_resdirlookup.size() == 0) return 0;

	/*
	   if (m_resdirlookup.size() > 0)
	   {
	   char acTmp[1024];
	   drx_sprintf(acTmp, "dir values: ");
	   OutputDebugString(acTmp);
	   for (u32 ui = 0; ui < m_resdirlookup.size(); ++ui)
	   {
	    drx_sprintf(acTmp, " %u ", m_resdirlookup[ui]);
	    OutputDebugString(acTmp);
	   }
	   drx_sprintf(acTmp, " \n");
	   OutputDebugString(acTmp);
	   }
	 */

	u32 uiOffset = 0;
	for (; uiOffset < m_resdirlookup.size() - 1; ++uiOffset)
	{
		if (m_resdirlookup[uiOffset + 1] > dirEntryName)
			break;
	}

	return uiOffset;
}
#endif

//////////////////////////////////////////////////////////////////////////

CResFileLookupDataMan::CResFileLookupDataMan() : m_TotalDirStored(0)
{
	m_bDirty = false;
	m_bReadOnly = true;

	m_VersionInfo.m_ResVersion = RES_COMPRESSION;

	float fVersion = (float)FX_CACHE_VER;
	drx_sprintf(m_VersionInfo.m_szCacheVer, "Ver: %.1f", fVersion);
}

CResFileLookupDataMan::~CResFileLookupDataMan()
{
	if (m_bDirty)
		Flush();
	Clear();
}

CDrxNameTSCRC CResFileLookupDataMan::AdjustName(tukk szName)
{
	char acTmp[1024];
	i32 userPathSize = gRenDev->m_cEF.m_szUserPath.size();
	i32 enginePathSize = strlen("%ENGINE%/");

	if (!strnicmp(szName, gRenDev->m_cEF.m_szUserPath.c_str(), userPathSize))
		drx_strcpy(acTmp, &szName[userPathSize]);
	else if (!strnicmp(szName, "%ENGINE%/", enginePathSize))
		drx_strcpy(acTmp, &szName[enginePathSize]);
	else if (!strnicmp(szName, "Levels", 6))
	{
		tukk acNewName = strstr(szName, "ShaderCache");
		assert(acNewName);
		PREFAST_ASSUME(acNewName);
		drx_strcpy(acTmp, acNewName);
	}
	else
		drx_strcpy(acTmp, szName);

	return acTmp;
}

//////////////////////////////////////////////////////////////////////////

void CResFileLookupDataMan::Clear()
{
	m_Data.clear();
}

void CResFileLookupDataMan::Flush()
{
	if (!m_bDirty)
		return;
	SaveData(m_Path.c_str(), CParserBin::m_bEndians);
	m_bDirty = false;
}

bool CResFileLookupDataMan::LoadData(
  tukk acFilename, bool bSwapEndianRead, bool bReadOnly)
{
	m_Path = acFilename;

	m_bReadOnly = bReadOnly;

	i32 nFlags = bReadOnly ? 0 : IDrxPak::FLAGS_NEVER_IN_PAK | IDrxPak::FLAGS_PATH_REAL | IDrxPak::FOPEN_ONDISK;

	FILE* handle = gEnv->pDrxPak->FOpen(acFilename, "rb", nFlags);
	if (handle == 0)
		return false;

	u32 hid;

	SVersionInfo versionInfo;

	gEnv->pDrxPak->FReadRaw(&hid, sizeof(u32), 1, handle);
	gEnv->pDrxPak->FReadRaw(&versionInfo, sizeof(SVersionInfo), 1, handle);

	if (bSwapEndianRead)
	{
		SwapEndian(hid, eBigEndian);
		SwapEndian(versionInfo, eBigEndian);
	}

	if (hid != IDRESHEADER)
	{
		gEnv->pDrxPak->FClose(handle);
		return false;
	}
	if (versionInfo.m_ResVersion != RESVERSION_LZSS && versionInfo.m_ResVersion != RESVERSION_LZMA && versionInfo.m_ResVersion != RESVERSION_DEBUG)
	{
		gEnv->pDrxPak->FClose(handle);
		return false;
	}

	float fVersion = (float)FX_CACHE_VER;
	char cacheVer[16];
	drx_sprintf(cacheVer, "Ver: %.1f", fVersion);
	if (strcmp(cacheVer, versionInfo.m_szCacheVer))
	{
		gEnv->pDrxPak->FClose(handle);
		return false;
	}

	m_VersionInfo = versionInfo;

	u32 uiCount;
	gEnv->pDrxPak->FReadRaw(&uiCount, sizeof(u32), 1, handle);

	if (bSwapEndianRead)
	{
		SwapEndian(uiCount, eBigEndian);
	}

	CDrxNameTSCRC name;
	CDirEntry dirEntry;
	CDrxNameTSCRC dirEntryName;
	u32 ui;
	for (ui = 0; ui < uiCount; ++ui)
	{
		gEnv->pDrxPak->FReadRaw(&name, sizeof(CDrxNameTSCRC), 1, handle);
		if (bSwapEndianRead)
		{
			SwapEndian(name, eBigEndian);
		}

		SResFileLookupDataDisk dirDataDisk;

#ifndef USE_PARTIAL_ACTIVATION

		gEnv->pDrxPak->FReadRaw(&dirDataDisk, sizeof(SResFileLookupDataDisk), 1, handle);
		if (bSwapEndianRead)
		{
			SwapEndian(dirDataDisk, eBigEndian);
		}
		SResFileLookupData dirData(dirDataDisk);

#else

		gEnv->pDrxPak->FReadRaw(&dirData.m_NumOfFilesUnique, sizeof(i32), 1, handle);
		gEnv->pDrxPak->FReadRaw(&dirData.m_NumOfFilesRef, sizeof(i32), 1, handle);
		gEnv->pDrxPak->FReadRaw(&dirData.m_OffsetDir, sizeof(u32), 1, handle);
		gEnv->pDrxPak->FReadRaw(&dirData.m_CRC32, sizeof(u32), 1, handle);

		if (bSwapEndianRead)
		{
			SwapEndian(dirData.m_NumOfFilesUnique, eBigEndian);
			SwapEndian(dirData.m_NumOfFilesRef, eBigEndian);
			SwapEndian(dirData.m_OffsetDir, eBigEndian);
			SwapEndian(dirData.m_CRC32, eBigEndian);
		}

		gEnv->pDrxPak->FReadRaw(&dirData.m_ContainsResDir, sizeof(bool), 1, handle);
		u32 uiDirSize;
		gEnv->pDrxPak->FReadRaw(&uiDirSize, sizeof(u32), 1, handle);
		if (bSwapEndianRead)
		{
			SwapEndian(uiDirSize, eBigEndian);
		}
		if (dirData.m_ContainsResDir)
		{
			dirData.m_resdir.reserve(uiDirSize);
			for (u32 uj = 0; uj < uiDirSize; ++uj)
			{
				gEnv->pDrxPak->FReadRaw(&dirEntry, sizeof(CDirEntry), 1, handle);
				if (bSwapEndianRead)
				{
					SwapEndian(dirEntry, eBigEndian);
				}
				dirData.m_resdir.push_back(dirEntry);
			}
		}
		else
		{
			dirData.m_resdirlookup.reserve(uiDirSize);
			for (u32 uj = 0; uj < uiDirSize; ++uj)
			{
				gEnv->pDrxPak->FReadRaw(&dirEntryName, sizeof(CDrxNameTSCRC), 1, handle);
				if (bSwapEndianRead)
				{
					SwapEndian(dirEntryName, eBigEndian);
				}
				dirData.m_resdirlookup.push_back(dirEntryName);
			}
		}
#endif

		m_Data[name] = dirData;
	}

	gEnv->pDrxPak->FReadRaw(&uiCount, sizeof(u32), 1, handle);
	if (bSwapEndianRead)
		SwapEndian(uiCount, eBigEndian);
	for (ui = 0; ui < uiCount; ++ui)
	{
		gEnv->pDrxPak->FReadRaw(&name, sizeof(CDrxNameTSCRC), 1, handle);
		if (bSwapEndianRead)
			SwapEndian(name, eBigEndian);

		SCFXLookupData CFXData;
		gEnv->pDrxPak->FReadRaw(&CFXData, sizeof(CFXData), 1, handle);
		if (bSwapEndianRead)
			SwapEndian(CFXData, eBigEndian);

		m_CFXData[name] = CFXData;
	}

	gEnv->pDrxPak->FClose(handle);

	return true;
}

void CResFileLookupDataMan::SaveData(
  tukk acFilename, bool bSwapEndianWrite) const
{
	// ignore invalid file access for lookupdata because it shouldn't be written
	// when shaders no compile is set anyway
	SCOPED_ALLOW_FILE_ACCESS_FROM_THIS_THREAD();

	FILE* handle = gEnv->pDrxPak->FOpen(acFilename, "w+b");
	if (handle == 0)
		return;

	u32 hid = IDRESHEADER;

	SVersionInfo versionInfo;
	versionInfo.m_ResVersion = RES_COMPRESSION;

	float fVersion = (float)FX_CACHE_VER;
	drx_sprintf(versionInfo.m_szCacheVer, "Ver: %.1f", fVersion);

	if (bSwapEndianWrite)
	{
		SwapEndian(hid, eBigEndian);
		SwapEndian(versionInfo, eBigEndian);
	}

	gEnv->pDrxPak->FWrite(&hid, sizeof(u32), 1, handle);
	gEnv->pDrxPak->FWrite(&versionInfo, sizeof(SVersionInfo), 1, handle);

	u32 uiCount = m_Data.size();
	if (bSwapEndianWrite)
	{
		SwapEndian(uiCount, eBigEndian);
	}
	gEnv->pDrxPak->FWrite(&uiCount, sizeof(u32), 1, handle);

	for (TFileResDirDataMap::const_iterator it = m_Data.begin(); it != m_Data.end(); ++it)
	{
		CDrxNameTSCRC name = it->first;
		if (bSwapEndianWrite)
		{
			SwapEndian(name, eBigEndian);
		}
		gEnv->pDrxPak->FWrite(&name, sizeof(CDrxNameTSCRC), 1, handle);

#ifndef USE_PARTIAL_ACTIVATION
		SResFileLookupDataDisk header(it->second);
		if (bSwapEndianWrite)
		{
			SwapEndian(header, eBigEndian);
		}
		gEnv->pDrxPak->FWrite(&header, sizeof(SResFileLookupDataDisk), 1, handle);
#else
		const SResFileLookupData& header = it->second;
		i32 numOfFilesUnique = header.m_NumOfFilesUnique;
		i32 numOfFilesRef = header.m_NumOfFilesRef;
		u32 offsetDir = header.m_OffsetDir;
		u32 crc = header.m_CRC32;
		if (bSwapEndianWrite)
		{
			SwapEndian(numOfFilesUnique, eBigEndian);
			SwapEndian(numOfFilesRef, eBigEndian);
			SwapEndian(offsetDir, eBigEndian);
			SwapEndian(crc, eBigEndian);
		}
		gEnv->pDrxPak->FWrite(&numOfFilesUnique, sizeof(i32), 1, handle);
		gEnv->pDrxPak->FWrite(&numOfFilesRef, sizeof(i32), 1, handle);
		gEnv->pDrxPak->FWrite(&offsetDir, sizeof(u32), 1, handle);
		gEnv->pDrxPak->FWrite(&crc, sizeof(u32), 1, handle);

		gEnv->pDrxPak->FWrite(&header.m_ContainsResDir, sizeof(bool), 1, handle);

		if (header.m_ContainsResDir)
		{
			u32 uiDirSize = header.m_resdir.size();
			if (bSwapEndianWrite)
			{
				SwapEndian(uiDirSize, eBigEndian);
			}
			gEnv->pDrxPak->FWrite(&uiDirSize, sizeof(u32), 1, handle);

			for (ResDir::const_iterator it2 = header.m_resdir.begin(); it2 != header.m_resdir.end(); ++it2)
			{
				CDirEntry dirEntry = *it2;
				if (bSwapEndianWrite)
				{
					SwapEndian(dirEntry, eBigEndian);
				}
				gEnv->pDrxPak->FWrite(&dirEntry, sizeof(CDirEntry), 1, handle);
			}
		}
		else
		{
			u32 uiDirSize = header.m_resdirlookup.size();
			if (bSwapEndianWrite)
			{
				SwapEndian(uiDirSize, eBigEndian);
			}
			gEnv->pDrxPak->FWrite(&uiDirSize, sizeof(u32), 1, handle);

			for (TResDirNames::const_iterator it2 = header.m_resdirlookup.begin(); it2 != header.m_resdirlookup.end(); ++it2)
			{
				CDrxNameTSCRC dirEntryName = *it2;
				if (bSwapEndianWrite)
				{
					SwapEndian(dirEntryName, eBigEndian);
				}
				gEnv->pDrxPak->FWrite(&dirEntryName, sizeof(CDrxNameTSCRC), 1, handle);
			}
		}
#endif
	}

	uiCount = m_CFXData.size();
	if (bSwapEndianWrite)
		SwapEndian(uiCount, eBigEndian);
	gEnv->pDrxPak->FWrite(&uiCount, sizeof(u32), 1, handle);
	for (TFileCFXDataMap::const_iterator it = m_CFXData.begin(); it != m_CFXData.end(); ++it)
	{
		CDrxNameTSCRC name = it->first;
		if (bSwapEndianWrite)
			SwapEndian(name, eBigEndian);
		gEnv->pDrxPak->FWrite(&name, sizeof(CDrxNameTSCRC), 1, handle);
		SCFXLookupData CFXData = it->second;
		if (bSwapEndianWrite)
			SwapEndian(CFXData, eBigEndian);
		gEnv->pDrxPak->FWrite(&CFXData, sizeof(SCFXLookupData), 1, handle);
	}

	gEnv->pDrxPak->FClose(handle);
}

void CResFileLookupDataMan::AddData(const CResFile* pResFile, u32 CRC)
{
	if (pResFile == 0)
		return;

	SResFileLookupData data;

	// store the dir data
	data.m_OffsetDir = pResFile->m_nOffsDir;
	data.m_NumOfFilesUnique = pResFile->m_nNumFilesUnique;

	float fVersion = (float)FX_CACHE_VER;
	u32 nMinor = (i32)(((float)fVersion - (float)(i32)fVersion) * 10.1f);
	u32 nMajor = (i32)fVersion;
	data.m_CacheMinorVer = nMinor;
	data.m_CacheMajorVer = nMajor;

	data.m_CRC32 = CRC;

#ifdef USE_PARTIAL_ACTIVATION
	// create the lookup data for the resdir
	if (pResFile->m_Dir.size() < 128)
	{
		data.m_ContainsResDir = true;

		m_TotalDirStored++;

		// just copy the whole vector
		data.m_resdir = pResFile->m_Dir;
		/*
		   data.m_resdir.reserve(pResFile->m_Dir.size());
		   for (u32 ui = 0; ui < pResFile->m_Dir.size(); ++ui)
		   {
		   data.m_resdir.push_back(pResFile->m_Dir[0]);
		   }
		   memcpy(&data.m_resdir[0], &pResFile->m_Dir[0], sizeof(CDirEntry) * pResFile->m_Dir.size());
		 */
	}
	else
	{
		data.m_ContainsResDir = false;

		u32 entries = 0;
		u32 entriesPerSlice = MAX_DIR_BUFFER_SIZE / sizeof(CDirEntry);
		while ((entries * entriesPerSlice) < pResFile->m_Dir.size())
		{
			data.m_resdirlookup.push_back(pResFile->m_Dir[entries * entriesPerSlice].Name);
			entries++;
		}
	}
#endif

	tukk acOrigFilename = pResFile->mfGetFileName();
	AddDataCFX(acOrigFilename, CRC);

	// remove the user info, if available
	CDrxNameTSCRC name = AdjustName(acOrigFilename);

	// simply overwrite the data if it was already in here
	m_Data[name] = data;
}

void CResFileLookupDataMan::AddDataCFX(tukk acOrigFilename, u32 CRC)
{
	char nm[256], nmDir[512];
	_splitpath(acOrigFilename, NULL, nmDir, nm, NULL);
	{
		tuk s = strchr(nm, '@');
		//assert(s);
		if (s)
			s[0] = 0;
	}

	CDrxNameTSCRC nameCFX = nm;
	SCFXLookupData CFX;
	CFX.m_CRC32 = CRC;
	m_CFXData[nameCFX] = CFX;
}

void CResFileLookupDataMan::RemoveData(u32 CRC)
{
	{
		TFileResDirDataMap tmpData;
		for (TFileResDirDataMap::iterator it = m_Data.begin(); it != m_Data.end(); ++it)
		{
			SResFileLookupData& data = it->second;
			if (data.m_CRC32 != CRC)
				tmpData[it->first] = data;
		}
		m_Data.swap(tmpData);
	}

	{
		TFileCFXDataMap tmpData;
		for (TFileCFXDataMap::iterator it = m_CFXData.begin(); it != m_CFXData.end(); ++it)
		{
			SCFXLookupData& data = it->second;
			if (data.m_CRC32 != CRC)
				tmpData[it->first] = data;
		}
		m_CFXData.swap(tmpData);
	}
}

SResFileLookupData* CResFileLookupDataMan::GetData(
  const CDrxNameTSCRC& name)
{
	TFileResDirDataMap::iterator it = m_Data.find(name);
	if (it == m_Data.end())
		return 0;

	return &it->second;
}

//////////////////////////////////////////////////////////////////////////
