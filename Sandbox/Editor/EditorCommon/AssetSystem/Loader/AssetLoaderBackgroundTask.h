// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <functional>

class CAsset;

namespace AssetLoaderBackgroundTask
{
	// Run a background task to load assets from provided paths, then calls \p finalize in main thread passing the results.
	void Run(const std::vector<string>& assetRootPaths, std::function<void(const std::vector<CAsset*>&)> finalize);
};
