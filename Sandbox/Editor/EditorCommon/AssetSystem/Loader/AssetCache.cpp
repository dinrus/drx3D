// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "AssetCache.h"
#include "AssetLoader.h"
#include "AssetSystem/Asset.h"
#include "AssetSystem/EditableAsset.h"
#include "AssetSystem/AssetType.h"
#include <drx3D/CoreX/Serialization/IArchiveHost.h>

namespace AssetCache_Private
{

struct SCache
{
	enum EVersion { eVersion_Actual = 2 };

	SCache(std::vector<CAsset*>& assets) : assets(assets) {}
	void Serialize(Serialization::IArchive& ar)
	{
		i32 version = eVersion_Actual;
		ar(version, "version", "Version");
		if (version == eVersion_Actual)
		{
			ar(assets, "assets", "Assets");
		}
	}
	std::vector<CAsset*>& assets;
};

}

namespace AssetLoader
{

struct SDependencyInfoSerializer
{
	SDependencyInfoSerializer(SAssetDependencyInfo& info) : info(info) {}

	bool Serialize(yasli::Archive & ar)
	{
		ar(info.path, "path", "Path");
		ar(info.usageCount, "usageCount", "Usage Count");
		return true;
	}
	SAssetDependencyInfo& info;
};

struct SAssetSerializer
{
	SAssetSerializer(CAsset*& pAsset) : pAsset(pAsset) {}

	bool Serialize(yasli::Archive& ar)
	{
		if (ar.isInput())
		{
			string type;
			DrxGUID guid;
			ar(type, "type", "Type");
			if (!ar(guid, "guid", "Guid"))
			{
				guid = DrxGUID::Create();
			}
			pAsset = new CAsset(type, guid, "");
		}
		else
		{
			string type(pAsset->GetType()->GetTypeName());
			ar(type, "type", "Type");
			ar(pAsset->GetGUID(), "guid", "Guid");
		}

		DRX_ASSERT(pAsset);
		CAsset& asset = *pAsset;

		string name = asset.GetName();
		uint64 lastModifiedTime = asset.GetLastModifiedTime();
		std::vector<string> files(asset.GetFiles());
		string sourceFile(asset.GetSourceFile());
		string metadataFile(asset.GetMetadataFile());
		std::vector<std::pair<string, string>> details(asset.GetDetails());
		std::vector<SAssetDependencyInfo> dependencies(asset.GetDependencies());

		ar(name, "name", "Name");
		ar(lastModifiedTime, "timestamp", "Modification Time");
		ar(files, "files", "Files");
		ar(sourceFile, "source", "Source file"); // fbx, tiff, etc.
		ar(metadataFile, "metadata", "Drxasset file"); 
		ar(details, "details", "Details");
		ar(dependencies, "dependencies", "Dependencies");

		if (ar.isInput())
		{
			CEditableAsset editable(asset);
			editable.SetName(name);
			editable.SetLastModifiedTime(lastModifiedTime);
			editable.SetSourceFile(sourceFile);
			editable.SetMetadataFile(metadataFile);
			for (auto& f : files)
			{
				editable.AddFile(f);
			}
			editable.SetDetails(details);
			editable.SetDependencies(dependencies);
		}

		return true;
	}

	CAsset*& pAsset;
};

std::vector<CAsset*> LoadAssetsCached(tukk szCacheFilename, AssetLoaderFunction assetLoader)
{
	using namespace AssetCache_Private;

	std::vector<CAsset*> cache;
	if (!Serialization::LoadJsonFile(SCache(cache), szCacheFilename))
	{
		return assetLoader([](auto, auto)
		{
			return true;
		});
	}

	std::vector<bool> validAssets(cache.size(), false);

	auto loadingPredicate = [&cache, &validAssets](tukk szAssetFilename, uint64 timestamp)
	{
		// The cache is sorted by metadata filename.
		auto it = std::lower_bound(cache.begin(), cache.end(), szAssetFilename, [](const auto a, const auto b)
		{
			return stricmp(a->GetMetadataFile(), b) < 0;
		});

		if ((it != cache.end()) && (stricmp(szAssetFilename, (*it)->GetMetadataFile()) == 0) && ((*it)->GetLastModifiedTime() == timestamp))
		{
			const size_t i = it - cache.begin();
			validAssets[i] = true;
			return false;
		}
		return true;
	};

	std::vector<CAsset*> loaded = assetLoader(std::ref(loadingPredicate));

	for (size_t i = 0, n = cache.size(); i < n; ++i)
	{
		if (!validAssets[i])
		{
			delete cache[i];
			cache[i] = nullptr;
		}
	}
	cache.erase(std::remove(cache.begin(), cache.end(), nullptr), cache.end());

	cache.insert(cache.end(), loaded.begin(), loaded.end());
	return cache;
}

bool SaveCache(tukk szCacheFilename, std::vector<CAsset*>& assets)
{
	using namespace AssetCache_Private;
	// The cache file needs to be sorted to speed-up the LoadAssetsCached() call.
	std::sort(assets.begin(), assets.end(), [](const CAsset* a, const CAsset* b)
	{
		return stricmp(a->GetMetadataFile(), b->GetMetadataFile()) < 0;
	});

	return Serialization::SaveJsonFile(szCacheFilename, SCache(assets));
}

}

// Has to be placed into the same namespace as serialized type.
bool Serialize(yasli::Archive& ar, CAsset*& pAsset, tukk name, tukk label)
{
	using namespace AssetLoader;

	SAssetSerializer serializer(pAsset);
	return ar(serializer, name, label);
}

// Has to be placed into the same namespace as serialized type.
bool Serialize(yasli::Archive& ar, SAssetDependencyInfo& info, tukk name, tukk label)
{
	using namespace AssetLoader;

	SDependencyInfoSerializer serializer(info);
	return ar(serializer, name, label);
}

