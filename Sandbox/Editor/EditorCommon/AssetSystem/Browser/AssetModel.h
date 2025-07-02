// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QAbstractItemModel>
#include "ProxyModels/ItemModelAttribute.h"
#include "ProxyModels/FavoritesHelper.h"

class CAsset;
class CAssetType;

enum EAssetColumns
{
	eAssetColumns_Favorite,
	eAssetColumns_Name,
	eAssetColumns_Type,
	eAssetColumns_Status,
	eAssetColumns_Folder,
	eAssetColumns_Uid,
	eAssetColumns_Thumbnail,
	eAssetColumns_FilterString, //optimization for smart search
	eAssetColumns_Details // First column of asset type-specific details. NOTE: Must be the last enumerator.
};

enum EAssetModelRowType
{
	eAssetModelRow_Asset,
	eAssetModelRow_Folder
};

namespace AssetModelAttributes
{
	extern CItemModelAttributeEnumFunc s_AssetTypeAttribute;
	extern CItemModelAttributeEnumFunc s_AssetStatusAttribute;
	extern CItemModelAttribute s_AssetFolderAttribute;
	extern CItemModelAttribute s_AssetUidAttribute;
	extern CItemModelAttribute s_FilterStringAttribute;
};

//! This model describes all the assets contained in the AssetManager
//! It uses attributes to enable smart filtering of assets in the asset browser
class EDITOR_COMMON_API CAssetModel : public QAbstractItemModel
{
	//CAssetModel instance is managed by the AssetManager
	friend class CAssetManager;
	CAssetModel(QObject* parent = nullptr);
	~CAssetModel();

	struct SDetailAttribute
	{
		const CItemModelAttribute* pAttribute;
		std::vector<CAssetType*> assetTypes; //!< All asset types sharing attribute \p pAttribute.
		std::function<QVariant(const CAsset* pAsset, const CItemModelAttribute* pDetail)> computeFn;
	};

public:
	static CAssetModel* GetInstance();

	enum class Roles : i32
	{
		InternalPointerRole = Qt::UserRole, //intptr_t (CAsset*)
		TypeCheckRole,						//EAssetModelRowType
		Max,
	};

	CAsset* ToAsset(const QModelIndex& index);
	const CAsset* ToAsset(const QModelIndex& index) const;

	QModelIndex ToIndex(const CAsset& asset, i32 col = 0) const;

	static const CItemModelAttribute* GetColumnAttribute(i32 column);
	static QVariant				GetHeaderData(i32 section, Qt::Orientation orientation, i32 role);
	static i32					GetColumnCount();
	bool AddComputedColumn(const CItemModelAttribute* pAttribute, std::function<QVariant(const CAsset* pAsset, const CItemModelAttribute* pAttribute)> computeFn);

	//////////////////////////////////////////////////////////
	// QAbstractItemModel implementation
	virtual bool			hasChildren(const QModelIndex& parent = QModelIndex()) const override;
	virtual i32             rowCount(const QModelIndex& parent) const override;
	virtual i32             columnCount(const QModelIndex& parent) const override;
	virtual QVariant        data(const QModelIndex& index, i32 role) const override;
	virtual bool            setData(const QModelIndex& index, const QVariant& value, i32 role) override;
	virtual QVariant        headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	virtual Qt::ItemFlags   flags(const QModelIndex& index) const override;
	virtual QModelIndex     index(i32 row, i32 column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex     parent(const QModelIndex& index) const override;
	virtual Qt::DropActions supportedDragActions() const override;
	/*virtual Qt::DropActions supportedDropActions() const override;*/
	virtual QStringList     mimeTypes() const override;
	virtual QMimeData*      mimeData(const QModelIndexList& indexes) const override;
	/*virtual bool            dropMimeData(const QMimeData* pData, Qt::DropAction action, i32 row, i32 column, const QModelIndex& parent) override;
	virtual bool            canDropMimeData(const QMimeData* pData, Qt::DropAction action, i32 row, i32 column, const QModelIndex& parent) const override;*/
	//////////////////////////////////////////////////////////

	static QStringList GetAssetTypesStrList();
	static QStringList GetStatusesList();

private:
	//! \see CAssetManager::Init() 
	void Init();

	//! Builds the vector of detail attributes. A detail attribute stores a CItemModelAttribute A and a name map M.
	//! For each asset type that returns a detail with attribute A, M contains a pair <N, T>, where T is the asset type and N is the detail name for A in T.
	//! Example: Assume MeshType returns a detail with name "vertexCount", and CItemModelAttribute vertexCountAttribute. Also, SkinType returns a detail with
	//! name "numVertices" but same attribute vertexCountAttribute.
	//! Then m_detailAttributes contains an element with A = vertexCountAttribute and M = { { MeshType, "vertexCount" }, { SkinType, "numVertices" } };
	void BuildDetailAttributes();

	void AddPredefinedComputedColumns();

	void PreUpdate();
	void PostUpdate();

	void PreInsert(const std::vector<CAsset*>& assets);
	void PostInsert(const std::vector<CAsset*>& assets);

	void PreRemove(const std::vector<CAsset*>& assets);
	void PostRemove();

	void OnAssetChanged(CAsset& asset);
	void OnAssetThumbnailLoaded(CAsset& asset);
	
	FavoritesHelper m_favHelper;

	std::vector<SDetailAttribute> m_detailAttributes;
};
