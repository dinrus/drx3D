// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __entityprototypelibrary_h__
#define __entityprototypelibrary_h__
#pragma once

#include "BaseLibrary.h"

/** Library of prototypes.
 */
class SANDBOX_API CEntityPrototypeLibrary : public CBaseLibrary
{
public:
	CEntityPrototypeLibrary(CBaseLibraryManager* pManager) : CBaseLibrary(pManager) {};
	virtual bool Save();
	virtual bool Load(const string& filename);
	virtual void Serialize(XmlNodeRef& node, bool bLoading);

private:
};

#endif // __entityprototypelibrary_h__

