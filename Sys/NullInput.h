// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef NULL_INPUT_H
#define NULL_INPUT_H

#pragma once
#include <drx3D/Input/IInput.h>

class CNullInput : public IInput
{
public:
	virtual void                 AddEventListener(IInputEventListener* pListener) override                                                                   {}
	virtual void                 RemoveEventListener(IInputEventListener* pListener) override                                                                {}

	virtual void                 AddConsoleEventListener(IInputEventListener* pListener) override                                                            {}
	virtual void                 RemoveConsoleEventListener(IInputEventListener* pListener) override                                                         {}

	virtual bool                 AddTouchEventListener(ITouchEventListener* pListener, tukk name) override                                            { return false; };
	virtual void                 RemoveTouchEventListener(ITouchEventListener* pListener) override                                                           {}

	virtual void                 SetExclusiveListener(IInputEventListener* pListener) override                                                               {}
	virtual IInputEventListener* GetExclusiveListener() override                                                                                             { return nullptr; }

	virtual bool                 AddInputDevice(IInputDevice* pDevice) override                                                                              { return false; }
	virtual bool                 RemoveInputDevice(IInputDevice* pDevice) override                                                                           { return false; }
	
	virtual void                 EnableEventPosting(bool bEnable) override                                                                                   {}
	virtual bool                 IsEventPostingEnabled() const override                                                                                      { return false; }
	virtual void                 PostInputEvent(const SInputEvent& event, bool bForce = false) override                                                      {}
	virtual void                 PostTouchEvent(const STouchEvent& event, bool bForce = false) override                                                      {}
	virtual void                 PostUnicodeEvent(const SUnicodeEvent& event, bool bForce = false) override                                                  {}

	virtual void                 ForceFeedbackEvent(const SFFOutputEvent& event) override                                                                    {}
	virtual void                 ForceFeedbackSetDeviceIndex(i32 index) override                                                                             {};

	virtual bool                 Init() override                                                                                                             { return true; }
	virtual void                 PostInit() override                                                                                                         {}
	virtual void                 Update(bool bFocus) override                                                                                                {}
	virtual void                 ShutDown() override                                                                                                         {}

	virtual void                 SetExclusiveMode(EInputDeviceType deviceType, bool exclusive, uk hwnd = 0) override                                      {}

	virtual bool                 InputState(const TKeyName& key, EInputState state) override                                                                 { return false; }

	virtual tukk          GetKeyName(const SInputEvent& event) const override                                                                         { return nullptr; }

	virtual tukk          GetKeyName(EKeyId keyId) const override                                                                                     { return nullptr; }

	virtual u32               GetInputCharUnicode(const SInputEvent& event) override                                                                      { return 0; }

	virtual SInputSymbol*        LookupSymbol(EInputDeviceType deviceType, i32 deviceIndex, EKeyId keyId) override                                           { return nullptr; }

	virtual const SInputSymbol*  GetSymbolByName(tukk name) const override                                                                            { return nullptr; }

	virtual tukk          GetOSKeyName(const SInputEvent& event) override                                                                             { return nullptr; }

	virtual void                 ClearKeyState() override                                                                                                    {}

	virtual void                 ClearAnalogKeyState() override                                                                                              {}

	virtual void                 RetriggerKeyState() override                                                                                                {}

	virtual bool                 Retriggering() override                                                                                                     { return false; }

	virtual bool                 HasInputDeviceOfType(EInputDeviceType type) override                                                                        { return false; }

	virtual i32                  GetModifiers() const override                                                                                               { return 0; }

	virtual void                 EnableDevice(EInputDeviceType deviceType, bool enable) override                                                             {}

	virtual void                 SetDeadZone(float fThreshold) override                                                                                      {}

	virtual void                 RestoreDefaultDeadZone() override                                                                                           {}

	virtual IInputDevice*        GetDevice(u16 id, EInputDeviceType deviceType) override                                                                  { return nullptr; }

	virtual u32               GetPlatformFlags() const override                                                                                           { return 0; }

	virtual IKinectInput*        GetKinectInput() override                                                                                                   { return nullptr; }
	virtual INaturalPointInput*  GetNaturalPointInput() override                                                                                             { return nullptr; }
	// Input blocking functionality
	virtual bool                 SetBlockingInput(const SInputBlockData& inputBlockData) override                                                            { return false; }
	virtual bool                 RemoveBlockingInput(const SInputBlockData& inputBlockData) override                                                         { return false; }
	virtual bool                 HasBlockingInput(const SInputBlockData& inputBlockData) const override                                                      { return false; }
	virtual i32                  GetNumBlockingInputs() const override                                                                                       { return 0; }
	virtual void                 ClearBlockingInputs() override                                                                                              {}
	virtual bool                 ShouldBlockInputEventPosting(const EKeyId keyId, const EInputDeviceType deviceType, u8k deviceIndex) const override { return false; }

	virtual void                 ProcessKey(u32 key, bool pressed, WCHAR unicode, bool repeat) override                                                   {}
	virtual bool                 GrabInput(bool bGrab) override                                                                                              { return false; }

	virtual i32                  ShowCursor(const bool bShow) override                                                                                       { return 0; }
};

#endif
