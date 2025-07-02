// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/WaterRippleUpr.h>

i32 CWaterRippleUpr::OnEventPhysCollision(const EventPhys* pEvent)
{
	EventPhysCollision* pEventPhysArea = (EventPhysCollision*)pEvent;
	if (!pEventPhysArea)
		return 1;

	// only add nearby hits
	if (pEventPhysArea->idmat[1] == gEnv->pPhysicalWorld->GetWaterMat())
	{
		// Compute the momentum of the object.
		// Clamp the mass so that particles and other "massless" objects still cause ripples.
		const float v = pEventPhysArea->vloc[0].GetLength();
		float fMomentum = v * max(pEventPhysArea->mass[0], 0.025f);

		// Removes small velocity collisions because too many small collisions cause too strong ripples.
		const float velRampMin = 1.0f;
		const float velRampMax = 10.0f;
		const float t = min(1.0f, max(0.0f, (v - velRampMin) / (velRampMax - velRampMin)));
		const float smoothstep = t * t * (3.0f - 2.0f * t);
		fMomentum *= smoothstep;

		if(gEnv->p3DEngine && fMomentum > 0.0f)
		{
			gEnv->p3DEngine->AddWaterRipple(pEventPhysArea->pt, 1.0f, fMomentum);
		}
	}

	return 1;
}

CWaterRippleUpr::CWaterRippleUpr()
{
}

CWaterRippleUpr::~CWaterRippleUpr()
{
	Finalize();
}

void CWaterRippleUpr::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_waterRippleInfos);
}

void CWaterRippleUpr::Initialize()
{
	if (gEnv->pPhysicalWorld)
	{
		gEnv->pPhysicalWorld->AddEventClient(EventPhysCollision::id, &OnEventPhysCollision, 1);
	}
}

void CWaterRippleUpr::Finalize()
{
	if (gEnv->pPhysicalWorld)
	{
		gEnv->pPhysicalWorld->RemoveEventClient(EventPhysCollision::id, &OnEventPhysCollision, 1);
	}
}

void CWaterRippleUpr::OnFrameStart()
{
	m_waterRippleInfos.clear();
}

void CWaterRippleUpr::Render(const SRenderingPassInfo& passInfo)
{
	auto* pRenderView = passInfo.GetIRenderView();

	if (pRenderView && passInfo.IsRecursivePass())
	{
		return;
	}

	const float simGridSize = static_cast<float>(SWaterRippleInfo::WaveSimulationGridSize);
	const float simGridRadius = 1.41421356f * 0.5f * simGridSize;
	const float looseSimGridRadius = simGridRadius * 1.5f;
	const float looseSimGridRadius2 = looseSimGridRadius * looseSimGridRadius;
	const Vec3 camPos3D = passInfo.GetCamera().GetPosition();
	const Vec2 camPos2D(camPos3D.x, camPos3D.y);

	for (auto& ripple : m_waterRippleInfos)
	{
		// add a ripple which is approximately in the water simulation grid.
		const Vec2 pos2D(ripple.position.x, ripple.position.y);
		if ((pos2D - camPos2D).GetLength2() <= looseSimGridRadius2)
		{
			pRenderView->AddWaterRipple(ripple);
		}
	}
}

void CWaterRippleUpr::AddWaterRipple(const Vec3 position, float scale, float strength)
{
	if (m_waterRippleInfos.size() < SWaterRippleInfo::MaxWaterRipplesInScene)
	{
		m_waterRippleInfos.emplace_back(position, scale, strength);
	}
}
