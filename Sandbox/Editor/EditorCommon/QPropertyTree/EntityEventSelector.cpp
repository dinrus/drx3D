// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//#include "stdafx.h"

#include <Models/EntityEventsModel.h>
#include <Dialogs/TreeViewDialog.h>
#include "IResourceSelectorHost.h"
#include <DrxEntitySystem/IEntityClass.h>
#include <DrxEntitySystem/IEntity.h>

dll_string EntityEventSelector(const SResourceSelectorContext& x, tukk previousValue, IEntity* pEntity)
{
	if (!pEntity)
		return previousValue;

	IEntityClass* pClass = pEntity->GetClass();
	if (!pClass)
		return previousValue;

	CEntityEventsModel* pModel = new CEntityEventsModel(*pClass);
	CTreeViewDialog dialog(x.parentWidget);
	QString selectedValue(previousValue);

	dialog.Initialize(pModel, 0);

	for (i32 row = pModel->rowCount(); row--; )
	{
		const QModelIndex index = pModel->index(row, 0);
		if (pModel->data(index, Qt::DisplayRole).value<QString>() == selectedValue)
		{
			dialog.SetSelectedValue(index, false);
		}
	}

	if (dialog.exec())
	{
		QModelIndex index = dialog.GetSelected();
		if (index.isValid())
		{
			return index.data().value<QString>().toLocal8Bit().data();
		}
	}

	return previousValue;
}

dll_string ValidateEntityEvent(const SResourceSelectorContext& x, tukk newValue, tukk previousValue, IEntity* pEntity)
{
	if (!newValue || !*newValue)
		return dll_string();

	if (!pEntity)
		return previousValue;

	IEntityClass* pClass = pEntity->GetClass();
	if (!pClass)
		return previousValue;

	CEntityEventsModel* pModel = new CEntityEventsModel(*pClass);
	i32 itemCount = pModel->rowCount();
	for (i32 i = 0; i < itemCount; ++i)
	{
		QModelIndex index = pModel->index(i, 0);
		if (pModel->data(index, Qt::DisplayRole).value<QString>() == newValue)
			return newValue;
	}

	return previousValue;
}

REGISTER_RESOURCE_VALIDATING_SELECTOR("EntityEvent", EntityEventSelector, ValidateEntityEvent, "")

