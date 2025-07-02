// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QAbstractItemModel>

#include <QString>
#include <QHash>

#include <vector>
#include <memory>

class CVegetationDragDropData;
class CVegetationMap;
class CVegetationObject;
struct CVegetationInstance;

class CVegetationModel : public QAbstractItemModel
{
	Q_OBJECT

	enum class Column
	{
		VisibleAndObject,
		ObjectCount,
		Textsize,
		Material,
		ElevationMin,
		ElevationMax,
		SlopeMin,
		SlopeMax,

		Count
	};

public:
	class CVegetationModelItem;
	typedef std::unique_ptr<CVegetationModelItem> VegetationModelItemPtr;
	class CVegetationModelItem
	{
		enum class EVisibility
		{
			Hidden,
			PartiallyVisible,
			Visible
		};

		friend class CVegetationModel;

	public:
		CVegetationModelItem(const QString& name, i32 r);                                                 // group item
		CVegetationModelItem(i32 r, CVegetationObject* pVegetationObject, CVegetationModelItem* pParent); // vegetation object item

		i32 rowCount() const;

	private:
		CVegetationModelItem* AddChild(CVegetationObject* pVegetationObject);
		bool                  IsGroup() const;

		static EVisibility    GetVisibilityFromCheckState(Qt::CheckState checkState);
		static Qt::CheckState GetCheckStateFromVisibility(EVisibility visibility);

	private:
		QString                             name;
		i32                                 row;
		CVegetationObject*                  pVegetationObject;
		EVisibility                         visibility;

		CVegetationModelItem*               pParent;
		std::vector<VegetationModelItemPtr> children;
	};

public:
	CVegetationModel(QObject* pParent = nullptr);
	~CVegetationModel() override;

	virtual QModelIndex         index(i32 row, i32 column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex         parent(const QModelIndex& child) const override;
	virtual i32                 rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual i32                 columnCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual QVariant            data(const QModelIndex& index, i32 role = Qt::DisplayRole) const override;
	virtual QVariant            headerData(i32 section, Qt::Orientation orientation, i32 role = Qt::DisplayRole) const override;
	virtual Qt::ItemFlags       flags(const QModelIndex& index) const override;
	virtual bool                setData(const QModelIndex& index, const QVariant& value, i32 role = Qt::EditRole) override;
	virtual bool                canDropMimeData(const QMimeData* pData, Qt::DropAction action, i32 row, i32 column, const QModelIndex& parent) const override;
	virtual bool                dropMimeData(const QMimeData* pData, Qt::DropAction action, i32 row, i32 column, const QModelIndex& parent) override;
	virtual QMimeData*          mimeData(const QModelIndexList& indexes) const override;
	virtual QStringList         mimeTypes() const override;
	virtual Qt::DropActions     supportedDragActions() const override;
	virtual Qt::DropActions     supportedDropActions() const override;

	CVegetationObject*          GetVegetationObject(const QModelIndex& index) const;
	QModelIndex					GetGroup(CVegetationObject* pVegetationObject);
	const CVegetationModelItem* GetGroup(const QModelIndex& index) const;
	bool                        IsGroup(const QModelIndex& index) const;
	bool                        IsVegetationObject(const QModelIndex& index) const;

	CVegetationObject*			AddVegetationObject(const QModelIndex& currentIndex, const QString& filename);
	void                        AddGroup();
	void                        RenameGroup(const QModelIndex& selectedGroupIndex, const QString& name);
	void                        RemoveItems(const QModelIndexList& selectedIndexes);
	void                        ReplaceVegetationObject(const QModelIndex& objectIndex, const QString& filename);
	void                        CloneObject(const QModelIndex& objectIndex);
	void                        Select(const QModelIndexList& selectedIndexes);
	QModelIndexList             FindVegetationObjects(const QVector<CVegetationInstance*> selectedInstances);
	void                        UpdateVegetationObject(CVegetationObject* pVegetationObject);
	void                        UpdateAllVegetationObjects();
	void                        MoveInstancesToGroup(const QString& groupName, const QVector<CVegetationInstance*> selectedInstances);
	void                        BeginResetOnLevelChange();
	void                        EndResetOnLevelChange();
	void                        Reset();

signals:
	void InfoDataChanged(i32 objectCount, i32 instanceCount, i32 textureUsage);

private:
	void                                ConnectVegetationMap();
	void                                DisconnectVegetationMap();
	void                                LoadObjects();
	void                                ClearSelection();
	CVegetationModelItem*               FindGroup(const QString& name);
	CVegetationModelItem*               CreateGroup(const QString& name);
	CVegetationModelItem*               FindOrCreateGroup(const QString& name);
	CVegetationModelItem*               AddGroupByName(const QString& name);
	void                                RenameGroup(CVegetationModelItem* pGroupItem, const QString& name);
	void                                RemoveVegetationObjectsFromParent(CVegetationModelItem* pParentGroup, const QSet<CVegetationModelItem*>& removeObjects);
	void                                RemoveGroups(const QSet<CVegetationModelItem*>& removeGroups);
	void                                RemoveVegetationObject(CVegetationModelItem* pVegetationObjectItem);
	void                                RemoveGroup(CVegetationModelItem* pGroup);
	void                                AddVegetationObjectToGroup(CVegetationModelItem* pGroup, CVegetationObject* pVegetationObject);
	QVariant                            GetGroupData(CVegetationModelItem* pGroup, Column column, i32 role) const;
	QVariant                            GetVegetationObjectData(CVegetationModelItem* pVegetationObjectItem, Column column, i32 role) const;
	void                                HandleModelInfoDataChange();
	void                                OnVegetationMapObjectUpdate(bool bNeedsReset);
	std::vector<VegetationModelItemPtr> DeserializeDropGroupData(const CVegetationDragDropData* pDragDropData);
	std::vector<VegetationModelItemPtr> DeserializeDropObjectData(const CVegetationDragDropData* pDragDropData);
	void                                DropVegetationObjects(std::vector<VegetationModelItemPtr>& movedVegetationObjectItems, i32 row, const QModelIndex& parent);
	void                                DropGroups(std::vector<VegetationModelItemPtr>& movedGroupItems, i32 row, const QModelIndex& parent);
	void                                ValidateChildRows(CVegetationModelItem* pGroup, i32 startIdx = 0) const;
	void                                ValidateRows(std::vector<VegetationModelItemPtr>& items, i32 startIdx = 0) const;
	void                                SetVisibility(CVegetationModelItem* pItem, CVegetationModelItem::EVisibility visibility);
	void                                SetVisibilityRecursively(CVegetationModelItem* pItem, CVegetationModelItem::EVisibility visibility);
	void                                UpdateParentVisibilityRecursively(CVegetationModelItem* pItem);

private:
	std::vector<VegetationModelItemPtr>              m_groups;
	QHash<CVegetationObject*, CVegetationModelItem*> m_vegetationObjectToItem;
	CVegetationMap* m_pVegetationMap;
};

