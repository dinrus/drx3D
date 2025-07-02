// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace sxema
{
struct CVars
{
	static ICVar* sc_RootFolder;
	static i32    sc_IgnorePAKFiles;
	static i32    sc_IgnoreUnderscoredFolders;
	static i32    sc_EnableScriptPartitioning;

	static i32    sc_LogToFile;
	static ICVar* sc_LogFileStreams;
	static ICVar* sc_LogFileMessageTypes;
	static i32    sc_DisplayCriticalErrors;

	static i32    sc_RunUnitTests;
	static ICVar* sc_ExperimentalFeatures;

	static i32 sc_allowFlowGraphNodes;

	static void               Register();
	static void               Unregister();

	static inline tukk GetStringSafe(const ICVar* pCVar)
	{
		if (pCVar)
		{
			tukk szResult = pCVar->GetString();
			if (szResult)
			{
				return szResult;
			}
		}
		return "";
	}
};
} // sxema
