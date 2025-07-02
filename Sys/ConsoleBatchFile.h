// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Executes an ASCII batch file of console commands...

   -------------------------------------------------------------------------
   История:
   - 19:04:2006   10:38 : Created by Jan Müller

*************************************************************************/

#ifndef CONSOLE_BATCH_FILE
#define CONSOLE_BATCH_FILE

#if _MSC_VER > 1000
	#pragma once
#endif

struct IConsoleCmdArgs;
struct IConsole;

class CConsoleBatchFile
{
public:
	static void Init();
	static bool ExecuteConfigFile(tukk filename);

private:
	static void ExecuteFileCmdFunc(IConsoleCmdArgs* args);
	static IConsole* m_pConsole;
};

#endif
