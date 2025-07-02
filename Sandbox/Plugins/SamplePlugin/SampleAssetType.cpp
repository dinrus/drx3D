// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "SampleAssetType.h"

#include <AssetSystem/AssetEditor.h>
#include <FilePathUtil.h>

REGISTER_ASSET_TYPE(CSampleAssetType)

namespace Private_SampleAssetType
{

// For the sake of simplicity, we directly use the C file API in this sample.
// In a real-world scenario, however, you should prefer the IDrxPak interface.

bool CreateDataFile(tukk szFilePath)
{
	FILE* pFile;
	if (fopen_s(&pFile, szFilePath, "w"))
	{
		return false;
	}
	fprintf(pFile, "hello, world");
	fclose(pFile);
	return true;
}

} //endns Private_SampleAssetType

bool CSampleAssetType::OnCreate(CEditableAsset& editAsset, ukk /* pCreateParams */) const
{
	using namespace Private_SampleAssetType;

	const string basePath = PathUtil::RemoveExtension(PathUtil::RemoveExtension(editAsset.GetAsset().GetMetadataFile()));

	const string dataFilePath = basePath + ".txt";
	const string absoluteDataFilePath = PathUtil::Make(PathUtil::GetGameProjectAssetsPath(), dataFilePath);

	if (!CreateDataFile(absoluteDataFilePath))
	{
		return false;
	}

	editAsset.AddFile(dataFilePath);

	return true;
}

CAssetEditor* CSampleAssetType::Edit(CAsset* pAsset) const
{
	return CAssetEditor::OpenAssetForEdit("Sample Asset Editor", pAsset);
}
