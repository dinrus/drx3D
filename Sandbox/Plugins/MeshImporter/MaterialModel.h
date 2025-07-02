// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "QtCommon.h"

#include <drx3D/CoreX/Math/LCGRandom.h>

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include <vector>
#include <memory>

class CMaterialElement;

class CMaterialModel : public QAbstractListModel
{
public:
	enum EColumnType
	{
		eColumnType_Name,
		eColumnType_SubMaterial,
		eColumnType_Physicalize,
		eColumnType_Color,
		eColumnType_Count,
	};

	enum EItemDataRole
	{
		eItemDataRole_Color = ::eItemDataRole_MAX,
		eItemDataRole_Comparison
	};

	CMaterialModel();
	~CMaterialModel();

	CMaterialElement* AtIndex(const QModelIndex& index) const
	{
		const size_t idx = (size_t)index.row();
		return idx < m_materials.size() ? m_materials[idx].get() : nullptr;
	}

	QModelIndex            AtIndex(CMaterialElement* pElement) const;

	const FbxTool::CScene* GetScene() const;
	FbxTool::CScene*       GetScene();

	void                   SetScene(FbxTool::CScene* pScene);
	void                   ClearScene();
	void                   Reset();

	void                   OnDataSerialized(CMaterialElement* pElement, bool bChanged);

	i32                    GetSortColumn()
	{
		return eColumnType_Name;
	}

	CMaterialElement* FindElement(const QString& name);

	typedef std::vector<std::pair<i32, QString>> TKnownMaterials;

	void                   ApplyMaterials(TKnownMaterials&& knownMaterials, bool bRefreshCollection);

	const TKnownMaterials& GetKnownMaterials() const
	{
		return m_knownMaterials;
	}

	// Finds and returns name of sub-material in .mtl file, if present. Returns empty string, otherwise.
	QString GetSubMaterialName(i32 index) const;

	// QAbstractListModel
	i32           rowCount(const QModelIndex& index) const override;
	i32           columnCount(const QModelIndex& index) const override;
	QVariant      data(const QModelIndex& index, i32 role) const override;
	QVariant      headerData(i32 column, Qt::Orientation orientation, i32 role) const override;
	bool          setData(const QModelIndex& modelIndex, const QVariant& variant, i32 role) override;
	Qt::ItemFlags flags(const QModelIndex& modelIndex) const;
	// ~QAbstractListModel

private:
	static QVariant GetToolTipForColumn(i32 column);

	void            MakeConsistent(CMaterialElement& provokingElement);

	FbxTool::CScene* m_pScene;
	std::vector<std::unique_ptr<CMaterialElement>> m_materials;
	TKnownMaterials  m_knownMaterials;

	CRndGen          m_random;
};

class CSortedMaterialModel : public QSortFilterProxyModel
{
public:
	CSortedMaterialModel(QObject* pParent = nullptr)
		: QSortFilterProxyModel(pParent)
		, m_pModel(new CMaterialModel())
	{
		setSortLocaleAware(false);
		setSortCaseSensitivity(Qt::CaseInsensitive);
		setSourceModel(m_pModel);
		setSortRole(CMaterialModel::eItemDataRole_Comparison);

		connect(m_pModel, &QAbstractItemModel::dataChanged, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<i32>& roles)
		{
			sort(m_pModel->GetSortColumn());
		});
	}

	~CSortedMaterialModel()
	{
		delete m_pModel;
	}

	CMaterialElement* AtIndex(const QModelIndex& index) const
	{
		const QModelIndex sourceIndex = mapToSource(index);
		return m_pModel->AtIndex(sourceIndex);
	}

	QModelIndex AtIndex(CMaterialElement* pElement) const
	{
		const QModelIndex sourceIndex = m_pModel->AtIndex(pElement);
		return mapFromSource(sourceIndex);
	}

	void                   SetScene(FbxTool::CScene* pScene);
	void                   ClearScene();
	void                   Reset();

	const FbxTool::CScene* GetScene() const;

	QString                GetSubMaterialName(i32 index) const;

	CMaterialElement*      FindElement(const QString& name)
	{
		return m_pModel->FindElement(name);
	}

	void ApplyMaterials(const string& materialName);

	void ApplyMaterials(CMaterialModel::TKnownMaterials&& knownMaterials, bool bRefreshCollection)
	{
		m_pModel->ApplyMaterials(std::move(knownMaterials), bRefreshCollection);
	}
protected:
	virtual bool lessThan(const QModelIndex& lhp, const QModelIndex& rhp) const override;
private:
	CMaterialModel* const m_pModel;
};

