// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "CoverSurfaceManager.h"
#include "Objects/AICoverSurface.h"

u32k BAI_COVER_FILE_VERSION_WRITE = 2;

CCoverSurfaceManager::CCoverSurfaceManager()
{
}

CCoverSurfaceManager::~CCoverSurfaceManager()
{
}

void CCoverSurfaceManager::ClearGameSurfaces()
{
	gEnv->pAISystem->GetCoverSystem()->Clear();

	SurfaceObjects::iterator it = m_surfaceObjects.begin();
	SurfaceObjects::iterator end = m_surfaceObjects.end();

	for (; it != end; ++it)
	{
		CAICoverSurface* coverSurfaceObject = *it;
		coverSurfaceObject->SetSurfaceID(CoverSurfaceID(0));
	}
}

// This also compresses CoverSurfaceIDs down to a sequential range
bool CCoverSurfaceManager::WriteToFile(tukk fileName)
{
	CDrxFile file;

	if (!file.Open(fileName, "wb"))
	{
		Warning("Could not save AI Cover Surfaces. [%s]", fileName);

		return false;
	}

	ICoverSystem* coverSystem = gEnv->pAISystem->GetCoverSystem();

	u32 fileVersion = BAI_COVER_FILE_VERSION_WRITE;
	u32 surfaceCount = coverSystem->GetSurfaceCount();

	file.Write(&fileVersion, sizeof(fileVersion));
	file.Write(&surfaceCount, sizeof(surfaceCount));

	SurfaceObjects::iterator it = m_surfaceObjects.begin();
	SurfaceObjects::iterator end = m_surfaceObjects.end();

	// How this works:
	// 1. Save all valid surfaces in sequence to the file
	// 2. Assign a sequenced ID to all the CAICoverSurface objects
	//		Assign 0 to any invalid CAICoverSurface objects
	// 3. Load the valid surfaces again in sequence
	u32 runningID = 0;

	for (; it != end; ++it)
	{
		CAICoverSurface* coverSurfaceObject = *it;
		CoverSurfaceID surfaceID = coverSurfaceObject->GetSurfaceID();
		coverSurfaceObject->SetSurfaceID(CoverSurfaceID(0));

		if (!surfaceID)
			continue;

		ICoverSystem::SurfaceInfo surfaceInfo;
		if (!coverSystem->GetSurfaceInfo(surfaceID, &surfaceInfo))
			continue;

		file.Write(&surfaceInfo.sampleCount, sizeof(surfaceInfo.sampleCount));
		file.Write(&surfaceInfo.flags, sizeof(surfaceInfo.flags));

		for (uint i = 0; i < surfaceInfo.sampleCount; ++i)
		{
			const ICoverSampler::Sample& sample = surfaceInfo.samples[i];

			file.Write(&sample.position, sizeof(sample.position));
			file.Write(&sample.height, sizeof(sample.height));
			file.Write(&sample.flags, sizeof(sample.flags));
		}

		++runningID;
		coverSurfaceObject->SetSurfaceID(CoverSurfaceID(runningID));
	}

	file.Close();

	coverSystem->Clear();
	return coverSystem->ReadSurfacesFromFile(fileName);
}

bool CCoverSurfaceManager::ReadFromFile(tukk fileName)
{
	return gEnv->pAISystem->GetCoverSystem()->ReadSurfacesFromFile(fileName);
}

void CCoverSurfaceManager::AddSurfaceObject(CAICoverSurface* surface)
{
	m_surfaceObjects.insert(surface);
}

void CCoverSurfaceManager::RemoveSurfaceObject(CAICoverSurface* surface)
{
	m_surfaceObjects.erase(surface);
}

const CCoverSurfaceManager::SurfaceObjects& CCoverSurfaceManager::GetSurfaceObjects() const
{
	return m_surfaceObjects;
}

