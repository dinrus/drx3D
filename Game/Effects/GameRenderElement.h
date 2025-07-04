// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _IGAME_RENDERELEMENT_
#define _IGAME_RENDERELEMENT_

#pragma once

// Includes
#include <drx3D/Game/Effects/GameEffectsSystem.h>
#include <drx3D/CoreX/Renderer/RendElements/CREGameEffect.h>

// Forward declares
struct IGameRenderElementParams;

//==================================================================================================
// Name: IGameRenderElement
// Desc: Base interface for all game render elements
// Author: James Chilvers
//==================================================================================================
struct IGameRenderElement : public IREGameEffect, public _i_reference_target_t
{
	virtual ~IGameRenderElement() {}

	virtual bool InitialiseGameRenderElement() = 0;
	virtual void ReleaseGameRenderElement() = 0;
	virtual void UpdatePrivateImplementation() = 0;

	virtual CREGameEffect*	GetCREGameEffect() = 0;

	virtual IGameRenderElementParams* GetParams() = 0;
};//------------------------------------------------------------------------------------------------

//==================================================================================================
// Name: CGameRenderElement
// Desc: Base class for all game render elements
// Author: James Chilvers
//==================================================================================================
class CGameRenderElement : public IGameRenderElement
{
public:
	CGameRenderElement();
	virtual ~CGameRenderElement() {}

	virtual bool InitialiseGameRenderElement();
	virtual void ReleaseGameRenderElement();
	virtual void UpdatePrivateImplementation();

	virtual CREGameEffect*	GetCREGameEffect();

protected:
	CREGameEffect*		m_pREGameEffect;
};//------------------------------------------------------------------------------------------------

//==================================================================================================
// Name: IGameRenderElementParams
// Desc: Game Render Element params
// Author: James Chilvers
//==================================================================================================
struct IGameRenderElementParams
{
	virtual ~IGameRenderElementParams() {}
};//------------------------------------------------------------------------------------------------

#endif // _IGAME_RENDERELEMENT_
