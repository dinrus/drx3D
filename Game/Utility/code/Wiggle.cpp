// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Wiggle.h>
#include <drx3D/Game/Hermite.h>



CWiggle::CWiggle()
	:	m_frequency(1.0f)
	,	m_time(0.0f)
{
	m_points[0] = drx_random(-1.0f, 1.0f);
	m_points[1] = drx_random(-1.0f, 1.0f);
	m_points[2] = drx_random(-1.0f, 1.0f);
	m_points[3] = drx_random(-1.0f, 1.0f);
}



void CWiggle::SetParams(float frequency)
{
	m_frequency = frequency;
}



float CWiggle::Update(float deltaTime)
{
	m_time += deltaTime * m_frequency;
	while (m_time > 1.0f)
	{
		m_points[0] = m_points[1];
		m_points[1] = m_points[2];
		m_points[2] = m_points[3];
		m_points[3] = drx_random(-1.0f, 1.0f);
		m_time -= 1.0f;
	}

	return CatmullRom(
		m_points[0], m_points[1],
		m_points[2], m_points[3],
		m_time);
}





void CWiggleVec3::SetParams(float frequency)
{
	m_x.SetParams(frequency);
	m_y.SetParams(frequency);
	m_z.SetParams(frequency);
}



Vec3 CWiggleVec3::Update(float deltaTime)
{
	return Vec3(
		m_x.Update(deltaTime),
		m_y.Update(deltaTime),
		m_z.Update(deltaTime));
}
