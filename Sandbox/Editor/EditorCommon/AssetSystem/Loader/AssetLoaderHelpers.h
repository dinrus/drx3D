// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/String/DrxString.h>
#include "AssetSystem/EditableAsset.h"
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

class CAsset;

namespace AssetLoader
{

//! \brief Returns the name of the asset with path \p szPath.
//! If \p szPath is a path to a file, the function returns the filename without extension.
//! If it is path to a directory instead, everything but the last directory is removed.
//! Examples:
//! GetAssetName("Assets/Models/Monster.cgf") = "Monster"
//! GetAssetName("Assets/Models/Monster.bak.cgf") = "Monster"
//! GetAssetName("Assets/Levels/Woods") = "Woods"
string GetAssetName(tukk szPath);

class EDITOR_COMMON_API CAssetFactory
{
public:
	//! Reads xml with metadata from memory.
	//! \param szAssetPath Must be relative to the assets root. 
	static CAsset* LoadAssetFromXmlFile(tukk szAssetPath);

	//! Reads xml with metadata from memory.
	//! \param szAssetPath Must be relative to the assets root. 
	static CAsset* LoadAssetFromInMemoryXmlFile(tukk szAssetPath, uint64 timestamp, tukk pBuffer, size_t numberOfBytes);

	//! Loads assets from pak file.
	//! \param szArchivePath the path should be relative to the project root folder. The pak must be opened by one of the IDrxPak::OpenPack(s) methods.
	//! \param Unary predicate which returns true for the required assets.
	static std::vector<CAsset*> LoadAssetsFromPakFile(tukk szArchivePath, std::function<bool(tukk szAssetRelativePath, uint64 timestamp)> predicate);
};

} //endns AssetLoader

