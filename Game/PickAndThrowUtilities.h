// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Utility classes for pick&throw mechanic

Created by Benito G.R., code refactored from PickAndThrow.cpp

*************************************************************************/

#pragma once

#ifndef _PICKANDTHROW_UTILITIES_H_
#define _PICKANDTHROW_UTILITIES_H_

struct IActor;
struct IStatObj;
#if !DRX_PLATFORM_LINUX && !DRX_PLATFORM_ANDROID && !DRX_PLATFORM_APPLE && !DRX_PLATFORM_ORBIS
struct IStatObj::SSubObject;
#endif
namespace PickAndThrow
{
	class CObstructionCheck
	{

	public:
		CObstructionCheck();
		~CObstructionCheck();

		void DoCheck( IActor* pOwnerActor, EntityId objectId);
		void IntersectionTestComplete(const QueuedIntersectionID& intersectionID, const IntersectionTestResult& result);

		void Reset();

		ILINE bool IsObstructed() const { return m_obstructed; }

	private:

		QueuedIntersectionID	m_queuedPrimitiveId;
		bool					m_obstructed;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	IStatObj::SSubObject* FindHelperObject( tukk pHelperName, const EntityId objectId, i32k slot );
	IStatObj::SSubObject* FindHelperObject_Basic( tukk pHelperName, const EntityId objectId, i32k slot );
	IStatObj::SSubObject* FindHelperObject_Extended( tukk pHelperName, const EntityId objectId, i32k slot );
	IStatObj::SSubObject* FindHelperObject_RecursivePart( IStatObj* pObj, tukk pHelperName );
	i32 FindActiveSlot( const EntityId objectId );

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool TargetEntityWithinFrontalCone(const Vec3& attackerLocation,const Vec3& victimLocation,const Vec3& attackerFacingdir, const float targetConeRads, float& theta);
	bool AllowedToTargetPlayer(const EntityId attackerId, const EntityId victimEntityId);

	ILINE float SelectAnimDurationOverride(const float cVarOverride, const float XMLOverride)
	{
		// This is the equivalent of the following more verbose version...
		/*

		float desiredDuration     =  cVarOverride;
		
		// If CVAR override not set.. we fall back to XML
		if(cVarOverride < 0.0f)
		{
			desiredDuration = XMLOverride;
		}

		return desiredDuration;

		*/

		return static_cast<float>(__fsel(cVarOverride, cVarOverride, XMLOverride));
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
}

#endif //_PICKANDTHROW_UTILITIES_H_
