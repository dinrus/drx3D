// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// Includes
#include <drx3D/Render/StdAfx.h>
#include <drx3D/CoreX/Renderer/RendElements/CREGameEffect.h>
#include <drx3D/Render/D3D/DriverD3D.h>

//--------------------------------------------------------------------------------------------------
// Name: CREGameEffect
// Desc: Constructor
//--------------------------------------------------------------------------------------------------
CREGameEffect::CREGameEffect()
{
	m_pImpl = NULL;
	mfSetType(eDATA_GameEffect);
	mfUpdateFlags(FCEF_TRANSFORM);
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: ~CREGameEffect
// Desc: Destructor
//--------------------------------------------------------------------------------------------------
CREGameEffect::~CREGameEffect()
{
	SAFE_DELETE(m_pImpl);
}//-------------------------------------------------------------------------------------------------
