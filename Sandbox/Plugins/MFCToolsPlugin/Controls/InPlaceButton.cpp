// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "InPlaceButton.h"
#include "Controls/SharedFonts.h"

// CInPlaceButton

IMPLEMENT_DYNAMIC(CInPlaceButton, CXTButton)

BEGIN_MESSAGE_MAP(CInPlaceButton, CXTPButton)
ON_CONTROL_REFLECT(BN_CLICKED, OnBnClicked)
ON_WM_CREATE()
END_MESSAGE_MAP()

CInPlaceButton::CInPlaceButton(OnClick onClickFunctor)
{
	m_onClick = onClickFunctor;
	SetTheme(xtpButtonThemeOffice2003);
}

CInPlaceButton::CInPlaceButton(OnClick onClickFunctor, bool bNoTheme)
{
	m_onClick = onClickFunctor;
}
i32 CInPlaceButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetFont(CFont::FromHandle(gMFCFonts.hSystemFont));
	return 0;
}

IMPLEMENT_DYNAMIC(CInPlaceColorButton, CColorButton)
BEGIN_MESSAGE_MAP(CInPlaceColorButton, CColorButton)
ON_CONTROL_REFLECT(BN_CLICKED, OnBnClicked)
ON_WM_CREATE()
ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
i32 CInPlaceColorButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;
	SetFont(CFont::FromHandle(gMFCFonts.hSystemFont));
	return 0;
}

//////////////////////////////////////////////////////////////////////////
BOOL CInPlaceColorButton::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

