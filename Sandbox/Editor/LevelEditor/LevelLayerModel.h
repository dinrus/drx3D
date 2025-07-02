// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAbstractItemModel>
#include <ProxyModels/ItemModelAttribute.h>

#include "LevelModel.h"

class CObjectLayer;
class CLevelModelsManager;
struct CLayerChangeEvent;

namespace LevelModelsAttributes
{
extern CItemModelAttribute s_layerNameAttribute;
extern CItemModelAttributeEnumT<ObjectType> s_objectTypeAttribute;
extern CItemModelAttribute s_objectTypeDescAttribute;
extern CItemModelAttribute s_visibleAttribute;
extern CItemModelAttribute s_frozenAttribute;
extern CItemModelAttribute s_defaultMaterialAttribute;
extern CItemModelAttribute s_customMaterialAttribute;
extern CItemModelAttribute s_breakableAttribute;
extern CItemModelAttribute s_smartObjectAttribute;
extern CItemModelAttribute s_flowGraphAttribute;
extern CItemModelAttribute s_geometryAttribute;
extern CItemModelAttribute s_geometryInstancesAttribute;
extern CItemModelAttribute s_lodCountAttribute;
extern CItemModelAttribute s_specAttribute;
extern CItemModelAttribute s_aiGroupIdAttribute;
extern CItemModelAttribute s_LayerColorAttribute;
}

//! This class is not meant to be instantiated directly, request an instance from LevelModelsManager
class CLevelLayerModel : public QAbstractItemModel
{
public:

	enum EObjectColumns
	{
		eObjectColumns_LayerColor,
		eObjectColumns_Visible, //Must be kept index 0 to match eLayerColumns_Visible
		eObjectColumns_Frozen,  //Must be kept index 1 to match eLayerColumns_Frozen
		eObjectColumns_Name,    //Must be kept index 2 to match eLayerColumns_Name
		eObjectColumns_Layer,
		eObjectColumns_Type,
		eObjectColumns_TypeDesc,
		eObjectColumns_DefaultMaterial,
		eObjectColumns_CustomMaterial,
		eObjectColumns_Breakability,
		eObjectColumns_SmartObject,
		eObjectColumns_FlowGraph,
		eObjectColumns_Geometry,
		eObjectColumns_InstanceCount,
		eObjectColumns_LODCount,
		eObjectColumns_MinSpec,
		eObjectColumns_AI_GroupId,
		eObjectColumns_Size
	};

	static CItemModelAttribute* GetAttributeForColumn(EObjectColumns column);
	static QVariant             GetHeaderData(i32 section, Qt::Orientation orientation, i32 role);

private:
	CLevelLayerModel(CObjectLayer* pLayer, QObject* parent = nullptr);
	~CLevelLayerModel();
	void Disconnect();

public:

	enum class Roles : i32
	{
		InternalPointerRole = CLevelModel::Roles::InternalPointerRole, // = Qt::UserRole, intptr_t (CBaseObject*)
		TypeCheckRole                                                  //Will return ELevelElementType::eLevelElement_Object
	};

	//////////////////////////////////////////////////////////
	// QAbstractItemModel implementation
	virtual i32 rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual i32 columnCount(const QModelIndex& parent = QModelIndex()) const override
	{
		return eObjectColumns_Size;
	}
	virtual QVariant        data(const QModelIndex& index, i32 role) const override;
	virtual QVariant        headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	virtual Qt::ItemFlags   flags(const QModelIndex& index) const override;
	virtual QModelIndex     index(i32 row, i32 column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex     parent(const QModelIndex& index) const override;
	virtual bool            setData(const QModelIndex& index, const QVariant& value, i32 role) override;
	virtual Qt::DropActions supportedDragActions() const override;
	virtual Qt::DropActions supportedDropActions() const override;
	virtual QStringList     mimeTypes() const override;
	virtual bool            canDropMimeData(const QMimeData* pData, Qt::DropAction action, i32 row, i32 column, const QModelIndex& parent) const override;
	virtual bool            dropMimeData(const QMimeData* pData, Qt::DropAction action, i32 row, i32 column, const QModelIndex& parent) override;
	virtual QMimeData*      mimeData(const QModelIndexList& indexes) const override;
	//////////////////////////////////////////////////////////

	CBaseObject*  ObjectFromIndex(const QModelIndex& index) const;
	QModelIndex   IndexFromObject(const CBaseObject* pObject) const;
	i32           RowFromObject(const CBaseObject* pObject) const;
	CObjectLayer* GetLayer() { return m_pLayer; }

private:
	void              OnLink(CBaseObject* pObject);
	void              OnUnLink(CBaseObject* pObject);

	static bool       Filter(CBaseObject const& obj, uk pLayer);

	CBaseObjectsArray GetLayerObjects() const;

	//! Updates data that was cached as it would be expensive to retrieve every time the view wants to update
	void        Connect();
	void        Rebuild();
	void        Clear();
	void        AddObject(CBaseObject* pObject);
	void        AddObjects(const std::vector<CBaseObject*>& objects);
	void        RemoveObject(CBaseObject* pObject);
	void        RemoveObjects(i32 row, i32 count = 1);
	void        RemoveObjects(const std::vector<CBaseObject*>& objects);
	void        UpdateCachedDataForObject(CBaseObject* pObject);

	void        OnObjectEvent(CObjectEvent&);
	void        OnSelectionChanged();
	void        OnBeforeObjectsAttached(CBaseObject* pParent, const std::vector<CBaseObject*>& objects, bool shouldKeepTransform);
	void        OnObjectsAttached(CBaseObject* pParent, const std::vector<CBaseObject*>& objects);
	void        OnBeforeObjectsDetached(CBaseObject* pParent, const std::vector<CBaseObject*>& objects, bool shouldKeepTransform);
	void        OnObjectsDetached(CBaseObject* pParent, const std::vector<CBaseObject*>& objects);
	void        OnBeforeObjectsDeleted(const CObjectLayer& layer, const std::vector<CBaseObject*>& objects);
	void        OnObjectsDeleted(const CObjectLayer& layer, const std::vector<CBaseObject*>& objects);

	tukk GetMaterialName(CBaseObject* pObject, bool bUseCustomMaterial) const;
	tukk GetObjectBreakability(CBaseObject* pObject) const;
	void        GetMaterialBreakability(std::set<string>* breakTypes, CMaterial* pMaterial) const;
	tukk GetSmartObject(CBaseObject* pObject) const;
	tukk GetFlowGraphNames(CBaseObject* pObject) const;
	tukk GetGeometryFile(CBaseObject* pObject) const;
	uint        GetInstancesCount(CBaseObject* pObject) const;
	uint        GETLODNumber(CBaseObject* pObject) const;
	i32         GetAIGroupID(CBaseObject* pObject) const;
	void        NotifyUpdateObject(CBaseObject* pObject, const QVector<i32>& updateRoles);

	bool        IsRelatedToTopLevelObject(CBaseObject* pObject) const;

	void StartBatchProcess();
	void FinishBatchProcess();

	CObjectLayer* m_pLayer;
	CBaseObject*  m_pTopLevelNotificationObj;

	// Cached data
	CBaseObjectsArray                m_rootObjects;
	std::map<CBaseObjectPtr, string> m_flowGraphMap;
	std::map<string, i32>            m_geometryCountMap;

	bool m_isRuningBatchProcess{ false };

	friend class CLevelModelsManager;
};

