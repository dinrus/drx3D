// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MiniGUI.h
//  Created:     26/08/2009 by Timur.
//  Описание: Interface to the Mini GUI subsystem
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __MiniGUI_h__
#define __MiniGUI_h__

#include <drx3D/Sys/IDrxMiniGUI.h>
#include <drx3D/CoreX/Math/Drx_Color.h>
#include <drx3D/Input/IHardwareMouse.h>
#include <drx3D/Input/IInput.h>

#include <drx3D/CoreX/Extension/ClassWeaver.h>

MINIGUI_BEGIN

class CMiniMenu;

//////////////////////////////////////////////////////////////////////////
// Root window all other controls derive from
class CMiniCtrl : public IMiniCtrl
{
public:

	CMiniCtrl() :
		m_nFlags(0),
		m_id(0),
		m_pGUI(NULL),
		m_renderCallback(NULL),
		m_fTextSize(12.f),
		m_prevX(0.f),
		m_prevY(0.f),
		m_moving(false),
		m_requiresResize(false),
		m_pCloseButton(NULL),
		m_saveStateOn(false)
	{};

	//////////////////////////////////////////////////////////////////////////
	// IMiniCtrl interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void        Reset();
	virtual void        SaveState();
	virtual void        RestoreState();

	virtual void        SetGUI(IMiniGUI* pGUI)      { m_pGUI = pGUI; };
	virtual IMiniGUI*   GetGUI() const              { return m_pGUI; };

	virtual i32         GetId() const               { return m_id; };
	virtual void        SetId(i32 id)               { m_id = id; };

	virtual tukk GetTitle() const            { return m_title; };
	virtual void        SetTitle(tukk title) { m_title = title; };

	virtual Rect        GetRect() const             { return m_rect; }
	virtual void        SetRect(const Rect& rc);

	virtual void        SetFlag(u32 flag)         { set_flag(flag); }
	virtual void        ClearFlag(u32 flag)       { clear_flag(flag); };
	virtual bool        CheckFlag(u32 flag) const { return is_flag(flag); }

	virtual void        AddSubCtrl(IMiniCtrl* pCtrl);
	virtual void        RemoveSubCtrl(IMiniCtrl* pCtrl);
	virtual void        RemoveAllSubCtrl();
	virtual i32         GetSubCtrlCount() const;
	virtual IMiniCtrl*  GetSubCtrl(i32 nIndex) const;
	virtual IMiniCtrl*  GetParent() const { return m_pParent; };

	virtual IMiniCtrl*  GetCtrlFromPoint(float x, float y);

	virtual void        SetVisible(bool state);

	virtual void OnEvent(float x, float y, EMiniCtrlEvent);

	virtual bool SetRenderCallback(RenderCallback callback) { m_renderCallback = callback; return true; };

	// Not implemented in base control
	virtual bool SetControlCVar(tukk sCVarName, float fOffValue, float fOnValue) { assert(0); return false; };
	virtual bool SetClickCallback(ClickCallback callback, uk pCallbackData)          { assert(0); return false; };
	virtual bool SetConnectedCtrl(IMiniCtrl* pConnectedCtrl)                            { assert(0); return false; };

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	virtual void AutoResize();

	//////////////////////////////////////////////////////////////////////////
	virtual void CreateCloseButton();

	void         DrawCtrl(CDrawContext& dc);

	virtual void Move(float x, float y);

protected:
	void set_flag(u32 flag)      { m_nFlags |= flag; }
	void clear_flag(u32 flag)    { m_nFlags &= ~flag; };
	bool is_flag(u32 flag) const { return (m_nFlags & flag) == flag; }

	//dynamic movement
	void StartMoving(float x, float y);
	void StopMoving();

protected:
	i32                       m_id;
	IMiniGUI*                 m_pGUI;
	u32                    m_nFlags;
	DrxFixedStringT<32>       m_title;
	Rect                      m_rect;
	_smart_ptr<IMiniCtrl>     m_pParent;
	std::vector<IMiniCtrlPtr> m_subCtrls;
	RenderCallback            m_renderCallback;
	float                     m_fTextSize;

	//optional close 'X' button on controls, ref counted by m_subCtrls
	IMiniCtrl* m_pCloseButton;

	//dynamic movement
	float m_prevX;
	float m_prevY;
	bool  m_moving;
	bool  m_requiresResize;
	bool  m_saveStateOn;
};

//////////////////////////////////////////////////////////////////////////
class CMiniGUI : public IMiniGUI, IHardwareMouseEventListener, public IInputEventListener
{
public:
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IMiniGUI)
	DRXINTERFACE_END()
	DRXGENERATE_SINGLETONCLASS_GUID(CMiniGUI, "MiniGUI", "1a049b87-9a4e-4b58-ac14-026e17e6255e"_drx_guid)

	CMiniGUI();
	virtual ~CMiniGUI() {}

public:
	void InitMetrics();
	void ProcessInput();

	//////////////////////////////////////////////////////////////////////////
	// IMiniGUI interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void       Init() override;
	virtual void       Done() override;
	virtual void       Draw() override;
	virtual void       Reset() override;
	virtual void       SaveState() override;
	virtual void       RestoreState() override;
	virtual void       SetEnabled(bool status) override;
	virtual void       SetInFocus(bool status) override;
	virtual bool       InFocus() override { return m_inFocus; }

	virtual void       SetEventListener(IMiniGUIEventListener* pListener) override;

	virtual SMetrics&  Metrics() override;

	virtual void       OnCommand(SCommand& cmd) override;

	virtual void       RemoveAllCtrl() override;
	virtual IMiniCtrl* CreateCtrl(IMiniCtrl* pParentCtrl, i32 nCtrlID, EMiniCtrlType type, i32 nCtrlFlags, const Rect& rc, tukk title) override;

	virtual IMiniCtrl* GetCtrlFromPoint(float x, float y) const override;

	void               SetHighlight(IMiniCtrl* pCtrl, bool bEnable, float x, float y);
	void               SetFocus(IMiniCtrl* pCtrl, bool bEnable);

	//////////////////////////////////////////////////////////////////////////
	// IHardwareMouseEventListener
	//////////////////////////////////////////////////////////////////////////
	virtual void OnHardwareMouseEvent(i32 iX, i32 iY, EHARDWAREMOUSEEVENT eHardwareMouseEvent, i32 wheelDelta = 0) override;
	//////////////////////////////////////////////////////////////////////////

	// IInputEventListener
	virtual bool OnInputEvent(const SInputEvent& rInputEvent) override;

	virtual void SetMovingCtrl(IMiniCtrl* pCtrl) override
	{
		m_pMovingCtrl = pCtrl;
	}

protected:

	//DPad menu navigation
	void UpdateDPadMenu(const SInputEvent& rInputEvent);
	void SetDPadMenu(IMiniCtrl* pMenu);
	void CloseDPadMenu();

protected:
	bool                             m_bListenersRegistered;
	bool                             m_enabled;
	bool                             m_inFocus;

	SMetrics                         m_metrics;

	_smart_ptr<CMiniCtrl>            m_pRootCtrl;

	_smart_ptr<IMiniCtrl>            m_highlightedCtrl;
	_smart_ptr<IMiniCtrl>            m_focusCtrl;

	IMiniGUIEventListener*           m_pEventListener;

	CMiniMenu*                       m_pDPadMenu;
	IMiniCtrl*                       m_pMovingCtrl;
	std::vector<minigui::IMiniCtrl*> m_rootMenus;
};

MINIGUI_END

#endif // __MiniGUI_h__
