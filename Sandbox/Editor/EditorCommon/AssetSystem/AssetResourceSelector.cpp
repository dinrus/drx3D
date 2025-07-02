// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"
#include "AssetResourceSelector.h"

#include "AssetSystem/Asset.h"
#include "AssetSystem/AssetManager.h"
#include "AssetSystem/AssetType.h"
#include "AssetSystem/Browser/AssetBrowserDialog.h"
#include "AssetSystem/Browser/AssetTooltip.h"
#include "PreviewToolTip.h"
#include "FileDialogs/EngineFileDialog.h"

namespace Private_AssetSelector
{
	CAsset* FindAssetForFileAndContext(const SResourceSelectorContext& selectorContext, tukk value)
	{
		const DrxPathString assetFile(PathUtil::ToUnixPath<DrxPathString>(value));
		CAssetManager* const pManager = CAssetManager::GetInstance();
		CAsset* pAsset = pManager->FindAssetForFile(assetFile.c_str());

		if (!pAsset)
		{
			DRX_ASSERT(selectorContext.resourceSelectorEntry && selectorContext.resourceSelectorEntry->IsAssetSelector());
			const SStaticAssetSelectorEntry* selector = static_cast<const SStaticAssetSelectorEntry*>(selectorContext.resourceSelectorEntry);
			const auto& assetTypes = selector->GetAssetTypes();

			if (assetTypes.size() != 1)
				return nullptr;

			const CAssetType* const pType = assetTypes.front();
			if (!pType)
				return nullptr;

			// value may points to source file (e.g. tif), try to find asset based on the asset type.
			pAsset = pManager->FindAssetForFile(PathUtil::ReplaceExtension(assetFile.c_str(), pType->GetFileExtension()));
		}

		return pAsset;
	}

	dll_string SelectAssetLegacy(const SResourceSelectorContext& selectorContext, tukk previousValue)
	{
		DRX_ASSERT(selectorContext.resourceSelectorEntry->IsAssetSelector());
		const SStaticAssetSelectorEntry* selector = static_cast<const SStaticAssetSelectorEntry*>(selectorContext.resourceSelectorEntry);

		CEngineFileDialog::RunParams runParams;
		runParams.initialFile = previousValue;

		for (auto type : selector->GetAssetTypes())
		{
			QString description = QString("%1 (*.%2)").arg(type->GetTypeName()).arg(type->GetFileExtension());
			runParams.extensionFilters << CExtensionFilter(description, type->GetFileExtension());
		}

		runParams.extensionFilters << CExtensionFilter("All Files (*.*)", "*");

		QString filename = CEngineFileDialog::RunGameOpen(runParams, nullptr);

		if (!filename.isEmpty())
			return filename.toStdString().c_str();
		else
			return previousValue;
	}

	dll_string SelectAsset(const SResourceSelectorContext& selectorContext, tukk previousValue)
	{
		const auto pickerState = (EAssetResourcePickerState)GetIEditor()->GetSystem()->GetIConsole()->GetCVar("ed_enableAssetPickers")->GetIVal();
		if (pickerState == EAssetResourcePickerState::Disable)
		{
			return SelectAssetLegacy(selectorContext, previousValue);
		}

		DRX_ASSERT(selectorContext.resourceSelectorEntry && selectorContext.resourceSelectorEntry->IsAssetSelector());
		const SStaticAssetSelectorEntry* selector = static_cast<const SStaticAssetSelectorEntry*>(selectorContext.resourceSelectorEntry);
		const auto& assetTypes = selector->GetAssetTypes();

		CAssetBrowserDialog dialog(selector->GetAssetTypeNames(), CAssetBrowserDialog::Mode::OpenSingleAsset, selectorContext.parentWidget);

		if (previousValue)
		{
			const CAsset* pAsset = FindAssetForFileAndContext(selectorContext, previousValue);

			if (pAsset)
			{
				dialog.SelectAsset(*pAsset);
			}
		}

		QObject::connect(&dialog, &CAssetBrowserDialog::SelectionChanged, [selectorContext](const QVector<CAsset*>& assets)
		{
			DRX_ASSERT(assets.size() <= 1);
			if (!assets.empty() && selectorContext.callback)
			{
				selectorContext.callback->SetValue(assets.front()->GetFile(0));
			}
		});

		if (dialog.Execute())
		{
			if (CAsset* pSelectedAsset = dialog.GetSelectedAsset())
			{
				return pSelectedAsset->GetFile(0);
			}
		}

		return previousValue;
	}

	void EditAsset(const SResourceSelectorContext& selectorContext, tukk value)
	{
		if (value && *value)
		{
			CAsset* pAsset = FindAssetForFileAndContext(selectorContext, value);

			if (pAsset)
				return pAsset->Edit();
		}
	}

