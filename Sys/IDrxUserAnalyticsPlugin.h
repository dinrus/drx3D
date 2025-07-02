// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/IDrxPlugin.h>

struct IUserAnalytics;

struct IDrxUserAnalyticsPlugin : public Drx::IEnginePlugin
{
	DRXINTERFACE_DECLARE_GUID(IDrxUserAnalyticsPlugin, "c97ad475-fd95-416d-9c30-48b2d2c5b7f6"_drx_guid);

public:
	virtual IUserAnalytics*         GetIUserAnalytics() const = 0;
};
