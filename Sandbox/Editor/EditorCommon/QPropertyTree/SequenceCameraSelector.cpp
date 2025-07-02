// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//#include "stdafx.h"

#include <Models/SequenceCamerasModel.h>
#include <Dialogs/TreeViewDialog.h>
#include "IResourceSelectorHost.h"

#include <DrxAnimation/IDrxAnimation.h>

dll_string SequenceCameraSelector(const SResourceSelectorContext& x, tukk previousValue, IAnimSequence* pSequence)
{
	if (!pSequence)
		return previousValue;

	CSequenceCamerasModel* pModel = new CSequenceCamerasModel(*pSequence);
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

dll_string ValidateSequenceCamera(const SResourceSelectorContext& x, tukk newValue, tukk previousValue, IAnimSequence* pSequence)
{
	if (!newValue || !*newValue)
		return dll_string();

	if (!pSequence)
		return previousValue;

	CSequenceCamerasModel* pModel = new CSequenceCamerasModel(*pSequence);
	i32 itemCount = pModel->rowCount();
	for (i32 i = 0; i < itemCount; ++i)
	{
		QModelIndex index = pModel->index(i, 0);
		if (pModel->data(index, Qt::DisplayRole).value<QString>() == newValue)
			return newValue;
	}

	return previousValue;
}

REGISTER_RESOURCE_VALIDATING_SELECTOR("SequenceCamera", SequenceCameraSelector, ValidateSequenceCamera, "")

