// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAbstractItemModel>

#include "ProxyModels/ItemModelAttribute.h"

class CObjectLayer;
struct CLayerChangeEvent;

enum ELayerColumns
{
	eLayerColumns_Color, //Must be kept index 0 to match eObjectColumns_LayerColor
	eLayerColumns_Visible,    //Must be kept index 1 to match eObjectColumns_Visible
	eLayerColumns_Frozen,     //Must be kept index 2 to match eObjectColumns_Frozen
	eLayerColumns_Name,       //Must be kept index 3 to match eObjectColumns_Name
	eLayerColumns_Exportable,
	eLayerColumns_ExportablePak,
	eLayerColumns_LoadedByDefault,
	eLayerColumns_HasPhysics,
	eLayerColumns_Platform,
	eLayerColumns_Size	
};

enum ELevelElementType
{
	eLevelElement_Layer,
	eLevelElement_Object,
};

namespace LevelModelsAttributes
{
extern CItemModelAttribute s_ExportableAttribute;
extern CItemModelAttribute s_ExportablePakAttribute;
extern CItemModelAttribute s_LoadedByDefaultAttribute;
extern CItemModelAttribute s_HasPhysicsAttribute;
extern CItemModelAttribute s_PlatformAttribute;
}

namespace LevelModelsUtil
{
void            ProcessIndexList(const QModelIndexList& list, std::vector<CBaseObject*>& outObjects, std::vector<CObjectLayer*>& outLayers, std::vector<CObjectLayer*>& outLayerFolders);
QModelIndexList FilterByColumn(const QModelIndexList& list, i32 column = 0);

QMimeData*      GetDragDropData(const QModelIndexList& list);
bool            ProcessDragDropData(const QMimeData* data, std::vector<CBaseObject*>& outObjects, std::vector<CObjectLayer*>& outLayers);
}

//! This class is not meant to be instantiated directly, request an instance from LevelModelsManager
class CLevelModel : public QAbstractItemModel, public IEditorNotifyListener
{
	static QString LayerSpecToString(i32 spec);
	static i32     LayerSpecToInt(const QString& spec);
private:
	CLevelModel(QObject* parent = nullptr);
	virtual ~CLevelModel();

	friend class CLevelModelsManager;

public:
	CItemModelAttribute* GetColumnAttribute(i32 column) const;

	enum class Roles : i32
	{
		InternalPointerRole = Qt::UserRole, //intptr_t (CObjectLayer*)
		TypeCheckRole                       //Will return ELevelElementType::eLevelElement_Layer
	};

	//////////////////////////////////////////////////////////
	// QAbstractItemModel implementation
	virtual i32             rowCount(const QModelIndex& parent) const override;
	virtual i32             columnCount(const QModelIndex& parent) const override { return eLayerColumns_Size; }
	virtual QVariant        data(const QModelIndex& index, i32 role) const override;
	virtual bool            setData(const QModelIndex& index, const QVariant& value, i32 role) override;
	virtual QVariant        headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	virtual Qt::ItemFlags   flags(const QModelIndex& index) const override;
	virtual QModelIndex     index(i32 row, i32 column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex     parent(const QModelIndex& index) const override;
	virtual Qt::DropActions supportedDragActions() const override;
	virtual Qt::DropActions supportedDropActions() const override;
	virtual QStringList     mimeTypes() const override;
	virtual QMimeData*      mimeData(const QModelIndexList& indexes) const override;
	virtual bool            dropMimeData(const QMimeData* pData, Qt::DropAction action, i32 row, i32 column, const QModelIndex& parent) override;
	virtual bool            canDropMimeData(const QMimeData* pData, Qt::DropAction action, i32 row, i32 column, const QModelIndex& parent) const override;
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// IEditorNotifyListener implementation
	virtual void OnEditorNotifyEvent(EEditorNotifyEvent event) override;
	//////////////////////////////////////////////////////////

	CObjectLayer* LayerFromIndex(const QModelIndex& index) const;
	QModelIndex   IndexFromLayer(const CObjectLayer* pLayer) const;
	i32           RootRowIfInserted(const CObjectLayer* pLayer) const;

private:
	//TODO : this is bad and should not be necessary !
	// was this because we want the level models manager to be notified after the event change? in this case let's just call the right method on the level models manager directly...
	CDrxSignal<void(const CLayerChangeEvent&)> signalBeforeLayerUpdateEvent;

	static tukk s_columnNames[eLayerColumns_Size];

	bool IsNameValid(tukk szName);
	void OnLayerUpdate(const CLayerChangeEvent& event);

	bool m_ignoreLayerUpdates;
};

