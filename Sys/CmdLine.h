// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements the command line interface ICmdLine.

   -------------------------------------------------------------------------
   История:
   - 2:8:2004   15:19 : Created by Márcio Martins

*************************************************************************/
#ifndef __CMDLINE_H__
#define __CMDLINE_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Sys/ICmdLine.h>
#include <drx3D/Sys/CmdLineArg.h>

class CCmdLine :
	public ICmdLine
{
public:
	CCmdLine(tukk commandLine);
	virtual ~CCmdLine();

	virtual const ICmdLineArg* GetArg(i32 n) const;
	virtual i32                GetArgCount() const;
	virtual const ICmdLineArg* FindArg(const ECmdLineArgType ArgType, tukk name, bool caseSensitive = false) const;
	virtual tukk        GetCommandLine() const { return m_sCmdLine; };

private:
	void   PushCommand(const string& sCommand, const string& sParameter);
	string Next(tuk& str);

	string                   m_sCmdLine;
	std::vector<CCmdLineArg> m_args;

};

#endif //__CMDLINE_H__
