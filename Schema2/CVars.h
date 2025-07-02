// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace sxema2
{
	struct CVars
	{
		static ICVar* sc_FileFormat;
		static ICVar* sc_RootFolder;
		static i32    sc_IgnorePAKFiles;
		static i32    sc_IgnoreUnderscoredFolders;
		static i32    sc_DiscardOnSave;

		static ICVar* sc_LogToFile;
		static ICVar* sc_LogFileStreams;
		static ICVar* sc_LogFileMessageTypes;
		static i32    sc_DisplayCriticalErrors;

		static i32    sc_RunUnitTests;
		static i32    sc_MaxRecursionDepth;
		static ICVar* sc_ExperimentalFeatures;
		
		static float  sc_FunctionTimeLimit;
		static i32    sc_UseNewGraphPipeline;

		static float  sc_RelevanceGridCellSize;
		static i32    sc_RelevanceGridDebugStatic;

		static void Register();
		static void Unregister();

		static inline tukk GetStringSafe(const ICVar* pCVar)
		{
			if(pCVar)
			{
				tukk szResult = pCVar->GetString();
				if(szResult)
				{
					return szResult;
				}
			}
			return "";
		}
	};
}
