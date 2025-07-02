// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CAsset;

namespace AssetLoader
{

	//! Unary predicate which returns true for the required assets. 
	//! \param szAssetRelativePath Path relative to assets root.
	typedef std::function<bool(tukk szAssetRelativePath, uint64 timestamp)> Predicate;

	// Should load assets for which predicate returns true.
	typedef std::function<std::vector<CAsset*>(Predicate predicate)> AssetLoaderFunction;

	// Loads assets, using the specified cache file to speed up the loading time.
	std::vector<CAsset*> LoadAssetsCached(tukk szCacheFilename, AssetLoaderFunction assetLoader);

	// Saves collection of assets as a cache file.
	bool SaveCache(tukk szCacheFilename, std::vector<CAsset*>& assets);
};
