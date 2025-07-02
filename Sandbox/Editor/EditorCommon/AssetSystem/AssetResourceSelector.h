// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include "IResourceSelectorHost.h"

class CAssetType;

//! Asset based resource picker
//! A picker is registered automatically for every asset type unless otherwise specified, i.e. CAssetType::IsUsingGenericPropertyTreePicker returns false.
struct EDITOR_COMMON_API SStaticAssetSelectorEntry : public SStaticResourceSelectorEntry
{
	SStaticAssetSelectorEntry(tukk typeName);
	SStaticAssetSelectorEntry(tukk typeName, const std::vector<tukk >& typeNames);
	SStaticAssetSelectorEntry(tukk typeName, const std::vector<string>& typeNames);

	const std::vector<const CAssetType*>& GetAssetTypes() const;
	const std::vector<string>& GetAssetTypeNames() const { return assetTypeNames; }

	virtual bool			ShowTooltip(const SResourceSelectorContext& context, tukk value) const override;
	virtual void			HideTooltip(const SResourceSelectorContext& context, tukk value) const override;

	//This helper method is here for special selectors that cannot use SStaticAssetSelectorEntry directly
	static dll_string SelectFromAsset(const SResourceSelectorContext& context, const std::vector<string>& types, tukk previousValue);

private:
	std::vector<string> assetTypeNames;
	std::vector<const CAssetType*> assetTypes;
};

//Register a picker type that accept several asset types
//Usage REGISTER_RESOURCE_SELECTOR_ASSET_MULTIPLETYPES("Picker", std::vector<tukk >({ "type1", "type2", ...}))
#define REGISTER_RESOURCE_SELECTOR_ASSET_MULTIPLETYPES(name, asset_types) \
 namespace Private_ResourceSelector { SStaticAssetSelectorEntry INTERNAL_RSH_COMBINE(selector_ ## function, __LINE__)((name), (asset_types)); \
INTERNAL_REGISTER_RESOURCE_SELECTOR(INTERNAL_RSH_COMBINE(selector_ ## function, __LINE__)) }
