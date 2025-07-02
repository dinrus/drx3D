// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/BloodSplats.h>

void CBloodSplats::Init(i32 type, float maxTime)
{
	m_type = type;
	m_maxTime = maxTime;
}

bool CBloodSplats::Update(float delta)
{
	m_currentTime -= delta;

	if (m_currentTime < 0.0f)
		return true;

	float scale = (2.0f * m_currentTime) / m_maxTime;
	if (scale > 1.0f) scale = 1.0f;

	gEnv->p3DEngine->SetPostEffectParam("BloodSplats_Amount", scale * scale);

	return false;
}

bool CBloodSplats::OnActivate()
{
	gEnv->p3DEngine->SetPostEffectParam("BloodSplats_Type", (float)m_type);
	gEnv->p3DEngine->SetPostEffectParam("BloodSplats_Active", 1.0f);
	gEnv->p3DEngine->SetPostEffectParam("BloodSplats_Amount", 1.0f);
	gEnv->p3DEngine->SetPostEffectParam("BloodSplats_Spawn", 1.0f);
	m_currentTime = m_maxTime;
	return true;
}

bool CBloodSplats::OnDeactivate()
{
	gEnv->p3DEngine->SetPostEffectParam("BloodSplats_Active", 0.0f);
	return true;
}

void CBloodSplats::GetMemoryUsage(IDrxSizer* s) const
{
	s->Add(*this);
}
