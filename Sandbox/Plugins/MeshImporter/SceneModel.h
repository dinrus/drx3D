// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
// Qt item model for FbxTool scene.
#pragma once

#include "SceneModelCommon.h"
#include "QtCommon.h"

#include <vector>
#include <memory>
#include <QAbstractItemModel>
#include <Drx3DEngine/CGF/DrxHeaders.h> // MAX_STATOBJ_LODS_NUM

class DrxIcon;

class QAdvancedTreeView;
class QVariant;

class CSceneElementSkin;

namespace DialogMesh
{

class CSceneUserData;

} //endns DialogMesh

class CSceneModel : public CSceneModelCommon
{
	Q_OBJECT
public:
	enum EColumnType
	{
		eColumnType_Name,
		eColumnType_Type,
		eColumnType_SourceNodeAttribute,
		eColumnType_COUNT
	};
private:
	enum ERoleType
	{
		eRoleType_Export = eItemDataRole_MAX
	};
public:
	CSceneModel();
	~CSceneModel();

	const FbxMetaData::SSceneUserSettings& GetSceneUserSettings() const;

	void                                   SetExportSetting(const QModelIndex& index, FbxTool::ENodeExportSetting value);
	void                                   ResetExportSettingsInSubtree(const QModelIndex& index);

	void                                   OnDataSerialized(CSceneElementCommon* pElement, bool bChanged);

	void                                   SetScene(FbxTool::CScene* pScene, const FbxMetaData::SSceneUserSettings& userSettings);
	void SetSceneUserData(DialogMesh::CSceneUserData* pSceneUserData);

	// QAbstractItemModel
	i32           columnCount(const QModelIndex& index) const override;
	QVariant      data(const QModelIndex& index, i32 role) const override;
	bool          setData(const QModelIndex& index, const QVariant& value, i32 role) override;
	QVariant      headerData(i32 column, Qt::Orientation orientation, i32 role) const override;
	Qt::ItemFlags flags(const QModelIndex& modelIndex) const override;
	// ~QAbstractItemModel
private:
	enum EState
	{
		eState_Enabled_Included,
		eState_Enabled_Excluded,
		eState_Disabled_Included,
		eState_Disabled_Excluded
	};
	void GetCheckStateRole(CSceneElementSourceNode* pSceneElement, EState& state, bool& partial) const;

	QVariant GetSourceNodeData(CSceneElementSourceNode* pElement, const QModelIndex& index, i32 role) const;
	QVariant GetSkinData(CSceneElementSkin* pElement, const QModelIndex& index, i32 role) const;

	static QVariant              GetToolTipForColumn(i32 column);

	bool SetDataSourceNodeElement(const QModelIndex& index, const QVariant& value, i32 role);
	bool SetDataSkinElement(const QModelIndex& index, const QVariant& value, i32 role);

	CItemModelAttribute* GetColumnAttribute(i32 col) const;
private:
	DialogMesh::CSceneUserData* m_pSceneUserData;
	FbxMetaData::SSceneUserSettings m_userSettings;

	DrxIcon* const                  m_pExportIcon;
	i32                             m_iconDimension;
};

