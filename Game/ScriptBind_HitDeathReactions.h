// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание: 

-------------------------------------------------------------------------
История:
- 13:11:2009	16:25 : Created by David Ramos
*************************************************************************/
#ifndef __SCRIPT_BIND_HIT_DEATH_REACTIONS_H
#define __SCRIPT_BIND_HIT_DEATH_REACTIONS_H

#include <drx3D/Game/HitDeathReactionsDefs.h>

class CPlayer;
struct HitInfo;

class CScriptBind_HitDeathReactions : public CScriptableBase
{
public:
	CScriptBind_HitDeathReactions(ISystem *pSystem, IGameFramework *pGameFramework);
	virtual ~CScriptBind_HitDeathReactions();

	//! <description>Notifies a hit event to the hit death reactions system</description>
	//! 	<param name="scriptHitInfo = script table with the hit info</param>
	//! <returns>TRUE if the hit is processed successfully, FALSE otherwise</returns>
	i32										OnHit(IFunctionHandler *pH, SmartScriptTable scriptHitInfo);


	//! <description>Executes a hit reaction using the default C++ execution code</description>
	//! 	<param name="reactionParams = script table with the reaction parameters</param>
	i32										ExecuteHitReaction (IFunctionHandler *pH, SmartScriptTable reactionParams);

	//! <description>Executes a death reaction using the default C++ execution code</description>
	//! 	<param name="reactionParams = script table with the reaction parameters</param>
	i32										ExecuteDeathReaction (IFunctionHandler *pH, SmartScriptTable reactionParams);

	//! <returns>Ends the current reaction</returns>
	i32										EndCurrentReaction (IFunctionHandler *pH);

	//! <description>Run the default C++ validation code and returns its result</description>
	//! 	<param name="validationParams = script table with the validation parameters</param>
	//! 	<param name="scriptHitInfo = script table with the hit info</param>
	//! <returns>TRUE is the validation was successful, FALSE otherwise</returns>
	i32										IsValidReaction (IFunctionHandler *pH, SmartScriptTable validationParams, SmartScriptTable scriptHitInfo);

	//! <description>Starts an animation through the HitDeathReactions. Pauses the animation graph while playing it
	//! 		and resumes automatically when the animation ends</description>
	//! 	<param name="sAnimName</param>
	//! 	<param name="bLoop">false</param>
	//! 	<param name="fBlendTime">0.2f</param>
	//! 	<param name="iSlot">0</param>
	//! 	<param name="iLayer">0</param>
	//! 	<param name="fAniSpeed">1.0f</param>
	i32										StartReactionAnim(IFunctionHandler *pH);

	//! <code>EndReactionAnim</code>
	//! <description>Ends the current reaction anim, if any</description>
	i32										EndReactionAnim(IFunctionHandler *pH);

	//! <description>Starts an interactive action.</description>
	//! 	<param name="szActionName">name of the interactive action</param>
	i32										StartInteractiveAction(IFunctionHandler *pH, tukk szActionName);

	virtual void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

private:
	CPlayer*							GetAssociatedActor(IFunctionHandler *pH) const;
	CHitDeathReactionsPtr GetHitDeathReactions(IFunctionHandler *pH) const;

	SmartScriptTable	m_pParams;

	ISystem*					m_pSystem;
	IGameFramework*		m_pGameFW;
};

#endif // __SCRIPT_BIND_HIT_DEATH_REACTIONS_H
