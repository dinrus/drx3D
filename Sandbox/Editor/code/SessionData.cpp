// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>

#include "QtUtil.h"

#include <drx3D/CoreX/Serialization/yasli/JSONOArchive.h>

namespace Private_SessionData
{

struct SSessionData
{
	// For now, command line arguments are sufficient.
	string commandLineArguments;

	void Serialize(yasli::Archive& ar)
	{
		ar(commandLineArguments, "cmdline");
	}
};

} //endns Private_SessionData

void WriteSessionData()
{
	Private_SessionData::SSessionData sessionData;
	sessionData.commandLineArguments = GetCommandLineA();

	yasli::JSONOArchive ar;
	ar(sessionData, "sessionData");

	const QString filePath = QtUtil::GetAppDataFolder() + "/LastSession.json";
	FILE* const pFile = fopen(filePath.toLocal8Bit().constData(), "w");
	if (pFile)
	{
		fprintf(pFile, "%s", ar.c_str());
		fclose(pFile);
	}
}

