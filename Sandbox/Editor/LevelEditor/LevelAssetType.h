// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "AssetSystem/AssetType.h"
#include "DrxString/DrxString.h"
#include <future>

class CLevelType : public CAssetType
{
	friend class CLevelEditor;
public:
	//! \sa CLevelType::OnCreate
	struct SCreateParams
	{
		i32   resolution;
		float unitSize;   // size in Meters = resolution * unitSize
		bool  bUseTerrain;

		struct STexture
		{
			i32   resolution;
			float colorMultiplier;
			bool  bHighQualityCompression;
		} texture;
	};

public:
	DECLARE_ASSET_TYPE_DESC(CLevelType)

	virtual tukk   GetTypeName() const          { return "Level"; }
	virtual tukk   GetUiTypeName() const        { return QT_TR_NOOP("Level"); }
	virtual tukk   GetFileExtension() const     { return GetFileExtensionStatic(); }
	virtual bool          IsImported() const           { return false; }
	virtual bool          CanBeCreated() const         { return true; }
	virtual bool          CanBeEdited() const override { return true; }
	virtual CAssetEditor* Edit(CAsset* pAsset) const override;
	virtual bool          DeleteAssetFiles(const CAsset& asset, bool bDeleteSourceFile, size_t& numberOfFilesDeleted) const override;
	virtual bool          RenameAsset(CAsset* pAsset, tukk szNewName) const override                     { return false; }
	virtual bool          MoveAsset(CAsset* pAsset, tukk szNewPath, bool bMoveSourcefile) const override { return false; }

	static tukk    GetFileExtensionStatic()                                                              { return "level"; }

private:
	virtual DrxIcon GetIconInternal() const override;

protected:
	//! \sa CLevelType::SCreateParams
	virtual bool OnCreate(CEditableAsset& editAsset, ukk pTypeSpecificParameter) const override;
	static void  UpdateDependencies(CEditableAsset& editAsset);

protected:
	mutable std::future<bool> m_asyncAction;
};
