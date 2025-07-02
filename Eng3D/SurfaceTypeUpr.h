// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   SurfaceUpr.h
//  Version:     v1.00
//  Created:     29/9/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __SurfaceUpr_h__
#define __SurfaceUpr_h__
#pragma once

#include <drx3D/Eng3D/I3DEngine.h>

#define MAX_SURFACE_ID 255

//////////////////////////////////////////////////////////////////////////
// SurfaceUpr is implementing ISurfaceUpr interface.
// Register and manages all known to game surface typs.
//////////////////////////////////////////////////////////////////////////
class CSurfaceTypeUpr : public ISurfaceTypeUpr, public DinrusX3dEngBase
{
public:
	CSurfaceTypeUpr();
	virtual ~CSurfaceTypeUpr();

	virtual void                    LoadSurfaceTypes();

	virtual ISurfaceType*           GetSurfaceTypeByName(tukk sName, tukk sWhy = NULL, bool warn = true);
	virtual ISurfaceType*           GetSurfaceType(i32 nSurfaceId, tukk sWhy = NULL);
	virtual ISurfaceTypeEnumerator* GetEnumerator();

	bool                            RegisterSurfaceType(ISurfaceType* pSurfaceType, bool bDefault = false);
	void                            UnregisterSurfaceType(ISurfaceType* pSurfaceType);

	ISurfaceType*                   GetSurfaceTypeFast(i32 nSurfaceId, tukk sWhy = NULL);

	void                            RemoveAll();

	void                            GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_nameToSurface);
		for (i32 i = 0; i < MAX_SURFACE_ID + 1; ++i)
			pSizer->AddObject(m_idToSurface[i]);
	}
private:
	i32                         m_lastSurfaceId;
	i32                         m_lastDefaultId;

	class CMaterialSurfaceType* m_pDefaultSurfaceType;

	void                        RegisterAllDefaultTypes();
	CMaterialSurfaceType*       RegisterDefaultType(tukk szName);
	void                        ResetSurfaceTypes();

	struct SSurfaceRecord
	{
		bool          bLoaded;
		ISurfaceType* pSurfaceType;

		void          GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(this, sizeof(*this));
		}
	};

	SSurfaceRecord* m_idToSurface[MAX_SURFACE_ID + 1];

	typedef std::map<string, SSurfaceRecord*> NameToSurfaceMap;
	NameToSurfaceMap m_nameToSurface;
};

#endif //__SurfaceUpr_h__
