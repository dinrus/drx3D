// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/String/DrxString.h>

#include <QAbstractItemModel>

#include <memory>

class CAsset;
class CAssetType;
class CItemModelAttribute;

//! Contains at most one item which represents a new asset. This item is editable, so that its name
//! can be set by a user. This item is only a placeholder and will be removed once a name is chosen.
//! This class is closely related to the "New asset" function of the asset browser.
class CNewAssetModel final : public QAbstractItemModel
{
private:
	struct SAssetDescription
	{
		string name;
		string path;
		const CAssetType* pAssetType;
		ukk pTypeSpecificParameter;
		bool bChanged = false;
	};

public:
	void BeginCreateAsset(const string& path, const string& name, const CAssetType& type, ukk pTypeSpecificParameter);
	void EndCreateAsset();
	bool IsEditing() { return m_pRow.get() != nullptr; }
	CAsset* GetNewAsset() const;

	static CNewAssetModel* GetInstance()
	{
		static CNewAssetModel theInstance;
		return &theInstance;
	}

	//////////////////////////////////////////////////////////
	// QAbstractItemModel implementation.
	virtual i32 rowCount(const QModelIndex& parent) const override;
	virtual i32 columnCount(const QModelIndex& parent) const override;
	virtual QVariant data(const QModelIndex& index, i32 role) const override;
	virtual bool setData(const QModelIndex& index, const QVariant& value, i32 role = Qt::EditRole) override;
	virtual QModelIndex index(i32 row, i32 column, const QModelIndex& parent) const override;
	virtual QModelIndex parent(const QModelIndex& index) const override;
	virtual QVariant headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
	//////////////////////////////////////////////////////////
private:
	CNewAssetModel();
private:
	std::unique_ptr<SAssetDescription> m_pRow;
	CAsset* m_pAsset;
	string m_defaultName;
};
