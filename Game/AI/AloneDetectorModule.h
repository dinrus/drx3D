// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:  ActorsPresenceAwareness.h
//  Version:     v1.00
//  Created:     30/01/2012 by Francesco Roccucci.
//  Описание: This is used for detecting the proximity of a specific 
//				set of AI agents to drive the selection of the behavior in
//				the tree without specific checks in the behavior itself
// -------------------------------------------------------------------------
//  История:
//	30/01/2012 12:00 - Added by Francesco Roccucci
//
////////////////////////////////////////////////////////////////////////////

#ifndef __AloneDetectorModule_h__
#define __AloneDetectorModule_h__

#pragma once

#include <drx3D/Game/GameAIHelpers.h>

class Agent;

class AloneDetectorContainer : public CGameAIInstanceBase
{
public:
	typedef u8 FactionID;

	struct AloneDetectorSetup
	{
		enum State
		{
			Alone,
			EntitiesInRange,
			Unkown,
		};

		AloneDetectorSetup(
			const float _rangeSq, 
			tukk _aloneSignal, 
			tukk _notAloneSignal
			)
			:range(_rangeSq)
			,aloneSignal(_aloneSignal)
			,notAloneSignal(_notAloneSignal)
			,state(Unkown)
		{
		}

		AloneDetectorSetup():state(Unkown), range(0.0f)
		{
		}

		float		range;
		string		aloneSignal;
		string		notAloneSignal;
		State		state;
	};

	void		Update(float frameTime);
	void		SetupDetector(const AloneDetectorSetup& setup);
	void		AddEntityClass(tukk const entityClassName);
	void		RemoveEntityClass(tukk const entityClassName);
	void		ResetDetector();
	bool		IsAlone() const;

private:
	float		GetSqDistanceFromLocation(const Vec3& location) const;
	void		SendCorrectSignal();

	tukk GetActorClassName(const Agent& agent) const;
	bool		IsActorValid(const Agent& agent) const;

	AloneDetectorSetup	m_setup;
	typedef std::vector<string> EntitiesList;
	EntitiesList		m_entityClassNames;
};

class AloneDetectorModule : public AIModuleWithInstanceUpdate<AloneDetectorModule, AloneDetectorContainer, 16, 8>
{
public:
	virtual tukk GetName() const { return "AloneDetector"; }
};

#endif // __AloneDetectorModule_h__