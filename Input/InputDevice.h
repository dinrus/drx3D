// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	Some useful and shared functionality for input devices.
   -------------------------------------------------------------------------
   История:
   - Dec 16,2005:	Created by Marco Koegler

*************************************************************************/
#ifndef __INPUTDEVICE_H__
#define __INPUTDEVICE_H__
#pragma once

#include <drx3D/Input/IInput.h>

#if !defined(RELEASE)
	#define DEBUG_CONTROLLER_LOG_BUTTON_ACTION(_SYMBOL) m_debugButtonsHistory.Add(_SYMBOL);
	#define DEBUG_CONTROLLER_RENDER_BUTTON_ACTION m_debugButtonsHistory.DebugRender();
#else
	#define DEBUG_CONTROLLER_LOG_BUTTON_ACTION(_SYMBOL)
	#define DEBUG_CONTROLLER_RENDER_BUTTON_ACTION
#endif

class CInputDevice : public IInputDevice
{
public:
	CInputDevice(IInput& input, tukk deviceName);
	virtual ~CInputDevice();

	// IInputDevice
	virtual tukk         GetDeviceName() const                       { return m_deviceName.c_str(); }
	virtual bool                IsOfDeviceType(EInputDeviceType type) const { return type == GetDeviceType(); }
	virtual EInputDeviceType    GetDeviceType() const                       { return m_deviceType; };
	virtual TInputDeviceId      GetDeviceId() const                         { return m_deviceId; };
	virtual bool                Init()                                      { return true;  }
	virtual void                PostInit()                                  {}
	virtual void                Update(bool bFocus);
	virtual bool                SetForceFeedback(IFFParams params)          { return false; };
	virtual bool                InputState(const TKeyName& key, EInputState state);
	virtual bool                SetExclusiveMode(bool value)                { return true;  }
	virtual void                ClearKeyState();
	virtual void                ClearAnalogKeyState(TInputSymbols& clearedSymbols);
	virtual void                SetUniqueId(u8 const uniqueId) { m_uniqueId = uniqueId; }
	virtual tukk         GetKeyName(const SInputEvent& event) const;
	virtual tukk         GetKeyName(const EKeyId keyId) const;
	virtual u32              GetInputCharUnicode(const SInputEvent& event);
	virtual tukk         GetOSKeyName(const SInputEvent& event);
	virtual SInputSymbol*       LookupSymbol(EKeyId id) const;
	virtual const SInputSymbol* GetSymbolByName(tukk name) const;
	virtual void                Enable(bool enable);
	virtual bool                IsEnabled() const             { return m_enabled; }
	virtual void                OnLanguageChange()            {};
	virtual void                SetDeadZone(float fThreshold) {};
	virtual void                RestoreDefaultDeadZone()      {};
	// ~IInputDevice

protected:
	IInput& GetIInput() const { return m_input;  }

	// device dependent id management
	//const TKeyName&	IdToName(TKeyId id) const;
	SInputSymbol* IdToSymbol(EKeyId id) const;
	u32        NameToId(const TKeyName& name) const;
	SInputSymbol* NameToSymbol(const TKeyName& name) const;
	SInputSymbol* DevSpecIdToSymbol(u32 devSpecId) const;
	SInputSymbol* MapSymbol(u32 deviceSpecificId, EKeyId keyId, const TKeyName& name, SInputSymbol::EType type = SInputSymbol::Button, u32 user = 0);

protected:
	EInputDeviceType m_deviceType;
	TInputDeviceId   m_deviceId;
	u8            m_uniqueId;
	bool             m_enabled;

#if !defined(RELEASE)
	class CDebugPressedButtons
	{
		enum EDebugFlags
		{
			eDF_Enabled        = BIT(0),
			eDF_LogChangeState = BIT(1),
		};

		struct SData
		{
			SData() : frame(0) {}
			SData(const SInputSymbol* pSymbol_, u32 frame_);

			u32 frame;
			ColorF color;
			string key;
			string state;
		};

		typedef std::vector<SData> TButtonsVector;
		enum { e_MaxNumEntries = 20, e_MaxReservedEntries = 32};

	public:
		CDebugPressedButtons() : m_frameCnt(0)
		{
			m_textPos2d.x = 50.f;
			m_textPos2d.y = 400.f;
			m_history.reserve(e_MaxReservedEntries); // try to ensure it does not try to allocate extra space when we are near the limit
		}

		void DebugRender();
		void Add(const SInputSymbol* pSymbol);

	private:
		u32         m_frameCnt;
		Vec2           m_textPos2d;
		TButtonsVector m_history;
	};
	CDebugPressedButtons m_debugButtonsHistory;
#endif

private:
	IInput& m_input;                          // point to input system in use
	string  m_deviceName;                     // name of the device (used for input binding)

	typedef std::map<TKeyName, u32>        TNameToIdMap;
	typedef std::map<TKeyName, SInputSymbol*> TNameToSymbolMap;
	typedef std::map<EKeyId, SInputSymbol*>   TIdToSymbolMap;
	typedef std::map<u32, SInputSymbol*>   TDevSpecIdToSymbolMap;

	TNameToIdMap          m_nameToId;
	TNameToSymbolMap      m_nameToInfo;
	TIdToSymbolMap        m_idToInfo;
	TDevSpecIdToSymbolMap m_devSpecIdToSymbol;

};

#endif //__INPUTDEVICE_H__
