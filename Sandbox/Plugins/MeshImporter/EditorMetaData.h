// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "FbxScene.h"
#include "FbxMetaData.h"
#include "GlobalImportSettings.h"
#include "NodeProperties.h"

#include <drx3D/CoreX/String/DrxString.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/yasli/Archive.h>
#include <drx3D/CoreX/Serialization/yasli/STL.h>
#include <drx3D/CoreX/Serialization/yasli/Enum.h>
#include <drx3D/CoreX/Serialization/yasli/JSONIArchive.h>
#include <drx3D/CoreX/Serialization/yasli/JSONOArchive.h>

namespace FbxTool
{
namespace Meta
{

struct SNodeMeta
{
	std::vector<string>         path;
	FbxTool::ENodeExportSetting exportSetting;
	i32                         lod;
	bool                        bIsProxy;
	std::vector<string>         customProperties;
	CNodeProperties             physicalProperties;

	void                        Serialize(Serialization::IArchive& ar);
};

void WriteNodeMetaData(const CScene& scene, std::vector<SNodeMeta>& nodeMeta);
void ReadNodeMetaData(const std::vector<SNodeMeta>& nodeMeta, CScene& scene);

struct SMaterialMeta
{
	string                      name;
	EMaterialPhysicalizeSetting physicalizeSetting;
	i32                         subMaterialIndex;
	bool                        bMarkedForIndexAutoAssignment;
	ColorF                      color;

	void                        Serialize(Serialization::IArchive& ar);
};

void WriteMaterialMetaData(const CScene& scene, std::vector<SMaterialMeta>& materialMeta);
void ReadMaterialMetaData(const std::vector<SMaterialMeta>& materialMeta, CScene& scene);

} //endns Meta
} //endns FbxTool

//! Data used to restore state of editor. Not passed to RC.
struct SEditorMetaData : FbxMetaData::IEditorMetaData
{
	CGlobalImportSettings editorGlobalImportSettings;
	std::vector<FbxTool::Meta::SNodeMeta>     editorNodeMeta;
	std::vector<FbxTool::Meta::SMaterialMeta> editorMaterialMeta;

	SEditorMetaData();

	// IEditorMetaData implementation.
	virtual void Serialize(yasli::Archive& ar) override;
	virtual std::unique_ptr<FbxMetaData::IEditorMetaData> Clone() const override;
};


