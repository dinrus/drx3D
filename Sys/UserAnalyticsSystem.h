// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/IUserAnalytics.h>

#if !defined(_RELEASE) && DRX_PLATFORM_WINDOWS
#include <drx3D/Sys/IDrxPluginUpr.h>

struct IDrxUserAnalyticsPlugin;

class CUserAnalyticsSystem : public IUserAnalyticsSystem, public Drx::IPluginUpr::IEventListener
{
public:
	CUserAnalyticsSystem();
	~CUserAnalyticsSystem();

	virtual void TriggerEvent(tukk szEventName, UserAnalytics::Attributes* attributes) override;

	void         Initialize();
	void         RegisterCVars();

private:
	virtual void OnPluginEvent(const DrxClassID& pluginClassId, Drx::IPluginUpr::IEventListener::EEvent event) override;

	IDrxUserAnalyticsPlugin* m_pUserAnalyticsPlugin;
	IUserAnalytics*          m_pUserAnalytics;

	static i32               m_enableUserAnalyticsCollect;
	static i32               m_enableUserAnalyticsLogging;
};
#else
class CUserAnalyticsSystem : public IUserAnalyticsSystem
{
public:
	CUserAnalyticsSystem() {};
	~CUserAnalyticsSystem() {};

	virtual void TriggerEvent(tukk szEventName, UserAnalytics::Attributes* attributes) override {}

	void         Initialize()                                                                          {}
	void         RegisterCVars()                                                                       {}
};
#endif
