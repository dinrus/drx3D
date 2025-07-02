// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "ColorCheckBox.h"

void CCustomButton::SetIcon(LPCSTR lpszIconResource, i32 nIconAlign, bool bDrawText)
{
	SetWindowText("");
	HICON hIcon = (HICON)::LoadImage(AfxGetInstanceHandle(), lpszIconResource, IMAGE_ICON, 0, 0, 0);
	__super::SetIcon(CSize(32, 32), hIcon, NULL, FALSE);
}

void CCustomButton::SetIcon(HICON hIcon, i32 nIconAlign, bool bDrawText)
{
	SetWindowText("");
	__super::SetIcon(CSize(32, 32), hIcon, NULL, FALSE);
}

//////////////////////////////////////////////////////////////////////////
void CCustomButton::SetToolTip(tukk tooltipText)
{

}

CCustomButton::CCustomButton()
{
	SetTheme(xtpButtonThemeOffice2003);
}

