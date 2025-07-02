// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QAbstractItemModel>
#include "TextureManager.h"

class CTextureManager;

class CTextureModel : public QAbstractItemModel
{
public:
	enum EColumn
	{
		eColumn_Filename,
		eColumn_RcSettings,
		eColumn_COUNT
	};

	enum ERcSettings
	{
		eRcSettings_Diffuse,
		eRcSettings_Bumpmap,
		eRcSettings_Specular,
		eRcSettings_COUNT
	};
public:
	static tukk GetDisplayNameOfSettings(CTextureModel::ERcSettings settings);

	CTextureModel(CTextureManager* pTextureManager, QObject* pParent = nullptr);

	CTextureManager::TextureHandle GetTexture(const QModelIndex& index);

	CTextureManager*               GetTextureManager();

	// QAbstractItemModel implementation.
	QModelIndex   index(i32 row, i32 column, const QModelIndex& parent) const override;
	QModelIndex   parent(const QModelIndex& index) const override;
	i32           rowCount(const QModelIndex& index) const override;
	i32           columnCount(const QModelIndex& index) const override;
	QVariant      data(const QModelIndex& index, i32 role) const override;
	bool          setData(const QModelIndex& index, const QVariant& value, i32 role) override;
	QVariant      headerData(i32 column, Qt::Orientation orientation, i32 role) const override;
	Qt::ItemFlags flags(const QModelIndex& modelIndex) const override;

	void          Reset();

	void   OnTexturesChanged();
private:
	string GetSettingsRcString(ERcSettings rcSettings) const;
	i32    GetSettings(const string& rcString) const;

	CTextureManager* m_pTextureManager;
};

