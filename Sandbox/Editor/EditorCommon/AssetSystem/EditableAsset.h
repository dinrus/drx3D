// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Asset.h"
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

namespace AssetLoader
{
	struct SAssetSerializer;
	class CAssetFactory;
}

//! CEditableAsset modifies an asset.
//! Modification of assets is restricted to a small number of friend classes of CEditableAsset.
//! Everybody else is supposed to use the read-only CAsset interface.
class EDITOR_COMMON_API CEditableAsset
{
	friend class CAssetType;
	friend class CAssetBrowser;
	friend class CAssetEditor;
	friend class CAssetManager;
	friend class CAssetImportContext;
	friend class AssetLoader::CAssetFactory;
	friend struct AssetLoader::SAssetSerializer;

	// TODO: Remove when CLevelEditor is CAssetEditor.
	friend class CLevelEditor;

public:
	CEditableAsset(const CEditableAsset&);
	CEditableAsset& operator=(const CEditableAsset&) = delete;

	CAsset& GetAsset();

	void WriteToFile();
	void SetName(tukk szName);
	void SetLastModifiedTime(uint64 t);
	void SetMetadataFile(tukk szFilepath);
	void SetSourceFile(tukk szFilepath);
	void AddFile(tukk szFilepath);
	void SetFiles(tukk szCommonPath, const std::vector<string>& filenames);
	void SetDetails(const std::vector<std::pair<string, string>>& details);
	void SetDependencies(std::vector<SAssetDependencyInfo> dependencies);

	//! Invalidates thumbnail, so that subsequent calls to CAsset::StartLoadingThumbnail() will re-load
	//! the thumbnail from disk.
	void InvalidateThumbnail();

	void SetOpenedInAssetEditor(CAssetEditor* pEditor);

private:
	explicit CEditableAsset(CAsset& asset);

private:
	CAsset& m_asset;
};

