// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//	File:ProfileLogSystem.h
//  Implementation of the IProfileLogSystem interface, which is used to
//  save hierarchical log with SHierProfileLogItem
//
//	История:
//
//////////////////////////////////////////////////////////////////////

#ifndef PROFILELOGSYSTEM_H
#define PROFILELOGSYSTEM_H

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Sys/ProfileLog.h>

class CLogElement : public ILogElement
{
public:
	CLogElement();
	CLogElement(CLogElement* pParent);
	CLogElement(CLogElement* pParent, tukk name, tukk message);

	virtual ILogElement* Log(tukk name, tukk message);
	virtual ILogElement* SetTime(float time);
	virtual void         Flush(stack_string& indent);

	void                 Clear();

	inline void          SetName(tukk name)
	{
		m_strName = name;
	}

	inline void SetMessage(tukk message)
	{
		m_strMessage = message;
	}

private:
	string                 m_strName;
	string                 m_strMessage;
	float                  m_time; // milliSeconds

	CLogElement*           m_pParent;
	std::list<CLogElement> m_logElements;
};

class CProfileLogSystem : public IProfileLogSystem
{
public:
	CProfileLogSystem();
	~CProfileLogSystem();

	virtual ILogElement* Log(tukk name, tukk message);
	virtual void         SetTime(ILogElement* pElement, float time);
	virtual void         Release();

private:
	CLogElement  m_rootElelent;
	ILogElement* m_pLastElelent;
};

#endif // PROFILELOGSYSTEM_H
