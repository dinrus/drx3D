// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DialogLoaderMK2.h
//  Version:     v1.00
//  Created:     07/07/2006 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Dialog Loader
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DIALOGLOADERMK2_H__
#define __DIALOGLOADERMK2_H__

#pragma once

#include <drx3D/Act/DialogScript.h>

class CDialogSystem;

class CDialogLoaderMK2
{
public:
	CDialogLoaderMK2(CDialogSystem* pDS);
	virtual ~CDialogLoaderMK2();

	// Loads all DialogScripts below a given path
	bool LoadScriptsFromPath(const string& path, TDialogScriptMap& outScripts, tukk levelName);

	// Loads a single DialogScript
	bool LoadScript(const string& stripPath, const string& fileName, TDialogScriptMap& outScripts);

protected:
	void InternalLoadFromPath(const string& stripPath, const string& path, TDialogScriptMap& outScriptMap, i32& numLoaded, tukk levelName);

	// get actor from string [1-based]
	bool GetActor(tukk actor, i32& outID);
	bool GetLookAtActor(tukk actor, i32& outID, bool& outSticky);
	bool ProcessScript(CDialogScript* pScript, const XmlNodeRef& rootNode);
	bool ReadLine(const XmlNodeRef& lineNode, CDialogScript::SScriptLine& outLine, tukk scriptID, i32 lineNumber);
	void ResetLine(CDialogScript::SScriptLine& scriptLine);

protected:

	CDialogSystem* m_pDS;
};

#endif
