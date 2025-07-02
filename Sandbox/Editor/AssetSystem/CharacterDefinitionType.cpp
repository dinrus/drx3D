// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "CharacterDefinitionType.h"
#include "QT/Widgets/QPreviewWidget.h"
#include "AssetSystem/Asset.h"
#include "FilePathUtil.h"
#include <ThreadingUtils.h>

REGISTER_ASSET_TYPE(CCharacterDefinitionType)

DrxIcon CCharacterDefinitionType::GetIconInternal() const
{
	return DrxIcon("icons:common/assets_character.ico");
}

void CCharacterDefinitionType::GenerateThumbnail(const CAsset* pAsset) const
{
	ThreadingUtils::PostOnMainThread([=]()
	{
		CMaterialManager* pManager = GetIEditorImpl()->GetMaterialManager();
	if (!pManager)
		return;

	DRX_ASSERT(0 < pAsset->GetFilesCount());
	if (1 > pAsset->GetFilesCount())
		return;

	tukk characterFileName = pAsset->GetFile(0);
	if (!characterFileName || !*characterFileName)
		return;

	const string thumbnailFileName = PathUtil::Make(PathUtil::GetGameProjectAssetsPath(), pAsset->GetThumbnailPath());

	static QPreviewWidget* pPreviewWidget = new QPreviewWidget();
	pPreviewWidget->SetGrid(false);
	pPreviewWidget->SetAxis(false);
	pPreviewWidget->EnableMaterialPrecaching(true);
	pPreviewWidget->LoadFile(characterFileName);
	pPreviewWidget->setGeometry(0, 0, ASSET_THUMBNAIL_SIZE, ASSET_THUMBNAIL_SIZE);
	pPreviewWidget->FitToScreen();

		pPreviewWidget->SavePreview(thumbnailFileName.c_str());
	}).get();
}


