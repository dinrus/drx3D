// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SystemLibraryModel.h"

namespace ACE
{
class CLibrary;

class CResourceLibraryModel final : public CSystemLibraryModel
{
public:

	explicit CResourceLibraryModel(CLibrary* const pLibrary, QObject* const pParent)
		: CSystemLibraryModel(pLibrary, pParent)
	{}

	CResourceLibraryModel() = delete;

protected:

	// CSystemLibraryModel
	virtual i32             columnCount(QModelIndex const& parent) const override;
	virtual QVariant        data(QModelIndex const& index, i32 role) const override;
	virtual bool            setData(QModelIndex const& index, QVariant const& value, i32 role) override;
	virtual QVariant        headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	virtual Qt::ItemFlags   flags(QModelIndex const& index) const override;
	virtual bool            canDropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent) const override;
	virtual bool            dropMimeData(QMimeData const* pData, Qt::DropAction action, i32 row, i32 column, QModelIndex const& parent) override;
	virtual Qt::DropActions supportedDropActions() const override;
	// ~CSystemLibraryModel
};
} //endns ACE
