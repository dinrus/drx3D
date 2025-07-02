// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "SelectSequenceDialog.h"
#include <DrxMovie/IMovieSystem.h>

// CSelectSequence dialog

IMPLEMENT_DYNAMIC(CSelectSequenceDialog, CGenericSelectItemDialog)

//////////////////////////////////////////////////////////////////////////
CSelectSequenceDialog::CSelectSequenceDialog(CWnd* pParent) : CGenericSelectItemDialog(pParent)
{
	m_dialogID = "Dialogs\\SelSequence";
}

//////////////////////////////////////////////////////////////////////////
/* virtual */ BOOL
CSelectSequenceDialog::OnInitDialog()
{
	SetTitle(_T("Select Sequence"));
	SetMode(eMODE_LIST);
	return __super::OnInitDialog();
}

//////////////////////////////////////////////////////////////////////////
/* virtual */ void
CSelectSequenceDialog::GetItems(std::vector<SItem>& outItems)
{
	IMovieSystem* pMovieSys = GetIEditor()->GetMovieSystem();
	for (i32 i = 0; i < pMovieSys->GetNumSequences(); ++i)
	{
		IAnimSequence* pSeq = pMovieSys->GetSequence(i);
		SItem item;
		string fullname = pSeq->GetName();
		item.name = fullname.c_str();
		outItems.push_back(item);
	}
}

