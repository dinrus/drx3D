// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//#include "stdafx.h"

#include <Models/CharacterAnimationsModel.h>
#include <Dialogs/TreeViewDialog.h>
#include "IResourceSelectorHost.h"

#include <DrxAnimation/IDrxAnimation.h>

dll_string CharacterAnimationSelector(const SResourceSelectorContext& x, tukk previousValue, IEntity* pEntity)
{
	if (!pEntity || !pEntity->GetCharacter(0) || !pEntity->GetCharacter(0)->GetIAnimationSet())
		return previousValue;

	IAnimationSet* pAnimSet = pEntity->GetCharacter(0)->GetIAnimationSet();
	CCharacterAnimationsModel* pModel = new CCharacterAnimationsModel(*pAnimSet);
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

dll_string ValidateCharacterAnimation(const SResourceSelectorContext& x, tukk newValue, tukk previousValue, IEntity* pEntity)
{
	if (!newValue || !*newValue)
		return dll_string();

	if (!pEntity || !pEntity->GetCharacter(0) || !pEntity->GetCharacter(0)->GetIAnimationSet())
		return previousValue;

	IAnimationSet* pAnimSet = pEntity->GetCharacter(0)->GetIAnimationSet();
	CCharacterAnimationsModel* pModel = new CCharacterAnimationsModel(*pAnimSet);
	i32 itemCount = pModel->rowCount();
	for (i32 i = 0; i < itemCount; ++i)
	{
		QModelIndex index = pModel->index(i, 0);
		if (pModel->data(index, Qt::DisplayRole).value<QString>() == newValue)
			return newValue;
	}

	return previousValue;
}

dll_string ValidateTrackCharacterAnimation(const SResourceSelectorContext& x, tukk newValue, tukk previousValue, IEntity* pEntity)
{
	return newValue;
}

REGISTER_RESOURCE_VALIDATING_SELECTOR("TrackCharacterAnimation", CharacterAnimationSelector, ValidateTrackCharacterAnimation, "")
REGISTER_RESOURCE_VALIDATING_SELECTOR("CharacterAnimation", CharacterAnimationSelector, ValidateCharacterAnimation, "")

