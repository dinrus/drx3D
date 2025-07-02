// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <AssetSystem/AssetType.h>

namespace EditorSubstance
{
	namespace AssetTypes
	{

		class CSubstanceInstanceType : public CAssetType
		{
		public:
			DECLARE_ASSET_TYPE_DESC(CSubstanceInstanceType);

			virtual tukk GetTypeName() const override { return "SubstanceInstance"; }
			virtual tukk GetUiTypeName() const override { return QT_TR_NOOP("Substance Instance"); }
			virtual bool IsImported() const override { return false; }
			virtual bool CanBeEdited() const override { return true; }
			virtual CAssetEditor* Edit(CAsset* asset) const override;

			virtual DrxIcon GetIcon() const override;
			virtual bool HasThumbnail() const override { return false; }
			virtual tukk GetFileExtension() const override { return "crysub"; }
			virtual void AppendContextMenuActions(const std::vector<CAsset*>& assets, CAbstractMenu* menu) const override;
			virtual bool OnCreate(CEditableAsset& editAsset, ukk pTypeSpecificParameter) const override;

		};
	}
}
