// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <DrxMovie/IMovieSystem.h>
#include <QAbstractItemModel>

class EDITOR_COMMON_API CSequenceEventsModel : public QAbstractItemModel
{
public:
	CSequenceEventsModel(IAnimSequence& sequence, QObject* pParent = nullptr)
		: m_pSequence(&sequence)
	{}

	virtual ~CSequenceEventsModel()
	{}

	// QAbstractItemModel
	virtual i32         rowCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual i32         columnCount(const QModelIndex& parent = QModelIndex()) const override;
	virtual QVariant    data(const QModelIndex& index, i32 role) const override;
	virtual QVariant    headerData(i32 section, Qt::Orientation orientation, i32 role) const override;
	virtual QModelIndex index(i32 row, i32 column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex parent(const QModelIndex& index) const override;
	// ~QAbstractItemModel

private:
	_smart_ptr<IAnimSequence> m_pSequence;
};

