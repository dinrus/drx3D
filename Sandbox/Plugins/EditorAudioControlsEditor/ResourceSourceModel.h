// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SystemSourceModel.h"

namespace ACE
{
class CResourceSourceModel final : public CSystemSourceModel
{
public:

	explicit CResourceSourceModel(QObject* const pParent)
		: CSystemSourceModel(pParent)
	{}

	CResourceSourceModel() = delete;

	static QVariant GetHeaderData(i32 const section, Qt::Orientation const orientation, i32 const role);

protected:

	// CSystemSourceModel
	virtual i32             columnCount(QModelIndex const& parent) const override;
	virtual QVariant        data(QModelIndex const& index, i32 role) const override;
	virtual bool            setData(QModelIndex const& index, QVariant const& value, i32 role) override;
	virtual QVariant        headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	virtual Qt::ItemFlags   flags(QModelIndex const& index) const override;
	virtual bool            canDropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent) const override;
	virtual bool            dropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent) override;
	virtual Qt::DropActions supportedDropActions() const override;
	// ~CSystemSourceModel
};
} //endns ACE
