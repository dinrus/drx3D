// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:

   -------------------------------------------------------------------------
   История:
   - 8:9:2004   10:32 : Created by Márcio Martins

*************************************************************************/
#ifndef __ACTIONFILTER_H__
#define __ACTIONFILTER_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "IActionMapUpr.h"

typedef std::set<ActionId> TFilterActions;

class CActionMapUpr;

class CActionFilter :
	public IActionFilter
{
public:
	CActionFilter(CActionMapUpr* pActionMapUpr, IInput* pInput, tukk name, EActionFilterType type = eAFT_ActionFail);
	virtual ~CActionFilter();

	// IActionFilter
	virtual void        Release() { delete this; };
	virtual void        Filter(const ActionId& action);
	virtual bool        SerializeXML(const XmlNodeRef& root, bool bLoading);
	virtual tukk GetName() { return m_name.c_str(); }
	virtual void        Enable(bool enable);
	virtual bool        Enabled() { return m_enabled; };
	// ~IActionFilter

	bool         ActionFiltered(const ActionId& action);

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const;

private:
	bool               m_enabled;
	CActionMapUpr* m_pActionMapUpr;
	IInput*            m_pInput;
	TFilterActions     m_filterActions;
	EActionFilterType  m_type;
	string             m_name;
};

#endif //__ACTIONFILTER_H__
