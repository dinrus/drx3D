// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DialogLoaderMK2.cpp
//  Version:     v1.00
//  Created:     07/07/2006 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Dialog Loader
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/DialogLoaderMK2.h>
#include <drx3D/Act/DialogCommon.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/Act/ILevelSystem.h>

////////////////////////////////////////////////////////////////////////////
CDialogLoaderMK2::CDialogLoaderMK2(CDialogSystem* pDS) : m_pDS(pDS)
{

}

////////////////////////////////////////////////////////////////////////////
CDialogLoaderMK2::~CDialogLoaderMK2()
{

}

void CDialogLoaderMK2::InternalLoadFromPath(const string& stripPath, const string& path, TDialogScriptMap& outScriptMap, i32& numLoaded, tukk levelName)
{
	IDrxPak* pDrxPak = gEnv->pDrxPak;
	_finddata_t fd;

	string realPath(path);
	realPath.TrimRight("/\\");
	string search(realPath);
	search += "/*.*";

	intptr_t handle = pDrxPak->FindFirst(search.c_str(), &fd);
	if (handle != -1)
	{
		do
		{
			if (strcmp(fd.name, ".") == 0 || strcmp(fd.name, "..") == 0)
				continue;

			if (fd.attrib & _A_SUBDIR)
			{
				if (!gEnv->IsEditor()) //only load current levels dialogs
				{
					if (!levelName || stricmp(levelName, fd.name))
						continue;
				}

				string subPath = realPath;
				subPath += "/";
				subPath += fd.name;
				InternalLoadFromPath(stripPath, subPath, outScriptMap, numLoaded, levelName);
				continue;
			}

			if (stricmp(PathUtil::GetExt(fd.name), "dlg") != 0)
				continue;

			// fd.name contains the profile name
			string filename = realPath;
			filename += "/";
			filename += fd.name;
			bool ok = LoadScript(stripPath, filename, outScriptMap);
			if (ok)
				++numLoaded;
		}
		while (pDrxPak->FindNext(handle, &fd) >= 0);

		pDrxPak->FindClose(handle);
	}
}

////////////////////////////////////////////////////////////////////////////
bool CDialogLoaderMK2::LoadScriptsFromPath(const string& path, TDialogScriptMap& outScriptMap, tukk levelName)
{
	string stripPath = path;
	PathUtil::ToUnixPath(stripPath);
	stripPath.TrimRight("/\\");
	stripPath += "/";

	i32 numLoaded = 0;
	InternalLoadFromPath(stripPath, path, outScriptMap, numLoaded, levelName);
	return numLoaded > 0;
}

////////////////////////////////////////////////////////////////////////////
bool CDialogLoaderMK2::LoadScript(const string& stripPath, const string& filename, TDialogScriptMap& outScriptMap)
{
	XmlNodeRef rootNode = GetISystem()->LoadXmlFromFile(filename);
	if (!rootNode)
	{
		GameWarning("[DIALOG] CDialogLoaderMK2::LoadScripts: Cannot find/load file '%s'", filename.c_str());
		return false;
	}

	if (rootNode->isTag("DialogScript") == false)
	{
		GameWarning("[DIALOG] CDialogLoaderMK2::LoadScripts: File '%s' not a dialog script.", filename.c_str());
		return false;
	}

	string scriptName = PathUtil::ToUnixPath(filename);
	// now remove prefix
	if (DrxStringUtils::stristr(scriptName.c_str(), stripPath.c_str()) == scriptName.c_str())
		scriptName = scriptName.Mid(stripPath.length());

	PathUtil::RemoveExtension(scriptName);
	scriptName.replace('/', '.');

	// Make nice uppercase name, if storedId and filename match case-insensitive
	tukk storedId = rootNode->getAttr("Name");
	if (storedId != 0 && stricmp(storedId, scriptName.c_str()) == 0)
	{
		scriptName.assign(storedId);
	}

	CDialogScript* pScript = new CDialogScript(scriptName);
	bool bOK = ProcessScript(pScript, rootNode);

	if (bOK)
	{
		// try to complete the script
		if (pScript->Complete() == true)
		{
			// add it to the map
			std::pair<TDialogScriptMap::iterator, bool> inserted =
			  outScriptMap.insert(TDialogScriptMap::value_type(pScript->GetID(), pScript));
			if (inserted.second == false)
			{
				bOK = false;
				GameWarning("[DIALOG] CDialogLoaderMK2::ProcessScript '%s': Script already defined. Discarded", scriptName.c_str());
			}
		}
		// completion not successful -> discard
		else
		{
			bOK = false;
			GameWarning("[DIALOG] CDialogLoaderMK2::ProcessScript '%s': Cannot complete Script. Discarded.", scriptName.c_str());
		}
	}

	// discard pScript
	if (bOK == false)
	{
		delete pScript;
	}

	return bOK;
}

