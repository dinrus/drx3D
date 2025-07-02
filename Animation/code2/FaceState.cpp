// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/FaceState.h>

//////////////////////////////////////////////////////////////////////////
void CFaceState::SetNumWeights(i32 n)
{
	m_weights.resize(n);
	m_balance.resize(n);
	Reset();
}

//////////////////////////////////////////////////////////////////////////
void CFaceState::Reset()
{
	if (!m_weights.empty())
		memset(&m_weights[0], 0, sizeof(float) * m_weights.size());
	if (!m_balance.empty())
		memset(&m_balance[0], 0, sizeof(float) * m_balance.size());
}
