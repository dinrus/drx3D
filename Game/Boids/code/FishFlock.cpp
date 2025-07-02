// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   fishflock.cpp
//  Version:     v1.00
//  Created:     8/2010 by Luciano Morpurgo (refactored from flock.cpp)
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual C++ 7.0
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/FishFlock.h>
#include <drx3D/Game/BoidFish.h>

//////////////////////////////////////////////////////////////////////////

void CFishFlock::CreateBoids( SBoidsCreateContext &ctx )
{
	CFlock::CreateBoids(ctx);

	for (u32 i = 0; i < m_RequestedBoidsCount; i++)
	{
		CBoidFish *boid = new CBoidFish( m_bc );
		float radius = m_bc.fSpawnRadius;
		boid->m_pos = m_origin + Vec3(radius*Boid::Frand(),radius*Boid::Frand(),Boid::Frand()*radius);

		if (m_bc.waterLevel != WATER_LEVEL_UNKNOWN && boid->m_pos.z > m_bc.waterLevel)
			boid->m_pos.z = m_bc.waterLevel-1;
		else
		{
			float terrainZ = m_bc.engine->GetTerrainElevation(boid->m_pos.x,boid->m_pos.y);
			if (boid->m_pos.z <= terrainZ)
				boid->m_pos.z = terrainZ + 0.01f;
		}

		boid->m_speed = m_bc.MinSpeed + (Boid::Frand()+1)/2.0f*(m_bc.MaxSpeed - m_bc.MinSpeed);
		boid->m_heading = ( Vec3(Boid::Frand(),Boid::Frand(),0) ).GetNormalized();
		boid->m_scale = m_bc.boidScale + Boid::Frand()*m_bc.boidRandomScale;

		AddBoid(boid);
	}
}