// returns actor's ID in outID [1 based]
// or 0, when not found
////////////////////////////////////////////////////////////////////////////
bool CDialogLoaderMK2::GetActor(tukk actor, i32& outID)
{
	static tukk actorPrefix = "actor";
	static i32k actorPrefixLen = strlen(actorPrefix);

	assert(actor != 0 && *actor != '\0');
	if (actor == 0 || *actor == '\0') // safety
	{
		outID = 0;
		return true;
	}

	tukk found = DrxStringUtils::stristr(actor, actorPrefix);
	if (found && sscanf(found + actorPrefixLen, "%d", &outID) == 1)
		return true;
	outID = 0;
	return false;
}

// returns actor's ID in outID [1 based]
// or 0, when not found
////////////////////////////////////////////////////////////////////////////
bool CDialogLoaderMK2::GetLookAtActor(tukk actor, i32& outID, bool& outSticky)
{
	static tukk actorPrefix = "actor";
	static i32k actorPrefixLen = strlen(actorPrefix);

	assert(actor != 0 && *actor != '\0');
	if (actor == 0 || *actor == '\0') // safety
	{
		outID = 0;
		outSticky = false;
		return true;
	}

	if (actor[0] == '$')
	{
		outSticky = true;
		++actor;
	}
	else
		outSticky = false;

	tukk found = DrxStringUtils::stristr(actor, actorPrefix);
	if (found && sscanf(found + actorPrefixLen, "%d", &outID) == 1)
		return true;
	outID = 0;
	return false;
}

////////////////////////////////////////////////////////////////////////////
bool CDialogLoaderMK2::ProcessScript(CDialogScript* pScript, const XmlNodeRef& node)
{
	CDialogScript::SScriptLine scriptLine;

	tukk scriptID = pScript->GetID();
	string desc = node->getAttr("Description");
	pScript->SetDescription(desc);

	for (i32 i = 0; i < node->getChildCount(); ++i)
	{
		XmlNodeRef lineNode = node->getChild(i);
		if (lineNode && lineNode->isTag("Line"))
		{
			ResetLine(scriptLine);
			if (ReadLine(lineNode, scriptLine, scriptID, i) == true)
			{
				pScript->AddLine(scriptLine);
			}
		}
	}
	return true;
}

void CDialogLoaderMK2::ResetLine(CDialogScript::SScriptLine& scriptLine)
{
	scriptLine.m_actor = CDialogScript::NO_ACTOR_ID;
	scriptLine.m_lookatActor = CDialogScript::NO_ACTOR_ID;
	scriptLine.m_flagLookAtSticky = false;
	scriptLine.m_flagResetFacial = false;
	scriptLine.m_flagResetLookAt = false;
	scriptLine.m_flagSoundStopsAnim = false;
	scriptLine.m_flagAGSignal = false;
	scriptLine.m_flagAGEP = false;
	scriptLine.m_audioID = DrxAudio::InvalidControlId;
	//scriptLine.m_sound = "";
	scriptLine.m_anim = "";
	scriptLine.m_facial = "";
	scriptLine.m_delay = 0.0f;
	scriptLine.m_facialWeight = 0.0f;
	scriptLine.m_facialFadeTime = 0.0f;
}

bool CDialogLoaderMK2::ReadLine(const XmlNodeRef& lineNode, CDialogScript::SScriptLine& line, tukk scriptID, i32 lineNumber)
{
	if (lineNode->getAttr("actor", line.m_actor) == false)
	{
		GameWarning("[DIALOG] CDialogLoaderMK2::ProcessScript '%s': No actor given in line %d", scriptID, lineNumber);
		return false;
	}

	line.m_flagResetLookAt = false;
	if (lineNode->getAttr("lookatActor", line.m_lookatActor))
	{
		if (line.m_lookatActor == CDialogScript::STICKY_LOOKAT_RESET_ID)
		{
			line.m_flagResetLookAt = true;
			line.m_lookatActor = CDialogScript::NO_ACTOR_ID;
		}
	}

	bool tmp;
	if (lineNode->getAttr("flagLookAtSticky", tmp)) line.m_flagLookAtSticky = tmp;
	if (lineNode->getAttr("flagSoundStopsAnim", tmp)) line.m_flagSoundStopsAnim = tmp;
	if (lineNode->getAttr("flagAGSignal", tmp)) line.m_flagAGSignal = tmp;
	if (lineNode->getAttr("flagAGEP", tmp)) line.m_flagAGEP = tmp;
	tukk szTriggerName = lineNode->getAttr("audioID");
	if (szTriggerName != nullptr && szTriggerName[0] != '\0')
	{
		line.m_audioID = DrxAudio::StringToId(szTriggerName);
	}
	else
	{
		line.m_audioID = DrxAudio::InvalidControlId;
	}

	line.m_anim = lineNode->getAttr("anim");
	line.m_facial = lineNode->getAttr("facial");
	if (DrxStringUtils::stristr(line.m_facial, "#RESET#") != 0)
	{
		line.m_flagResetFacial = true;
		line.m_facial = "";
	}
	lineNode->getAttr("delay", line.m_delay);
	lineNode->getAttr("facialWeight", line.m_facialWeight);
	lineNode->getAttr("facialFadeTime", line.m_facialFadeTime);
	return true;
}
