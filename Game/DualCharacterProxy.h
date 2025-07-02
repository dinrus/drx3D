// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Dual character proxy, to play animations on fp character and shadow character

-------------------------------------------------------------------------
История:
- 09-11-2009		Benito G.R. - Extracted from Player

*************************************************************************/

#pragma once

#ifndef _DUALCHARACTER_PROXY_H_
#define _DUALCHARACTER_PROXY_H_

#include <drx3D/Act/IAnimationGraph.h>

#if !defined(_RELEASE)

	#define PAIRFILE_GEN_ENABLED 1

#endif //!_RELEASE 

class CAnimationProxyDualCharacterBase : public CAnimationPlayerProxy
{
public:

	#if PAIRFILE_GEN_ENABLED
		static void Generate1P3PPairFile();
	#endif //PAIRFILE_GEN_ENABLED

	static void Load1P3PPairFile();
	static void ReleaseBuffers();

	CAnimationProxyDualCharacterBase();
	virtual bool StartAnimation(IEntity *entity, tukk szAnimName, const DrxCharAnimationParams& Params, float speedMultiplier = 1.0f);

	virtual void OnReload();

protected:

	struct SPlayParams
	{
		i32 animIDFP;
		i32 animIDTP;
		float speedFP;
		float speedTP;
	};

	void GetPlayParams(i32 animID, float speedMul, IAnimationSet *animSet, SPlayParams &params);

	static i32 Get3PAnimID(IAnimationSet *animSet, i32 animID);


	typedef std::map<u32, u32> NameHashMap;
	static NameHashMap s_animCrCHashMap;

	i32 m_characterMain;
	i32 m_characterShadow;
	bool m_firstPersonMode;
};

class CAnimationProxyDualCharacter : public CAnimationProxyDualCharacterBase
{
public:

	CAnimationProxyDualCharacter();

	virtual bool StartAnimationById(IEntity *entity, i32 animId, const DrxCharAnimationParams& Params, float speedMultiplier = 1.0f);
	virtual bool StopAnimationInLayer(IEntity *entity, i32 nLayer, f32 BlendOutTime);
	virtual bool RemoveAnimationInLayer(IEntity *entity, i32 nLayer, u32 token);
	virtual const CAnimation *GetAnimation(IEntity *entity, i32 layer);
	virtual CAnimation *GetAnimation(IEntity *entity, i32 layer, u32 token);

	virtual void OnReload();

	void SetFirstPerson(bool FP)
	{
		m_firstPersonMode = FP;
	}

	void SetKillMixInFirst(bool killMix)
	{
		m_killMixInFirst = killMix;
	}

	void SetCanMixUpperBody(bool canMix)
	{
		m_allowsMix = canMix;
	}

	bool CanMixUpperBody() const
	{
		return m_allowsMix;
	}

private:
	bool m_killMixInFirst;
	bool m_allowsMix;
};

class CAnimationProxyDualCharacterUpper : public CAnimationProxyDualCharacterBase
{
public:

	CAnimationProxyDualCharacterUpper();

	virtual bool StartAnimationById(IEntity *entity, i32 animId, const DrxCharAnimationParams& Params, float speedMultiplier = 1.0f);
	virtual bool StopAnimationInLayer(IEntity *entity, i32 nLayer, f32 BlendOutTime);
	virtual bool RemoveAnimationInLayer(IEntity *entity, i32 nLayer, u32 token);

	virtual void OnReload();

	void SetFirstPerson(bool FP)
	{
		m_firstPersonMode = FP;
	}

private:
	bool m_killMixInFirst;
};

#endif //_DUALCHARACTER_PROXY_H_