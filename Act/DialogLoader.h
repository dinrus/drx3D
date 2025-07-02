// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Act/DialogScript.h>

class CDialogSystem;

class CDialogLoader
{
public:
	CDialogLoader(CDialogSystem* pDS);
	virtual ~CDialogLoader();

	// Loads all DialogScripts below a given path
	bool LoadScriptsFromPath(const string& path, TDialogScriptMap& outScripts);

	// Loads a single DialogScript
	bool LoadScript(const string& fileName, TDialogScriptMap& outScripts);

protected:

	// some intermediate data structure
	struct IMScriptLine
	{
		IMScriptLine() { Reset(); }
		void Reset()
		{
			audioID = DrxAudio::InvalidControlId;
			actor = anim = facial = lookat = "";
			delay = facialFadeTime = 0.0f;
			facialWeight = 0.5f;
		}

		bool IsValid() const { return (actor != 0 && *actor != 0); }

		tukk         actor;
		//tukk sound;
		DrxAudio::ControlId audioID;
		tukk         anim;
		tukk         lookat;
		float               delay;
		string              facial;
		float               facialWeight;
		float               facialFadeTime;
	};

	struct IMScript
	{
		IMScript() { Reset(); }
		void Reset()         { name = 0; lines.resize(0); }
		bool IsValid() const { return (name != 0 && *name != 0 && lines.size() > 0); }
		tukk               name;
		std::vector<IMScriptLine> lines;
	};

	// returns the number of successfully loaded scripts
	i32 LoadFromTable(XmlNodeRef tableNode, const string& fileName, TDialogScriptMap& outScriptMap);

	// preprocessing stuff
	bool ProcessScript(IMScript& theScript, const string& groupName, TDialogScriptMap& outScriptMap);

	// get actor from string [1-based]
	bool GetActor(tukk actor, i32& outID);
	bool GetLookAtActor(tukk actor, i32& outID, bool& outSticky);

protected:

	CDialogSystem* m_pDS;
};
