// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//	File:Log.h
//
//	История:
//	-Feb 2,2001:Created by Pavlo Gryb
//
//////////////////////////////////////////////////////////////////////

#ifndef PROFILELOG_H
#define PROFILELOG_H

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Sys/ISystem.h> // <> required for Interfuscator
#include <drx3D/Sys/ITimer.h>  // <> required for Interfuscator

struct ILogElement
{
	virtual ~ILogElement(){}
	virtual ILogElement* Log(tukk name, tukk message) = 0;
	virtual ILogElement* SetTime(float time) = 0;
	virtual void         Flush(stack_string& indent) = 0;
};

struct IProfileLogSystem
{
	virtual ~IProfileLogSystem(){}
	virtual ILogElement* Log(tukk name, tukk msg) = 0;
	virtual void         SetTime(ILogElement* pElement, float time) = 0;
	virtual void         Release() = 0;
};

struct SHierProfileLogItem
{
	SHierProfileLogItem(tukk name, tukk msg, i32 inbDoLog)
		: m_pLogElement(NULL)
		, m_bDoLog(inbDoLog)
	{
		if (m_bDoLog)
		{
			m_pLogElement = gEnv->pProfileLogSystem->Log(name, msg);
			m_startTime = gEnv->pTimer->GetAsyncTime();
		}
	}
	~SHierProfileLogItem()
	{
		if (m_bDoLog)
		{
			CTimeValue endTime = gEnv->pTimer->GetAsyncTime();
			gEnv->pProfileLogSystem->SetTime(m_pLogElement, (endTime - m_startTime).GetMilliSeconds());
		}
	}

private:
	i32          m_bDoLog;
	CTimeValue   m_startTime;
	ILogElement* m_pLogElement;
};

#define HPROFILE_BEGIN(msg1, msg2, doLog) { SHierProfileLogItem __hier_profile_uniq_var_in_this_scope__(msg1, msg2, doLog);
#define HPROFILE_END()                    }

#define HPROFILE(msg1, msg2, doLog)       SHierProfileLogItem __hier_profile_uniq_var_in_this_scope__(msg1, msg2, doLog);

#endif
