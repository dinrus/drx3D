// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 1999-2014.
// -------------------------------------------------------------------------
//  File name:   ConsolePlugin.cpp
//  Version:     v1.00
//  Created:     03/03/2014 by Matthijs vd Meide
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio 2010
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#define DRX_USE_ATL
#define DRX_USE_MFC
#include <drx3D/CoreX/Platform/DrxAtlMfc.h>

#include <IEditorClassFactory.h>
#include "ConsolePlugin.h"
#include "ConsoleWindow.h"
#include "CVarWindow.h"

#define CONSOLE_WINDOW_NAME "Console"
#define CVAR_WINDOW_NAME    "CVars"
#define CATEGORY_NAME       "Console"

#include <drx3D/Sandbox/Editor/Plugin/QtViewPane.h>

#include <drx3D/CoreX/Platform/platform_impl.inl>

//single instance
CConsolePlugin* CConsolePlugin::s_pInstance;

REGISTER_VIEWPANE_FACTORY_AND_MENU(CConsoleWindow, "Console", "Tools", false, "Advanced")

REGISTER_PLUGIN(CConsolePlugin);

//create plugin instance
CConsolePlugin::CConsolePlugin() : CEngineListener(GetIEditor() ? GetIEditor()->GetSystem() : NULL)
{
	s_pInstance = this;
	if (GetIEditor())
	{
		Init(CONSOLE_MAX_HISTORY);
	}
}

//release plugin instance
CConsolePlugin::~CConsolePlugin()
{
	s_pInstance = NULL;
}

//deduce type of CVar we're dealing with
inline Messages::EVarType GetVarType(ICVar* pVar)
{
	if (pVar)
	{
		switch (pVar->GetType())
		{
		case CVAR_INT:
			return Messages::eVarType_Int;
		case CVAR_FLOAT:
			return Messages::eVarType_Float;
		case CVAR_STRING:
			return Messages::eVarType_String;
		}
	}
	return Messages::eVarType_None;
}

//handle auto-complete request
Messages::SAutoCompleteReply CConsolePlugin::HandleAutoCompleteRequest(const Messages::SAutoCompleteRequest& req)
{
	RefreshCVarsAndCommands();

	std::vector<Messages::SAutoCompleteReply::SItem> matches;
	AutoComplete(req.query, [&matches](const string& name, ICVar* pVar)
	{
		Messages::SAutoCompleteReply::SItem match;
		match.Set(name, GetVarType(pVar), pVar ? pVar->GetString() : NULL);
		matches.push_back(match);
	});
	Messages::SAutoCompleteReply msg;
	msg.Set(req.query, std::move(matches));
	return msg;
}

//get unique address
string CConsolePlugin::GetUniqueAddress() const
{
	char buf[20];
	static volatile i32 id;
	i32 unique = DrxInterlockedIncrement(&id);
	drx_sprintf(buf, "Console/%u", unique);
	return buf;
}
