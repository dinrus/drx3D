// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MiniButton.h
//  Created:     26/08/2009 by Timur.
//  Описание: Button implementation in the MiniGUI
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __MiniButton_h__
#define __MiniButton_h__

#include <drx3D/Sys/MiniGUI.h>

MINIGUI_BEGIN

//////////////////////////////////////////////////////////////////////////
// Root window all other controls derive from
class CMiniButton : public CMiniCtrl
{
public:
	CMiniButton();

	//////////////////////////////////////////////////////////////////////////
	// CMiniCtrl interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual EMiniCtrlType GetType() const { return eCtrlType_Button; }
	virtual void          SetRect(const Rect& rc);
	virtual void          OnPaint(CDrawContext& dc);
	virtual void          OnEvent(float x, float y, EMiniCtrlEvent event);
	virtual void          Reset();
	virtual void          SaveState();
	virtual void          RestoreState();
	//////////////////////////////////////////////////////////////////////////

	virtual bool SetControlCVar(tukk sCVarName, float fOffValue, float fOnValue);
	virtual bool SetClickCallback(ClickCallback callback, uk pCallbackData);
	virtual bool SetConnectedCtrl(IMiniCtrl* pConnectedCtrl);

protected:
	ICVar*        m_pCVar;
	float         m_fCVarValue[2];
	ClickCallback m_clickCallback;
	uk         m_pCallbackData;
	IMiniCtrl*    m_pConnectedCtrl;
};

MINIGUI_END

#endif // __MiniButton_h__
