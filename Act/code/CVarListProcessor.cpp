// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/CVarListProcessor.h>

CCVarListProcessor::CCVarListProcessor(tukk path) : m_fileName(path)
{

}

CCVarListProcessor::~CCVarListProcessor()
{

}

void CCVarListProcessor::Process(ICVarListProcessorCallback* cb)
{
	FILE* f = gEnv->pDrxPak->FOpen(m_fileName, "rt");
	if (!f)
		return;

	static i32k BUFSZ = 4096;
	char buf[BUFSZ];

	size_t nRead;
	string cvar;
	bool comment = false;
	do
	{
		cvar.resize(0);
		buf[0] = '\0';
		nRead = gEnv->pDrxPak->FRead(buf, BUFSZ, f);

		for (size_t i = 0; i < nRead; i++)
		{
			char c = buf[i];
			if (comment)
			{
				if (c == '\r' || c == '\n')
					comment = false;
			}
			else
			{
				if (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
				{
					cvar += c;
				}
				else if (c == '\t' || c == '\r' || c == '\n' || c == ' ')
				{
					if (ICVar* pV = gEnv->pConsole->GetCVar(cvar.c_str()))
					{
						cb->OnCVar(pV);
						//DrxLog( "Unprotecting '%s'",cvar.c_str());
					}
					cvar.resize(0);
				}
				else if (c == '#')
				{
					comment = true;
				}
			}
		}
	}
	while (nRead != 0);

	if (!cvar.empty())
	{
		if (ICVar* pV = gEnv->pConsole->GetCVar(cvar.c_str()))
		{
			cb->OnCVar(pV);
			//DrxLog( "Unprotecting '%s'",cvar.c_str());
		}
	}

	gEnv->pDrxPak->FClose(f);
}
