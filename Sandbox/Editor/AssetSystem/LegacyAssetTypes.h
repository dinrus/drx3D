// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "AssetSystem/AssetType.h"

//Support for legacy lua scripts
class CScriptType : public CAssetType
{
public:
	DECLARE_ASSET_TYPE_DESC(CScriptType);

	virtual tukk GetTypeName() const override { return "Script"; }
	virtual tukk GetUiTypeName() const override { return QT_TR_NOOP("Script"); }
	virtual tukk GetFileExtension() const override { return "lua"; }
	virtual bool        IsImported() const override { return false; }
	virtual bool        CanBeEdited() const override { return false; }

private:
	virtual DrxIcon GetIconInternal() const override
	{
		return DrxIcon("icons:General/File.ico");
	}
};

//This encapsulates all xml files. In the future this should provide possibility to convert explicitly to more specialized types
//which could work with database view or other editors of xml based formats
class CXmlType : public CAssetType
{
public:
	DECLARE_ASSET_TYPE_DESC(CXmlType);

	virtual tukk GetTypeName() const override { return "Xml"; }
	virtual tukk GetUiTypeName() const override { return QT_TR_NOOP("Xml"); }
	virtual tukk GetFileExtension() const override { return "xml"; }
	virtual bool IsImported() const override { return false; }
	virtual bool CanBeEdited() const override { return true; }

private:
	virtual DrxIcon GetIconInternal() const override
	{
		return DrxIcon("icons:General/File.ico");
	}

	virtual std::vector<CItemModelAttribute*> GetDetails() const override;
	virtual QVariant GetDetailValue(const CAsset* pAsset, const CItemModelAttribute* pDetail) const override;

	CAssetEditor* Edit(CAsset* pAsset) const override;

public:
	static CItemModelAttributeEnumFunc s_xmlTypeAttribute;
};

// CGA Geometry Animation
// A pivot based animated hard body geometry data. That means we do not use a skinned geometry for this asset type and still got animated rigid parts of an Entity in the Engine.This is the asset type to use for Vehicles since it allows animations like opening the door or any animation which does not use parts bending like skinned geometries for characters.The.cga file is created in the 3D application.The parts will be linked to each other in a hierarchy and will have a main root base.It only supports directly linked objects and does not support skeleton based animation(bone animation) with weighted vertices.Works together with.anm files.
class CAnimatedMeshType : public CAssetType
{
public:
	DECLARE_ASSET_TYPE_DESC(CAnimatedMeshType);

	virtual tukk GetTypeName() const override { return "AnimatedMesh"; }
	virtual tukk GetUiTypeName() const override { return QT_TR_NOOP("Animated Mesh"); }
	virtual tukk GetFileExtension() const override { return "cga"; }
	virtual bool        IsImported() const override { return false; }
	virtual bool        CanBeEdited() const override { return false; }

private:
	virtual DrxIcon GetIconInternal() const override
	{
		return DrxIcon("icons:common/assets_meshanim.ico");
	}
};

// ANM Animation
// Contains animation data for .cga objects. Each CGA file with more then one animation uses .anm files to move the individual parts of the CGA object. If there is only one animation for the object, it will be stored within the CGA file as the default animation in the Character Editor..anm files are used to group different rigid body animations.For example, you can group landing gear coming down, separately from the hatch, to the cockpit opening.
class CMeshAnimationType : public CAssetType
{
public:
	DECLARE_ASSET_TYPE_DESC(CMeshAnimationType);

	virtual tukk GetTypeName() const override { return "MeshAnimation"; }
	virtual tukk GetUiTypeName() const override { return QT_TR_NOOP("Mesh Animation"); }
	virtual tukk GetFileExtension() const override { return "anm"; }
	virtual bool        IsImported() const override { return false; }
	virtual bool        CanBeEdited() const override { return false; }

private:
	virtual DrxIcon GetIconInternal() const override
	{
		return DrxIcon("icons:common/assets_animation.ico");
	}
};

