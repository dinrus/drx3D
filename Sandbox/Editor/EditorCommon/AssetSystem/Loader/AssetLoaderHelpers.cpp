// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "AssetLoaderHelpers.h"
#include "Metadata.h"
#include <AssetSystem/Asset.h>
#include <AssetSystem/EditableAsset.h>

#include <drx3D/Sys/ISystem.h>
#include <Drx3DEngine/I3DEngine.h>
#include <Drx3DEngine/CGF/IChunkFile.h>  
#include <Drx3DEngine/CGF/CGFContent.h>
#include <drx3D/CoreX/String/DrxPath.h>

namespace AssetLoader
{

string GetAssetName(tukk szPath)
{
	tukk szExt = nullptr;
	tukk szFile = szPath;
	for (; *szPath; ++szPath)
	{
		// Asset name begins with the position past the last directory separator.
		if (*szPath == '/' || *szPath == '\\')
		{
			szFile = szPath;
			++szFile;
			szExt = nullptr;
		}

		// Asset name ends at the position of the first extension separator.
		if (!szExt && *szPath == '.')
		{
			szExt = szPath;
		}
	}

	return szExt ? string(szFile, szExt - szFile) : string(szFile);
}

CAsset* CAssetFactory::LoadAssetFromXmlFile(tukk szAssetPath)
{
	IDrxPak* const pPak = GetISystem()->GetIPak();
	FILE* pFile = pPak->FOpen(szAssetPath, "rbx");
	if (!pFile)
	{
		return nullptr;
	}

	std::vector<char> buffer(pPak->FGetSize(pFile));
	const uint64 timestamp = pPak->GetModificationTime(pFile);
	const size_t numberOfBytesRead = pPak->FReadRawAll(buffer.data(), buffer.size(), pFile);
	pPak->FClose(pFile);

	return CAssetFactory::LoadAssetFromInMemoryXmlFile(szAssetPath, timestamp, buffer.data(), numberOfBytesRead);
}

CAsset* CAssetFactory::LoadAssetFromInMemoryXmlFile(tukk szAssetPath, uint64 timestamp, tukk pBuffer, size_t numberOfBytes)
{
	AssetLoader::SAssetMetadata metadata;
	{
		XmlNodeRef container = GetISystem()->LoadXmlFromBuffer(pBuffer, numberOfBytes);
		if (!container || !AssetLoader::ReadMetadata(container, metadata))
		{
			return nullptr;
		}
	}

	const string path = PathUtil::GetPathWithoutFilename(szAssetPath);

	DrxGUID guid = (metadata.guid != DrxGUID::Null()) ? metadata.guid : DrxGUID::Create();

	CAsset* const pAsset = new CAsset(metadata.type, guid, GetAssetName(szAssetPath));

	CEditableAsset editableAsset(*pAsset);
	editableAsset.SetLastModifiedTime(timestamp);
	editableAsset.SetFiles(path, metadata.files);
	editableAsset.SetMetadataFile(szAssetPath);
	editableAsset.SetDetails(metadata.details);

	// Source file may be next to the asset or relative to the assets root directory.
	if (!metadata.sourceFile.empty() && !strpbrk(metadata.sourceFile.c_str(), "/\\"))
	{
		metadata.sourceFile = PathUtil::Make(path, metadata.sourceFile);
	}
	editableAsset.SetSourceFile(metadata.sourceFile);

	// Make dependency paths relative to the assets root directory.
	{
		std::vector<SAssetDependencyInfo> dependencies;
		std::transform(metadata.dependencies.begin(), metadata.dependencies.end(), std::back_inserter(dependencies), [&path](const auto& x) -> SAssetDependencyInfo
		{
			if (strncmp("./", x.first.c_str(), 2) == 0)
			{
				return { PathUtil::Make(path.c_str(), x.first.c_str() + 2), x.second };
			}
			else
			{
				return { x.first, x.second };
			}
		});

		// This is a quick solution to fix dependency lookup for tif files.
		// TODO: Drxasset file should never have dependencies on source files of other assets.
		for (auto& item : dependencies)
		{
			if (stricmp(PathUtil::GetExt(item.path.c_str()), "tif") == 0)
			{
				item.path = PathUtil::ReplaceExtension(item.path.c_str(), "dds");
			}
		}

		editableAsset.SetDependencies(dependencies);
	}

	return pAsset;

}

std::vector<CAsset*> CAssetFactory::LoadAssetsFromPakFile(tukk szArchivePath, std::function<bool(tukk szAssetRelativePath, uint64 timestamp)> predicate)
{
	std::vector<CAsset*> assets;
	assets.reserve(1024);

	std::vector<char> buffer(1 << 24);

	std::stack<string> stack;
	stack.push("");
	while (!stack.empty())
	{
		const DrxPathString mask = PathUtil::Make(stack.top(), "*");
		const DrxPathString folder = stack.top();
		stack.pop();

		GetISystem()->GetIPak()->ForEachArchiveFolderEntry(szArchivePath, mask, [&stack, &folder, &buffer, &assets, predicate](const IDrxPak::ArchiveEntryInfo& entry)
		{
			const DrxPathString path(PathUtil::Make(folder.c_str(), entry.szName));
			if (entry.bIsFolder)
			{
				stack.push(path);
				return;
			}

			if (stricmp(PathUtil::GetExt(path), "cryasset") != 0)
			{
				return;
			}

			IDrxPak* const pPak = GetISystem()->GetIPak();

			auto closeFile = [pPak](FILE* f) { return pPak->FClose(f); };
			std::unique_ptr<FILE, decltype(closeFile)> file(pPak->FOpen(path, "rbx"), closeFile);
			if (!file)
			{
				return;
			}

			const uint64 timestamp = pPak->GetModificationTime(file.get());
			if (!predicate(path.c_str(), timestamp))
			{
				return;
			}

			buffer.resize(pPak->FGetSize(file.get()));
			const size_t numberOfBytesRead = pPak->FReadRawAll(buffer.data(), buffer.size(), file.get());
			CAsset* pAsset = LoadAssetFromInMemoryXmlFile(path.c_str(), timestamp, buffer.data(), numberOfBytesRead);
			if (pAsset)
			{
				assets.push_back(pAsset);
			}
		});
	}

	return assets;
}

} //endns AssetLoader

