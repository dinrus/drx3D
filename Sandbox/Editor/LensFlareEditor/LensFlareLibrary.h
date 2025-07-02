// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2011.
// -------------------------------------------------------------------------
//  File name:   LensFlareLibrary.h
//  Created:     12/Dec/2012 by Jaesik.
////////////////////////////////////////////////////////////////////////////
#include "BaseLibrary.h"

class SANDBOX_API CLensFlareLibrary : public CBaseLibrary
{
public:
	CLensFlareLibrary(CBaseLibraryManager* pManager) : CBaseLibrary(pManager) {};
	virtual bool          Save();
	virtual bool          Load(const string& filename);
	virtual void          Serialize(XmlNodeRef& node, bool bLoading);

	IOpticsElementBasePtr GetOpticsOfItem(tukk szflareName);
};

