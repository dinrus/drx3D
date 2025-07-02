// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 1999-2014.
// -------------------------------------------------------------------------
//  File name:   ConsolePlugin.h
//  Version:     v1.00
//  Created:     03/03/2014 by Matthijs vd Meide
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio 2010
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#include <IEditor.h>
#include <IPlugin.h>
#include "EngineListener.h"
#include "Messages.h"

//console plugin class, UI side
class CConsolePlugin : public IPlugin, public CEngineListener
{
	//called back by the EngineListener when we need to send messages
	void EmitCVar(ICVar* pVar) const                      {}
	void EmitLine(size_t index, const string& line) const {}
	void DestroyCVar(ICVar* pVar) const                   {}
	
public:
	CConsolePlugin();
	~CConsolePlugin();

	Messages::SAutoCompleteReply HandleAutoCompleteRequest(const Messages::SAutoCompleteRequest& req);

	//the singleton instance of the plugin
	static CConsolePlugin* GetInstance() { return s_pInstance; }

	//get a unique address for messages
	string GetUniqueAddress() const;

	//IPlugin implementation
	i32       GetPluginVersion() override                          { return 1; }
	tukk GetPluginName() override                             { return "Console"; }
	tukk GetPluginDescription() override						 { return "Adds the Console window"; }

private:
	//singleton instance
	static CConsolePlugin* s_pInstance;
};

