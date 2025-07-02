// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RESFILELOOKUPDATAMAN_H__
#define __RESFILELOOKUPDATAMAN_H__

#include <drx3D/Render/ResFile.h>

typedef std::vector<CDrxNameTSCRC> TResDirNames;

struct SResFileLookupDataDisk
{
	i32    m_NumOfFilesUnique;
	u32 m_OffsetDir;
	u32 m_CRC32;
	u16 m_CacheMajorVer;
	u16 m_CacheMinorVer;

	SResFileLookupDataDisk() {}
	SResFileLookupDataDisk(const struct SResFileLookupData& inLookup);

	AUTO_STRUCT_INFO;
};
struct SResFileLookupData
{
	i32    m_NumOfFilesUnique;
	u32 m_OffsetDir;
	u32 m_CRC32;
	u16 m_CacheMajorVer;
	u16 m_CacheMinorVer;

	SResFileLookupData() {}
	SResFileLookupData(const SResFileLookupDataDisk& inLookup)
		: m_NumOfFilesUnique(inLookup.m_NumOfFilesUnique)
		, m_OffsetDir(inLookup.m_OffsetDir)
		, m_CRC32(inLookup.m_CRC32)
		, m_CacheMajorVer(inLookup.m_CacheMajorVer)
		, m_CacheMinorVer(inLookup.m_CacheMinorVer)
#ifdef USE_PARTIAL_ACTIVATION
		, m_ContainsResDir(false)
#endif
	{
	}

#ifdef USE_PARTIAL_ACTIVATION
	bool         m_ContainsResDir;

	TResDirNames m_resdirlookup;
	ResDir       m_resdir;

	u32 GetDirOffset(const CDrxNameTSCRC& dirEntryName) const;
#endif
};

struct SCFXLookupData
{
	u32 m_CRC32;
	SCFXLookupData() {}
	AUTO_STRUCT_INFO;
};

#define MAX_DIR_BUFFER_SIZE 300000

typedef std::map<CDrxNameTSCRC, SResFileLookupData> TFileResDirDataMap;
typedef std::map<CDrxNameTSCRC, SCFXLookupData>     TFileCFXDataMap;

class CResFile;

struct SVersionInfo
{
	SVersionInfo() : m_ResVersion(0)
	{ memset(m_szCacheVer, 0, sizeof(m_szCacheVer)); }

	i32  m_ResVersion;
	char m_szCacheVer[16];

	AUTO_STRUCT_INFO;
};

class CResFileLookupDataMan
{
public:
	CResFileLookupDataMan();
	~CResFileLookupDataMan();

	void                Clear();
	void                Flush();

	CDrxNameTSCRC       AdjustName(tukk szName);
	i32                 GetResVersion() const { return m_VersionInfo.m_ResVersion; }

	bool                LoadData(tukk acFilename, bool bSwapEndianRead, bool bReadOnly);
	void                SaveData(tukk acFilename, bool bSwapEndianWrite) const;

	bool                IsReadOnly() { return m_bReadOnly; }

	void                AddData(const CResFile* pResFile, u32 CRC);
	void                AddDataCFX(tukk szPath, u32 CRC);
	void                RemoveData(u32 CRC);

	SResFileLookupData* GetData(const CDrxNameTSCRC& name);
	void                MarkDirty(bool bDirty) { m_bDirty = bDirty; }

protected:

	string             m_Path;
	SVersionInfo       m_VersionInfo;
	TFileResDirDataMap m_Data;
	TFileCFXDataMap    m_CFXData;
	u32       m_TotalDirStored;
	byte               m_bDirty    : 1;
	byte               m_bReadOnly : 1;
};

#endif //  __RESFILELOOKUPDATAMAN_H__
