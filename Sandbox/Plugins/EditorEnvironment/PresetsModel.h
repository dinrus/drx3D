// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QtCore/QAbstractItemModel>

#include <drx3D/CoreX/DrxFlags.h>
#include <drx3D/CoreX/smartptr.h>

struct CPresetsModelNode : public _i_reference_target_t
{
	typedef std::vector<CPresetsModelNode*> ChildNodes;

	enum EType
	{
		EType_Group,
		EType_Leaf
	};

	enum EFlags
	{
		EFlags_Modified = BIT(0),
		EFlags_InPak    = BIT(1),
		EFlags_InFolder = BIT(2),
		//EFlags_Default     = BIT(3),
	};
	typedef CDrxFlags<u8> NodeFlags;

	CPresetsModelNode(EType _type, tukk _szName, tukk _szPath, CPresetsModelNode* parent)
		: type(_type)
		, name(_szName)
		, path(_szPath)
		, pParent(parent)
	{

	}

	~CPresetsModelNode()
	{
		for (size_t i = 0, childCount = children.size(); i < childCount; ++i)
			delete children[i];
	}

	EType              type;
	NodeFlags          flags;
	string             name;
	string             path;

	CPresetsModelNode* pParent;
	ChildNodes         children;
};
typedef _smart_ptr<CPresetsModelNode> CPresetsModelNodePtr;

class QPresetsWidget;

class CPresetsModel : public QAbstractItemModel
{
	Q_OBJECT

	enum EColumn
	{
		eColumn_Name,
		eColumn_PakStatus,

		eColumn_COUNT
	};

public:
	CPresetsModel(QPresetsWidget& presetsWidget, QObject* pParent);

	QModelIndex               index(i32 row, i32 column, const QModelIndex& parent) const override;
	Qt::ItemFlags             flags(const QModelIndex& index) const;

	i32                       rowCount(const QModelIndex& parent) const override;
	i32                       columnCount(const QModelIndex& parent) const override;

	QVariant                  headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	bool                      hasChildren(const QModelIndex& parent) const override;
	QVariant                  data(const QModelIndex& index, i32 role) const override;
	bool                      setData(const QModelIndex& index, const QVariant& value, i32 role /* = Qt::EditRole */) override;
	QModelIndex               parent(const QModelIndex& index) const override;
	tukk               GetHeaderSectionName(i32 section) const;

	QModelIndex               ModelIndexFromNode(CPresetsModelNode* pNode) const;
	static CPresetsModelNode* GetEntryNode(const QModelIndex& index);

public slots:
	void OnSignalBeginAddEntryNode(CPresetsModelNode* pEntryNode);
	void OnSignalEndAddEntryNode(CPresetsModelNode* pEntryNode);
	void OnSignalBeginDeleteEntryNode(CPresetsModelNode* pEntryNode);
	void OnSignalEndDeleteEntryNode();
	void OnSignalBeginResetModel();
	void OnSignalEndResetModel();
	void OnSignalEntryNodeDataChanged(CPresetsModelNode* pEntryNode);
protected:
	QPresetsWidget& m_presetsWidget;
};

