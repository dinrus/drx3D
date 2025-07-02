// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	BaseInput implementation. This is primarily a "get things to
              compile" thing for new platforms which haven't gotten a
              real input implementation done yet. It implements all
              the listener functionality and offers a uniform device
              interface using IInputDevice.
   -------------------------------------------------------------------------
   История:
   - Dec 05,2005:	Created by Marco Koegler

*************************************************************************/
#ifndef __BASEINPUT_H__
#define __BASEINPUT_H__
#pragma once

#include <drx3D/CoreX/Platform/platform.h>

#if DRX_PLATFORM_DURANGO
	#include "KinectInputWinRT.h"
#else
	#ifdef USE_KINECT
		#include "KinectInput.h"
	#else
		#include "KinectInputNULL.h"
	#endif
#endif

//#define USE_TRACKIR

#if (DRX_PLATFORM_WINDOWS && defined(USE_TRACKIR))
	#include "TrackIRInput.h"
typedef CTrackIRInput TNaturalPointInput;
#else
	#include "NaturalPointInputNULL.h"
typedef CNaturalPointInputNull TNaturalPointInput;
#endif

#include <drx3D/CoreX/Containers/DrxListenerSet.h>

struct  IInputDevice;
class CInputCVars;

class CBaseInput : public IInput, public ISystemEventListener
{
public:
	CBaseInput();
	virtual ~CBaseInput();

	// IInput
	// stub implementation
	virtual bool                Init();
	virtual void                PostInit();
	virtual void                Update(bool bFocus);
	virtual void                ShutDown();
	virtual void                SetExclusiveMode(EInputDeviceType deviceType, bool exclusive, uk pUser);
	virtual bool                InputState(const TKeyName& keyName, EInputState state);
	virtual tukk         GetKeyName(const SInputEvent& event) const;
	virtual tukk         GetKeyName(EKeyId keyId) const;
	virtual u32              GetInputCharUnicode(const SInputEvent& event);
	virtual SInputSymbol*       LookupSymbol(EInputDeviceType deviceType, i32 deviceIndex, EKeyId keyId);
	virtual const SInputSymbol* GetSymbolByName(tukk name) const;
	virtual tukk         GetOSKeyName(const SInputEvent& event);
	virtual void                ClearKeyState();
	virtual void                ClearAnalogKeyState();
	virtual void                RetriggerKeyState();
	virtual bool                Retriggering() { return m_retriggering;  }
	virtual bool                HasInputDeviceOfType(EInputDeviceType type);
	virtual void                SetDeadZone(float fThreshold);
	virtual void                RestoreDefaultDeadZone();
	virtual IInputDevice*       GetDevice(u16 id, EInputDeviceType deviceType);

	// listener functions (implemented)
	virtual void                 AddEventListener(IInputEventListener* pListener);
	virtual void                 RemoveEventListener(IInputEventListener* pListener);
	virtual bool                 AddTouchEventListener(ITouchEventListener* pListener, tukk name);
	virtual void                 RemoveTouchEventListener(ITouchEventListener* pListener);
	virtual void                 AddConsoleEventListener(IInputEventListener* pListener);
	virtual void                 RemoveConsoleEventListener(IInputEventListener* pLstener);
	virtual void                 SetExclusiveListener(IInputEventListener* pListener);
	virtual IInputEventListener* GetExclusiveListener();
	virtual bool                 AddInputDevice(IInputDevice* pDevice);
	virtual bool                 RemoveInputDevice(IInputDevice* pDevice);
	virtual void                 EnableEventPosting(bool bEnable);
	virtual bool                 IsEventPostingEnabled() const;
	virtual void                 PostInputEvent(const SInputEvent& event, bool bForce = false);
	virtual void                 PostTouchEvent(const STouchEvent& event, bool bForce = false);
	virtual void                 PostUnicodeEvent(const SUnicodeEvent& event, bool bForce = false);
	virtual void                 ForceFeedbackEvent(const SFFOutputEvent& event);
	virtual void                 ForceFeedbackSetDeviceIndex(i32 index);
	virtual void                 EnableDevice(EInputDeviceType deviceType, bool enable);
	virtual void                 ProcessKey(u32 key, bool pressed, wchar_t unicode, bool repeat) {};
	// ~IInput

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);
	// ~ISystemEventListener

	bool           HasFocus() const            { return m_hasFocus;  }
	i32            GetModifiers() const        { return m_modifiers; }
	void           SetModifiers(i32 modifiers) { m_modifiers = modifiers;  }

	virtual u32 GetPlatformFlags() const    { return m_platformFlags; }

	// Input blocking functionality
	virtual bool                SetBlockingInput(const SInputBlockData& inputBlockData);
	virtual bool                RemoveBlockingInput(const SInputBlockData& inputBlockData);
	virtual bool                HasBlockingInput(const SInputBlockData& inputBlockData) const;
	virtual i32                 GetNumBlockingInputs() const;
	virtual void                ClearBlockingInputs();
	virtual bool                ShouldBlockInputEventPosting(const EKeyId keyId, const EInputDeviceType deviceType, u8k deviceIndex) const;

	virtual IKinectInput*       GetKinectInput()       { return m_pKinectInput; }
	virtual IEyeTrackerInput*   GetEyeTrackerInput()   { return m_pEyeTrackerInput; }

	virtual INaturalPointInput* GetNaturalPointInput() { return m_pNaturalPointInput; }

	virtual bool                GrabInput(bool bGrab);

	virtual i32                 ShowCursor(const bool bShow) { return 0; }

protected:
	// Input blocking functionality
	void UpdateBlockingInputs();

	typedef std::vector<IInputDevice*> TInputDevices;
	void PostHoldEvents();
	void ClearHoldEvent(SInputSymbol* pSymbol);

private:
	bool        SendEventToListeners(const SInputEvent& event);
	bool        SendEventToListeners(const SUnicodeEvent& event);
	void        AddEventToHoldSymbols(const SInputEvent& event);
	void        RemoveDeviceHoldSymbols(EInputDeviceType deviceType, u8 deviceIndex);
	static bool OnFilterInputEventDummy(SInputEvent* pInput);

	// listener functionality
	typedef std::list<IInputEventListener*> TInputEventListeners;
	TInputSymbols                      m_holdSymbols;
	TInputEventListeners               m_listeners;
	TInputEventListeners               m_consoleListeners;
	IInputEventListener*               m_pExclusiveListener;
	CListenerSet<ITouchEventListener*> m_touchListeners;

	bool                               m_enableEventPosting;
	bool                               m_retriggering;
	DrxCriticalSection                 m_postInputEventMutex;

	bool                               m_hasFocus;

	// input device management
	TInputDevices m_inputDevices;

	//Filter for exclusive force-feedback output on an individual device
	i32 m_forceFeedbackDeviceIndex;

	// Input blocking functionality
	typedef std::list<SInputBlockData> TInputBlockData;
	TInputBlockData m_inputBlockData;

	// marcok: a bit nasty ... but I want to restrict access to CKeyboard ... this makes sure that
	// even mouse events could have a modifier
	i32 m_modifiers;

	//CVars
	CInputCVars* m_pCVars;

#if DRX_PLATFORM_DURANGO
	CKinectInputWinRT* m_pKinectInput;
#else
	#ifdef USE_KINECT
	CKinectInput*     m_pKinectInput;
	#else
	CKinectInputNULL* m_pKinectInput;
	#endif
#endif

	IEyeTrackerInput*   m_pEyeTrackerInput;

	TNaturalPointInput* m_pNaturalPointInput;

protected:
	u32 m_platformFlags;
};

#endif //__BASEINPUT_H__
