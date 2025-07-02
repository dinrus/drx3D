// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <EditorFramework/Preferences.h>
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

//////////////////////////////////////////////////////////////////////////
// General Preferences
//////////////////////////////////////////////////////////////////////////
struct SEditorGeneralPreferences : public SPreferencePage
{
	SEditorGeneralPreferences();
	virtual bool Serialize(yasli::Archive& ar) override;

	bool showWindowsInTaskbar;

	ADD_PREFERENCE_PAGE_PROPERTY(bool, enableSourceControl, setEnableSourceControl)
	ADD_PREFERENCE_PAGE_PROPERTY(bool, saveOnlyModified, setSaveOnlyModified)
	ADD_PREFERENCE_PAGE_PROPERTY(bool, freezeReadOnly, setFreezeReadOnly)
	ADD_PREFERENCE_PAGE_PROPERTY(bool, showTimeInConsole, setShowTimeInConsole)
};

//////////////////////////////////////////////////////////////////////////
// File Preferences
//////////////////////////////////////////////////////////////////////////
struct SEditorFilePreferences : public SPreferencePage
{
	SEditorFilePreferences();
	virtual bool Serialize(yasli::Archive& ar) override;

	bool   openCSharpSolution;
	string textEditorCSharp;
	string textEditorScript;
	string textEditorShaders;
	string textEditorBspaces;
	string textureEditor;
	string animEditor;
	string strStandardTempDirectory;

	i32    autoSaveMaxCount;
	bool   autoSaveEnabled;
	bool   filesBackup;

	ADD_PREFERENCE_PAGE_PROPERTY(i32, autoSaveTime, setAutoSaveTime)
};

EDITOR_COMMON_API extern SEditorGeneralPreferences gEditorGeneralPreferences;
EDITOR_COMMON_API extern SEditorFilePreferences gEditorFilePreferences;

