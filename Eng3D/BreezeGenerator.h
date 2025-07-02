// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _BREEZEGENERATOR_H
#define _BREEZEGENERATOR_H

#include <drx3D/Eng3D/DinrusX3dEngBase.h>

struct SBreeze;

// Spawns wind volumes around the camera to emulate breezes
class CBreezeGenerator : public DinrusX3dEngBase
{
	friend class C3DEngine;

	// The array of active breezes
	SBreeze* m_breezes;

	// The radius around the camera where the breezes will be spawned
	float m_spawn_radius;

	// The spread (variation in direction on spawn)
	float m_spread;

	// The max. number of wind areas active at the same time
	u32 m_count;

	// The max. extents of each breeze
	float m_radius;

	// The max. life of each breeze
	float m_lifetime;

	// The random variance of each breeze in respect to it's other attributes
	float m_variance;

	// The strength of the breeze (as a factor of the original wind vector)
	float m_strength;

	// The speed of the breeze movement (not coupled to the wind speed)
	float m_movement_speed;

	// The global direction of the environment wind
	Vec3 m_wind_speed;

	// Set a fixed height for the breeze, for levels without terrain. -1 uses the terrain height
	float m_fixed_height;

	// Approximate threshold velocity that the wind can add to an entity part per second that will awake it (0 disables)
	float m_awake_thresh;

	// breeze generation enabled?
	bool m_enabled;

public:

	CBreezeGenerator();
	~CBreezeGenerator();

	void Initialize();

	void Reset();

	void Shutdown();

	void Update();
};

#endif