	dll_string ValidateAsset(const SResourceSelectorContext& selectorContext, tukk newValue, tukk previousValue)
	{
		if (!newValue || !*newValue)
			return dll_string();

		DRX_ASSERT(selectorContext.resourceSelectorEntry->IsAssetSelector());
		const SStaticAssetSelectorEntry* selector = static_cast<const SStaticAssetSelectorEntry*>(selectorContext.resourceSelectorEntry);

		QFileInfo fileInfo(newValue);
		QString assetPath(newValue);

		if (fileInfo.suffix().isEmpty())
		{
			//Try to autocomplete it
			if (selector->GetAssetTypes().size() == 1)
			{
				assetPath += ".";
				assetPath += selector->GetAssetTypes()[0]->GetFileExtension();
			}
			else
			{
				//cannot autocomplete, invalid
				return previousValue;
			}
		}

		//If suffix is not empty, does it match allowed types?
		bool bMatch = false;
		const QString suffix = fileInfo.suffix();
		for (auto type : selector->GetAssetTypes())
		{
			if (suffix == type->GetFileExtension())
			{
				bMatch = true;
				break;
			}
		}

		if (bMatch)
		{
			const auto pickerState = (EAssetResourcePickerState)GetIEditor()->GetSystem()->GetIConsole()->GetCVar("ed_enableAssetPickers")->GetIVal();
			if (pickerState == EAssetResourcePickerState::Disable)
			{
				return assetPath.toStdString().c_str();
			}
			else
			{
				//Finally check if there is a valid asset for this path
				CAsset* asset = CAssetManager::GetInstance()->FindAssetForFile(assetPath.toStdString().c_str());
				if (asset)
					return assetPath.toStdString().c_str();
			}
		}

		return previousValue;
	}
}

SStaticAssetSelectorEntry::SStaticAssetSelectorEntry(tukk typeName)
	: SStaticResourceSelectorEntry(typeName, Private_AssetSelector::SelectAsset, Private_AssetSelector::ValidateAsset, Private_AssetSelector::EditAsset, "", false)
	, assetTypeNames({ typeName })
{
	//Note: we cannot populate the icon easily as CAssetType returns an icon and not an icon path
	isAsset = true;
}

SStaticAssetSelectorEntry::SStaticAssetSelectorEntry(tukk typeName, const std::vector<tukk >& typeNames)
	: SStaticResourceSelectorEntry(typeName, Private_AssetSelector::SelectAsset, Private_AssetSelector::ValidateAsset, Private_AssetSelector::EditAsset, "", false)
{
	for (auto& typeName : typeNames)
	{
		assetTypeNames.push_back(typeName);
	}

	isAsset = true;
}

SStaticAssetSelectorEntry::SStaticAssetSelectorEntry(tukk typeName, const std::vector<string>& typeNames)
	: SStaticResourceSelectorEntry(typeName, Private_AssetSelector::SelectAsset, Private_AssetSelector::ValidateAsset, Private_AssetSelector::EditAsset, "", false)
{
	assetTypeNames = typeNames;
	isAsset = true;
}

bool SStaticAssetSelectorEntry::ShowTooltip(const SResourceSelectorContext& context, tukk value) const
{
	if (value && *value)
	{
		CAsset* asset = CAssetManager::GetInstance()->FindAssetForFile(value);
		if (asset)
		{
			CAssetTooltip::ShowTrackingTooltip(asset, context.parentWidget);
			return true;
		}
	}

	return SStaticResourceSelectorEntry::ShowTooltip(context, value);
}

void SStaticAssetSelectorEntry::HideTooltip(const SResourceSelectorContext& context, tukk value) const
{
	//Will also hide asset tooltip
	QTrackingTooltip::HideTooltip();
}

const std::vector<const CAssetType*>& SStaticAssetSelectorEntry::GetAssetTypes() const
{
	if (!assetTypes.size())
	{
		const_cast<SStaticAssetSelectorEntry*>(this)->assetTypes.reserve(assetTypes.size());
		for (tukk typeName : assetTypeNames)
		{
			auto assetType = GetIEditor()->GetAssetManager()->FindAssetType(typeName);
			DRX_ASSERT(assetType);
			const_cast<SStaticAssetSelectorEntry*>(this)->assetTypes.push_back(assetType);
		}
	}

	return assetTypes;
}

//////////////////////////////////////////////////////////////////////////

dll_string SStaticAssetSelectorEntry::SelectFromAsset(const SResourceSelectorContext& context, const std::vector<string>& types, tukk previousValue)
{
	CAssetBrowserDialog dialog(types, CAssetBrowserDialog::Mode::OpenSingleAsset);

	if (previousValue)
	{
		CAssetManager* const pManager = CAssetManager::GetInstance();

		const CAsset* pAsset = pManager->FindAssetForFile(previousValue);

		// previousValue may points to source file (e.g. tif), try to find asset based on the asset type.
		if (!pAsset && types.size() == 1)
		{
			CAssetType* const pType = pManager->FindAssetType(types.front());
			if (pType)
			{
				pAsset = pManager->FindAssetForFile(PathUtil::ReplaceExtension(previousValue, pType->GetFileExtension()));
			}
		}

		if (pAsset)
		{
			dialog.SelectAsset(*pAsset);
		}
	}

	QObject::connect(&dialog, &CAssetBrowserDialog::SelectionChanged, [&context](const QVector<CAsset*>& assets)
	{
		DRX_ASSERT(assets.size() <= 1);
		if (!assets.empty() && context.callback)
		{
			context.callback->SetValue(assets.front()->GetFile(0));
		}
	});

	if (dialog.Execute())
	{
		if (CAsset* pSelectedAsset = dialog.GetSelectedAsset())
		{
			return pSelectedAsset->GetFile(0);
		}
	}

	return previousValue;
}
