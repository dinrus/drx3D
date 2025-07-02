// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _DRX_SYSTEM_PAK_VARS_HDR_
#define _DRX_SYSTEM_PAK_VARS_HDR_

enum EPakPriority
{
	ePakPriorityFileFirst         = 0,
	ePakPriorityPakFirst          = 1,
	ePakPriorityPakOnly           = 2,
	ePakPriorityFileFirstModsOnly = 3,
};

// variables that control behaviour of DrxPak/StreamEngine subsystems
struct PakVars
{
	i32 nSaveLevelResourceList;
	i32 nSaveFastloadResourceList;
	i32 nSaveTotalResourceList;
	i32 nSaveMenuCommonResourceList;
	i32 nLogMissingFiles;
	i32 nLoadCache;
	i32 nLoadModePaks;
	i32 nReadSlice;
	i32 nPriority;
	i32 nMessageInvalidFileAccess;
	i32 nValidateFileHashes;
	i32 nTotalInMemoryPakSizeLimit;
	i32 nStreamCache;
	i32 nInMemoryPerPakSizeLimit;
	i32 nLogInvalidFileAccess;
	i32 nLoadFrontendShaderCache;
	i32 nUncachedStreamReads;
#ifndef _RELEASE
	i32 nLogAllFileAccess;
#endif
	i32 nDisableNonLevelRelatedPaks;

	PakVars()
		: nPriority(0)
		, nReadSlice(0)
		, nLogMissingFiles(0)
		, nInMemoryPerPakSizeLimit(0)
		, nSaveTotalResourceList(0)
		, nSaveFastloadResourceList(0)
		, nSaveMenuCommonResourceList(0)
		, nSaveLevelResourceList(0)
		, nValidateFileHashes(0)
		, nUncachedStreamReads(1)
	{
		nInMemoryPerPakSizeLimit = 6;    // 6 Megabytes limit
		nTotalInMemoryPakSizeLimit = 30; // Megabytes

		nLoadCache = 0;       // Load in memory paks from _FastLoad folder
		nLoadModePaks = 0;    // Load menucommon/gamemodeswitch paks
		nStreamCache = 0;

#if defined(_RELEASE)
		nPriority = ePakPriorityPakOnly;  // Only read from pak files by default
#else
		nPriority = ePakPriorityFileFirst;
#endif

		nMessageInvalidFileAccess = 0;
		nLogInvalidFileAccess = 0;

#ifndef _RELEASE
		nLogAllFileAccess = 0;
#endif

#if defined(_DEBUG)
		nValidateFileHashes = 1;
#endif

		nLoadFrontendShaderCache = 0;
		nDisableNonLevelRelatedPaks = 1;
	}
};

#endif
