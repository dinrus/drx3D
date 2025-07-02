// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "IPlugin.h"

class CameraTransformEvent;

class C3DConnexionDriver : public IPlugin, public IAutoEditorNotifyListener
{
public:
	C3DConnexionDriver();
	~C3DConnexionDriver();

	bool        InitDevice();
	bool        GetInputMessageData(uk message, long* returnVal);

	i32       GetPluginVersion() override { return 1; };
	tukk GetPluginName() override { return "3DConnexionDriver"; };
	tukk GetPluginDescription() override { return "3DConnexionDriver"; };
	void        OnEditorNotifyEvent(EEditorNotifyEvent aEventId) override;

private:
	PRAWINPUTDEVICELIST           m_pRawInputDeviceList;
	PRAWINPUTDEVICE               m_pRawInputDevices;
	i32                           m_nUsagePage1Usage8Devices;
	float                         m_fMultiplier;
};

