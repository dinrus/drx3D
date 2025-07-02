// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAbstractItemModel>
#include <QAdvancedTreeView.h>
#include <drx3D/CoreX/Serialization/Forward.h>

class CContentCGF;

class CTargetMeshModel : public QAbstractItemModel
{
	struct SItem;
public:
	// Can be attached to a property tree.
	struct SItemProperties
	{
		SItem* pItem;

		void   Serialize(Serialization::IArchive& ar);
	};
private:
	struct SItem
	{
		SItem*              m_pParent;
		std::vector<SItem*> m_children;
		i32                 m_siblingIndex;
		string              m_name;
		SItemProperties     m_properties;

		SItem()
			: m_pParent(nullptr)
			, m_children()
			, m_siblingIndex(0)
		{
			m_properties.pItem = this;
		}

		void SetName(const string& name)
		{
			m_name = name;
		}

		void AddChild(SItem* pItem)
		{
			DRX_ASSERT(pItem && !pItem->m_pParent);
			pItem->m_pParent = this;
			pItem->m_siblingIndex = m_children.size();
			m_children.push_back(pItem);
		}
	};

	enum EColumnType
	{
		eColumnType_Name,
		eColumnType_COUNT
	};
public:
	explicit CTargetMeshModel(QObject* pParent = nullptr);
	void                   LoadCgf(tukk filename);
	void                   Clear();
	SItemProperties*       GetProperties(const QModelIndex& index);
	const SItemProperties* GetProperties(const QModelIndex& index) const;

	// QAbstractItemModel implementation.

	virtual QModelIndex index(i32 row, i32 column, const QModelIndex& parent) const override;
	virtual QModelIndex parent(const QModelIndex& index) const override;
	virtual i32         rowCount(const QModelIndex& index) const override;
	virtual i32         columnCount(const QModelIndex& index) const override;
	virtual QVariant    data(const QModelIndex& index, i32 role) const override;
	virtual QVariant    headerData(i32 column, Qt::Orientation orientation, i32 role) const override;

private:
	void RebuildInternal(CContentCGF& cgf);

private:
	std::vector<SItem>  m_items;
	std::vector<SItem*> m_rootItems;
};

class CTargetMeshView : public QAdvancedTreeView
{
public:
	explicit CTargetMeshView(QWidget* pParent = nullptr);

	CTargetMeshModel* model();

	virtual void      reset() override;
private:
	std::unique_ptr<CTargetMeshModel> m_pModel;
};

