// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/BreezeGenerator.h>

namespace
{
static inline Vec3 RandomPosition(const Vec3& centre, const Vec3& wind_speed, float life_time, float radius, float extents)
{
	Vec3 result;
	result.x = centre.x - (wind_speed.x * life_time * 0.5f) + drx_random(-radius, radius);
	result.y = centre.y - (wind_speed.y * life_time * 0.5f) + drx_random(-radius, radius);
	result.z = centre.z - (wind_speed.z * life_time * 0.5f) + drx_random(-radius, radius);
	return result;
}

static inline Vec3 RandomDirection(const Vec3& dir, float spread)
{
	return Matrix33::CreateRotationZ(DEG2RAD(drx_random(-spread, spread))) * dir;
}
}

struct SBreeze
{
	IPhysicalEntity* wind_area;
	Vec3             position;
	Vec3             direction;
	float            lifetime;

	SBreeze()
		: wind_area()
		, position()
		, direction()
		, lifetime(-1.f)
	{}
};

CBreezeGenerator::CBreezeGenerator()
	: m_breezes(nullptr)
	, m_spawn_radius(0.0f)
	, m_spread(0.0f)
	, m_count(0)
	, m_radius(0.0f)
	, m_lifetime(0.0f)
	, m_variance(0.0f)
	, m_strength(0.0f)
	, m_movement_speed(0.0f)
	, m_wind_speed(ZERO)
	, m_fixed_height(0.0f)
	, m_awake_thresh(0.0f)
	, m_enabled(false)
{

}

CBreezeGenerator::~CBreezeGenerator()
{
	Shutdown();
}

void CBreezeGenerator::Initialize()
{
	if (m_enabled && m_count)
	{
		m_breezes = new SBreeze[m_count];
		for (u32 i = 0; i < m_count; ++i)
		{
			primitives::box geomBox;
			geomBox.Basis.SetIdentity();
			geomBox.center = Vec3(0, 0, 0);
			geomBox.size = Vec3(
			  drx_random(m_radius, m_radius + m_variance),
			  drx_random(m_radius, m_radius + m_variance),
			  drx_random(m_radius, m_radius + m_variance));
			geomBox.bOriented = 0;
			IGeometry* pGeom = gEnv->pPhysicalWorld->GetGeomUpr()->CreatePrimitive(primitives::box::type, &geomBox);
			m_breezes[i].wind_area = gEnv->pPhysicalWorld->AddArea(pGeom, Vec3(0, 0, 0), Quat::CreateIdentity(), 1.f);

			pe_params_buoyancy buoyancy;
			buoyancy.waterDensity = 1.f;
			buoyancy.waterResistance = 1.f;
			buoyancy.waterFlow = m_wind_speed * drx_random(0.0f, m_strength);
			buoyancy.iMedium = 1;
			buoyancy.waterPlane.origin.Set(0, 0, 1e6f);
			buoyancy.waterPlane.n.Set(0, 0, -1);
			m_breezes[i].wind_area->SetParams(&buoyancy);

			pe_params_area area;
			area.bUniform = 1;
			area.falloff0 = 0.80f;
			m_breezes[i].wind_area->SetParams(&area);

			pe_params_foreign_data fd;
			fd.iForeignFlags = PFF_OUTDOOR_AREA;
			m_breezes[i].wind_area->SetParams(&fd);
		}
	}
}

void CBreezeGenerator::Reset()
{
	Shutdown();
	Initialize();
}

void CBreezeGenerator::Shutdown()
{
	if (m_enabled)
		for (u32 i = 0; i < m_count; ++i)
			gEnv->pPhysicalWorld->DestroyPhysicalEntity(m_breezes[i].wind_area);

	delete[] m_breezes;
	m_breezes = NULL;
	m_count = 0;
}

void CBreezeGenerator::Update()
{
	DRX_PROFILE_FUNCTION(PROFILE_3DENGINE);
	if (!m_enabled)
		return;

	pe_params_buoyancy buoyancy;
	buoyancy.waterDensity = 1.f;
	buoyancy.waterResistance = 1.f;
	buoyancy.iMedium = 1;

	pe_params_pos pp;
	const float frame_time = GetTimer()->GetFrameTime();
	const Vec3 centre = Get3DEngine()->GetRenderingCamera().GetPosition();
	for (u32 i = 0; i < m_count; ++i)
	{
		SBreeze& breeze = m_breezes[i];
		breeze.lifetime -= frame_time;
		if (breeze.lifetime < 0.f)
		{
			Vec3 pos = RandomPosition(centre, m_wind_speed, m_lifetime, m_spawn_radius, m_radius);
			if (m_fixed_height != -1.f)
				pos.z = m_fixed_height;
			else
				pos.z = Get3DEngine()->GetTerrainElevation(pos.x, pos.y) - m_radius * 0.5f;
			breeze.position = pp.pos = pos;
			buoyancy.waterFlow = breeze.direction = RandomDirection(m_wind_speed, m_spread) * drx_random(0.0f, m_strength);
			breeze.wind_area->SetParams(&pp);
			breeze.wind_area->SetParams(&buoyancy);
			breeze.lifetime = m_lifetime * drx_random(0.5f, 1.0f);
		}
		else
		{
			pp.pos = (breeze.position += breeze.direction * m_movement_speed * frame_time);
			if (m_fixed_height != -1.f)
				pp.pos.z = m_fixed_height;
			else
				pp.pos.z = Get3DEngine()->GetTerrainElevation(pp.pos.x, pp.pos.y) - m_radius * 0.5f;
			breeze.wind_area->SetParams(&pp);

			if (m_awake_thresh)
			{
				pe_params_bbox pbb;
				breeze.wind_area->GetParams(&pbb);
				Vec3 sz = (pbb.BBox[1] - pbb.BBox[0]) * 0.5f;
				float kv = m_strength * buoyancy.waterResistance;
				IPhysicalEntity* pentBuf[128], ** pents = pentBuf;
				pe_params_part ppart;
				pe_action_awake aa;
				for (i32 j = gEnv->pPhysicalWorld->GetEntitiesInBox(pp.pos - sz, pp.pos + sz, pents, ent_sleeping_rigid | ent_allocate_list, 128) - 1; j >= 0; j--)
					for (ppart.ipart = 0; pents[j]->GetParams(&ppart); ppart.ipart++)
						if (sqr(ppart.pPhysGeom->V * cube(ppart.scale)) * cube(kv) > cube(ppart.mass * m_awake_thresh))
						{
							pents[j]->Action(&aa);
							break;
						}
				if (pents != pentBuf)
					gEnv->pPhysicalWorld->GetPhysUtils()->DeletePointer(pents);
			}
		}
	}
}
