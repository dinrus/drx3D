// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __GameTokenLibrary_h__
#define __GameTokenLibrary_h__
#pragma once

#include "BaseLibrary.h"

//////////////////////////////////////////////////////////////////////////
class SANDBOX_API CGameTokenLibrary : public CBaseLibrary
{
public:
	CGameTokenLibrary(CBaseLibraryManager* pManager) : CBaseLibrary(pManager) {};
	virtual bool Save();
	virtual bool Load(const string& filename);
	virtual void Serialize(XmlNodeRef& node, bool bLoading);
private:
};

#endif // __GameTokenLibrary_h__

