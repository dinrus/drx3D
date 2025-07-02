// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <QAbstractItemModel>

struct IAnimationSet;

class EDITOR_COMMON_API CCharacterAnimationsModel : public QAbstractItemModel
{
public:
	CCharacterAnimationsModel(IAnimationSet& animationSet, QObject* pParent = nullptr)
		: m_animationSet(animationSet)
	{}

	virtual ~CCharacterAnimationsModel()
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
	IAnimationSet& m_animationSet;
};

