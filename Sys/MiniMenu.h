// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MiniMenu.h
//  Created:     26/08/2009 by Timur.
//  Описание: Button implementation in the MiniGUI
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __MiniMenu_h__
#define __MiniMenu_h__

#include <drx3D/Sys/MiniGUI.h>
#include <drx3D/Sys/MiniButton.h>

MINIGUI_BEGIN

//////////////////////////////////////////////////////////////////////////
// Root window all other controls derive from
class CMiniMenu : public CMiniButton
{
public:
	CMiniMenu();

	//////////////////////////////////////////////////////////////////////////
	// CMiniCtrl interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual EMiniCtrlType GetType() const { return eCtrlType_Menu; }
	virtual void          SetRect(const Rect& rc);
	virtual void          OnPaint(CDrawContext& dc);
	virtual void          OnEvent(float x, float y, EMiniCtrlEvent event);
	virtual void          AddSubCtrl(IMiniCtrl* pCtrl);
	virtual void          Reset();
	virtual void          SaveState();
	virtual void          RestoreState();
	//////////////////////////////////////////////////////////////////////////

	//digital selection funcs
	CMiniMenu* UpdateSelection(EMiniCtrlEvent event);

	void       Open();
	void       Close();

protected:

protected:
	bool  m_bVisible;
	bool  m_bSubMenu;
	float m_menuWidth;

	i32   m_selectionIndex;
};

MINIGUI_END

#endif // __MiniMenu_h__
