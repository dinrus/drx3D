// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "SelectAnimationDialog.h"
#include <DrxAnimation/IDrxAnimation.h>

IMPLEMENT_DYNAMIC(CSelectAnimationDialog, CGenericSelectItemDialog)

//////////////////////////////////////////////////////////////////////////
CSelectAnimationDialog::CSelectAnimationDialog(CWnd* pParent)
	: CGenericSelectItemDialog(pParent),
	m_pCharacterInstance(0)
{
	m_dialogID = "Dialogs\\SelAnim";
}

//////////////////////////////////////////////////////////////////////////
/* virtual */ BOOL
CSelectAnimationDialog::OnInitDialog()
{
	SetTitle(_T("Select Animation"));
	SetMode(eMODE_TREE);
	ShowDescription(true);
	return __super::OnInitDialog();
}

//////////////////////////////////////////////////////////////////////////
/* virtual */ void
CSelectAnimationDialog::GetItems(std::vector<SItem>& outItems)
{
	IAnimationSet* pAnimSet = (m_pCharacterInstance ? m_pCharacterInstance->GetIAnimationSet() : 0);

	for (i32 animIndex = 0, animCount = (pAnimSet ? pAnimSet->GetAnimationCount() : 0); animIndex < animCount; ++animIndex)
	{
		tukk animName = (pAnimSet ? pAnimSet->GetNameByAnimID(animIndex) : 0);
		if (animName && *animName)
		{
			string animNameString = animName;
			string::size_type underscorePosition = animNameString.find("_");
			if (underscorePosition == 0)
				underscorePosition = animNameString.find("_", underscorePosition + 1);
			string category = (underscorePosition != string::npos ? animNameString.substr(0, underscorePosition) : "");

			SItem item;
			item.name = (!category.empty() ? category + "/" : "");
			item.name += animNameString;
			outItems.push_back(item);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
/* virtual */ void CSelectAnimationDialog::ItemSelected()
{
}

//////////////////////////////////////////////////////////////////////////
void CSelectAnimationDialog::SetCharacterInstance(ICharacterInstance* pCharacterInstance)
{
	m_pCharacterInstance = pCharacterInstance;
}

//////////////////////////////////////////////////////////////////////////
CString CSelectAnimationDialog::GetSelectedItem()
{
	i32 slashPos = m_selectedItem.Find("/");
	return (slashPos >= 0 ? m_selectedItem.Right(m_selectedItem.GetLength() - slashPos - 1) : m_selectedItem);
}

