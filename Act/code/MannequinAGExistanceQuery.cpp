// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/MannequinAGExistanceQuery.h>

// ============================================================================
// ============================================================================

namespace MannequinAG
{

// ============================================================================
// ============================================================================

//////////////////////////////////////////////////////////////////////////
CMannequinAGExistanceQuery::CMannequinAGExistanceQuery(IAnimationGraphState* pAnimationGraphState)
	: m_pAnimationGraphState(pAnimationGraphState)
{
	DRX_ASSERT(m_pAnimationGraphState != NULL);
}

//////////////////////////////////////////////////////////////////////////
CMannequinAGExistanceQuery::~CMannequinAGExistanceQuery()
{
}

//////////////////////////////////////////////////////////////////////////
IAnimationGraphState* CMannequinAGExistanceQuery::GetState()
{
	return m_pAnimationGraphState;
}

//////////////////////////////////////////////////////////////////////////
void CMannequinAGExistanceQuery::SetInput(InputID, float)
{
}

//////////////////////////////////////////////////////////////////////////
void CMannequinAGExistanceQuery::SetInput(InputID, i32)
{
}

//////////////////////////////////////////////////////////////////////////
void CMannequinAGExistanceQuery::SetInput(InputID, tukk )
{
}

//////////////////////////////////////////////////////////////////////////
bool CMannequinAGExistanceQuery::Complete()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
CTimeValue CMannequinAGExistanceQuery::GetAnimationLength() const
{
	return CTimeValue();
}

//////////////////////////////////////////////////////////////////////////
void CMannequinAGExistanceQuery::Reset()
{
}

// ============================================================================
// ============================================================================

} // MannequinAG
