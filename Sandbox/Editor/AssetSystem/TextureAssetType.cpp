// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#include <StdAfx.h>
#include "TextureAssetType.h"

#include "AssetSystem/Asset.h"
#include "AssetSystem/AssetEditor.h"
#include "AssetSystem/AssetManager.h"
#include "Util/Image.h"
#include "Util/ImageUtil.h"
#include "QtUtil.h"
#include "FilePathUtil.h"
#include <QImage>
#include <QDir>

#include <drx3D/CoreX/ToolsHelpers/ResourceCompilerHelper.h>

REGISTER_ASSET_TYPE(CTextureType)

namespace Private_TextureType
{

static bool CallRcWithUserDialog(const string& filename, const string& options = "")
{	
	const CResourceCompilerHelper::ERcExePath path = CResourceCompilerHelper::eRcExePath_editor;
	const auto result = CResourceCompilerHelper::CallResourceCompiler(
	  filename.c_str(),
	  options.c_str(),
	  nullptr,
	  false, // may show window?
	  path,
	  true,  // silent?
	  false); // no user dialog?
	if (result != CResourceCompilerHelper::eRcCallResult_success)
	{
		return false;
	}
	return true;
}

} //endns Private_TextureType

 // Detail attributes.
CItemModelAttribute CTextureType::s_widthAttribute("Width", eAttributeType_Int, CItemModelAttribute::StartHidden);
CItemModelAttribute CTextureType::s_heightAttribute("Height", eAttributeType_Int, CItemModelAttribute::StartHidden);
CItemModelAttribute CTextureType::s_mipCountAttribute("Mip count", eAttributeType_Int, CItemModelAttribute::StartHidden);


DrxIcon CTextureType::GetIconInternal() const
{
	return DrxIcon("icons:common/assets_texture.ico");
}

void CTextureType::GenerateThumbnail(const CAsset* pAsset) const
{
	DRX_ASSERT(0 < pAsset->GetFilesCount());
	if (1 > pAsset->GetFilesCount())
		return;

	tukk textureFileName = pAsset->GetFile(0);
	if (!textureFileName || !*textureFileName)
		return;

	CImageEx image;
	CImageUtil::LoadImage(textureFileName, image);
	if (!image.IsValid())
		return;

	CImageEx preview;
	i32 width = min(image.GetWidth(), ASSET_THUMBNAIL_SIZE);
	i32 height = min(image.GetHeight(), ASSET_THUMBNAIL_SIZE);

	preview.Allocate(width, height);
	preview.Fill(0);
	CImageUtil::ScaleToFit(image, preview);
	preview.SwapRedAndBlue();

	const string thumbnailFileName = PathUtil::Make(PathUtil::GetGameProjectAssetsPath(), pAsset->GetThumbnailPath());

	QDir().mkpath(QtUtil::ToQString(PathUtil::GetPathWithoutFilename(thumbnailFileName)));
	QImage thumbnail = QImage((const uchar*)preview.GetData(), width, height, QImage::Format_ARGB32);
	thumbnail.save(QtUtil::ToQString(thumbnailFileName), "PNG");
}

QWidget* CTextureType::CreateBigInfoWidget(const CAsset* pAsset) const
{
	DRX_ASSERT(0 < pAsset->GetFilesCount());
	if (1 > pAsset->GetFilesCount())
		return nullptr;

	tukk textureFileName = pAsset->GetFile(0);
	if (!textureFileName || !*textureFileName)
		return nullptr;

	CImageEx image;
	CImageUtil::LoadImage(textureFileName, image);
	if (!image.IsValid())
		return nullptr;

	image.SwapRedAndBlue();

	QImage image2 = QImage((const uchar*)image.GetData(), image.GetWidth(), image.GetHeight(), QImage::Format_ARGB32);

	auto label = new QLabel();
	label->setPixmap(QPixmap::fromImage(image2));
	return label;
}

CAssetEditor* CTextureType::Edit(CAsset* pAsset) const
{
	// Editing a texture type presents a special case, as it does not return an asset editor.
	// Instead, we present the modal dialog of the RC.  In the future, this should be changed, and
	// a properly integrated editor should exist in the Sandbox.

	using namespace Private_TextureType;

	if (!pAsset->GetSourceFile() || !*pAsset->GetSourceFile())
	{
		DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Unable to edit %s. No source file.", pAsset->GetName());
		return nullptr;
	}

	DRX_ASSERT(pAsset && pAsset->GetType() && pAsset->GetType()->GetTypeName() == "Texture");

	// A texture can have as a source different file formats than tif, so we check first if 
	// by any chance the source isn't editable asset itself
	CAsset* potentialAsset = CAssetManager::GetInstance()->FindAssetForFile(pAsset->GetSourceFile());
	if (potentialAsset && strcmp(potentialAsset->GetType()->GetTypeName(), "SubstanceInstance") == 0)
	{
		return CAssetEditor::OpenAssetForEdit("Substance Instance Editor", potentialAsset);
	}
	
	if (!GetISystem()->GetIPak()->IsFileExist(pAsset->GetSourceFile()))
	{
		DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_WARNING, "Unable to edit %s. The source file not found: %s.", pAsset->GetName(), pAsset->GetSourceFile());
		return nullptr;
	}

	// A texture asset references a data file of type DDS and a source file of type TIF.
	// We want to call the RC on the TIF, as it stores the user settings.

	const string absTifFilePath = PathUtil::Make(PathUtil::GetGameProjectAssetsPath(), pAsset->GetSourceFile());
	CallRcWithUserDialog(absTifFilePath);

	return nullptr;
}

std::vector<CItemModelAttribute*> CTextureType::GetDetails() const
{
	return
	{
		&s_widthAttribute,
		&s_heightAttribute,
		&s_mipCountAttribute
	};
}

QVariant CTextureType::GetDetailValue(const CAsset * pAsset, const CItemModelAttribute * pDetail) const
{
	static const std::pair<CItemModelAttribute*, tukk > attributes[]
	{
		{ &s_widthAttribute, "width" },
		{ &s_heightAttribute , "height" },
		{ &s_mipCountAttribute, "mipCount" }
	};

	for( const auto& attr : attributes)
	{
		if (attr.first == pDetail)
		{
			return GetVariantFromDetail(pAsset->GetDetailValue(attr.second), pDetail);
		}
	}
	return QVariant();
}

