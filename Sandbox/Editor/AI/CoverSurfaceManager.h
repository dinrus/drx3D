// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CoverSurfaceManager_h__
#define __CoverSurfaceManager_h__

#pragma once

#include <DrxAISystem/ICoverSystem.h>

class CAICoverSurface;
class CCoverSurfaceManager
{
public:
	CCoverSurfaceManager();
	virtual ~CCoverSurfaceManager();

	bool WriteToFile(tukk fileName);
	bool ReadFromFile(tukk fileName);

	void ClearGameSurfaces();
	void AddSurfaceObject(CAICoverSurface* surface);
	void RemoveSurfaceObject(CAICoverSurface* surface);

	typedef std::set<CAICoverSurface*> SurfaceObjects;
	const SurfaceObjects& GetSurfaceObjects() const;

private:
	SurfaceObjects m_surfaceObjects;
};

#endif

