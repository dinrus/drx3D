// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

struct SLayerPakStats
{
	struct SEntry
	{
		string name;
		size_t nSize;
		string status;
		bool   bStreaming;
	};
	typedef std::vector<SEntry> TEntries;
	TEntries m_entries;

	size_t   m_MaxSize;
	size_t   m_UsedSize;
};

//! IResource manager interface.
struct IResourceUpr
{
	// <interfuscator:shuffle>
	virtual ~IResourceUpr(){}

	//! Called by level system to set the level folder.
	virtual void PrepareLevel(tukk sLevelFolder, tukk sLevelName) = 0;

	//! Called by level system after the level has been unloaded.
	virtual void UnloadLevel() = 0;

	//! Call to get current level resource list.
	virtual IResourceList* GetLevelResourceList() = 0;

	//! Load pak file from level cache to memory.
	//! SBindRoot is a path in virtual file system, where new pak will be mapper to (ex. LevelCache/mtl).
	virtual bool LoadLevelCachePak(tukk sPakName, tukk sBindRoot, bool bOnlyDuringLevelLoading = true) = 0;

	//! Unloads level cache pak file from memory.
	virtual void UnloadLevelCachePak(tukk sPakName) = 0;

	//! Loads the pak file for mode switching into memory e.g. Single player mode to Multiplayer mode.
	virtual bool LoadModeSwitchPak(tukk sPakName, const bool multiplayer) = 0;

	//! Unloads the mode switching pak file.
	virtual void UnloadModeSwitchPak(tukk sPakName, tukk sResourceListName, const bool multiplayer) = 0;

	//! Load general pak file to memory.
	virtual bool LoadPakToMemAsync(tukk pPath, bool bLevelLoadOnly) = 0;

	//! Unload all aync paks.
	virtual void UnloadAllAsyncPaks() = 0;

	//! Load pak file from active layer to memory.
	virtual bool LoadLayerPak(tukk sLayerName) = 0;

	//! Unloads layer pak file from memory if no more references.
	virtual void UnloadLayerPak(tukk sLayerName) = 0;

	//! Retrieve stats on the layer pak.
	virtual void GetLayerPakStats(SLayerPakStats& stats, bool bCollectAllStats) const = 0;

	//! Return time it took to load and precache the level.
	virtual CTimeValue GetLastLevelLoadTime() const = 0;

	virtual void       GetMemoryStatistics(IDrxSizer* pSizer) = 0;
	// </interfuscator:shuffle>
};

//! \endcond