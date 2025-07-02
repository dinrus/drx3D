// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "ColorCtrl.h"

class PLUGIN_API CCustomButton : public CXTPButton
{
public:
	CCustomButton();
	//! Set icon to display on button, (must not be shared).
	void SetIcon(HICON hIcon, i32 nIconAlign = BS_CENTER, bool bDrawText = false);
	void SetIcon(LPCSTR lpszIconResource, i32 nIconAlign = BS_CENTER, bool bDrawText = false);
	void SetToolTip(tukk tooltipText);
};
typedef CCustomButton CColoredPushButton;

