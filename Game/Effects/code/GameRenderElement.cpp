// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// Base class for all game render elements.

// Includes
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameRenderElement.h>

//--------------------------------------------------------------------------------------------------
// Name: CGameRenderElement
// Desc: Constructor
//--------------------------------------------------------------------------------------------------
CGameRenderElement::CGameRenderElement()
{
	m_pREGameEffect = NULL;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: InitialiseGameRenderElement
// Desc: Initialises game render element
//--------------------------------------------------------------------------------------------------
bool CGameRenderElement::InitialiseGameRenderElement()
{
	m_pREGameEffect	= (CREGameEffect*)gEnv->pRenderer->EF_CreateRE(eDATA_GameEffect);
	if(m_pREGameEffect)
	{
		m_pREGameEffect->SetPrivateImplementation(this);
		m_pREGameEffect->mfUpdateFlags(FCEF_TRANSFORM);
	}

	return true;
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: ReleaseGameRenderElement
// Desc: Releases game render element
//--------------------------------------------------------------------------------------------------
void CGameRenderElement::ReleaseGameRenderElement()
{
	if(m_pREGameEffect)
	{
		m_pREGameEffect->SetPrivateImplementation(NULL);
		m_pREGameEffect->Release(false);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: UpdatePrivateImplementation
// Desc: Updates private implementation
//--------------------------------------------------------------------------------------------------
void CGameRenderElement::UpdatePrivateImplementation()
{
	if(m_pREGameEffect)
	{
		m_pREGameEffect->SetPrivateImplementation(this);
	}
}//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Name: GetCREGameEffect
// Desc: returns the game effect render element
//--------------------------------------------------------------------------------------------------
CREGameEffect* CGameRenderElement::GetCREGameEffect()
{
	return m_pREGameEffect;
}//-------------------------------------------------------------------------------------------------
