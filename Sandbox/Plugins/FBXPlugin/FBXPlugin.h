// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2001-2012
// -------------------------------------------------------------------------
//  Created:     24 June 2011 by Sergiy Shaykin.
//  Description: Attachment to the Sandbox plug-in system
//
////////////////////////////////////////////////////////////////////////////
#include <IPlugin.h>

class CFBXPlugin : public IPlugin
{
public:
	CFBXPlugin();
	~CFBXPlugin();

	i32       GetPluginVersion() override;
	tukk GetPluginName() override;
	tukk GetPluginDescription() override;
};

