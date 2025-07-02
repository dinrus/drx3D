// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  network debugging helpers for DinrusAction code
   -------------------------------------------------------------------------
   История:
   - 01/06/2000   11:28 : Created by Sergii Rustamov
*************************************************************************/
#ifndef __NETDEBUG_H__
#define __NETDEBUG_H__

#pragma once

#if ENABLE_NETDEBUG

class CNetDebug
{
public:
	CNetDebug();
	~CNetDebug();

	void DebugAspectsChange(IEntity* pEntity, u8 aspects);
	void DebugRMI(tukk szDescription, size_t nSize);
	void Update();

private:
	static i32k MAX_ASPECTS = 6;
	static i32k MAX_TTL = 600;
	static i32k UPDATE_DIFF_MSEC = 1000;

	struct DebugAspectsEntity
	{
		string sEntityName;
		u32 aspectChangeCount[MAX_ASPECTS];
		float  aspectChangeRate[MAX_ASPECTS];
		u32 aspectLastCount[MAX_ASPECTS];
		i32    entityTTL[MAX_ASPECTS];

		DebugAspectsEntity()
		{
			memset(&aspectChangeCount, 0, sizeof(aspectChangeCount));
			memset(&aspectChangeRate, 0, sizeof(aspectChangeRate));
			memset(&aspectLastCount, 0, sizeof(aspectLastCount));
			memset(&entityTTL, 0, sizeof(entityTTL));
		}
	};
	typedef std::map<string, DebugAspectsEntity> TDebugEntities;
	struct DebugAspectsContext
	{
		string         sEntityClass;
		TDebugEntities entities;
	};
	typedef std::map<string, DebugAspectsContext> TDebugAspects;

private:
	struct DebugRMIEntity
	{
		string sDesc;
		u32 invokeCount;
		size_t totalSize;
		size_t lastTotalSize;
		float  rate;
		float  peakRate;

		DebugRMIEntity() : invokeCount(0), totalSize(0), lastTotalSize(0), rate(0), peakRate(0) {}
	};
	typedef std::map<string, DebugRMIEntity> TDebugRMI;

private:
	float         m_fLastTime;
	TDebugAspects m_aspects;
	TDebugRMI     m_rmi;
	i32           m_varDebugRMI;
	i32           m_varDebug;
	i32           m_trapValue;
	i32           m_trapValueCount;

private:
	void        UpdateAspectsDebugEntity(const DebugAspectsContext& ctx, DebugAspectsEntity& ent, u8 aspects);
	void        UpdateAspectsRates(float fDiffTimeMsec);
	void        DrawAspects();
	tukk IdxToAspectString(i32 idx);
	i32         AspectToIdx(EEntityAspects aspect);
	bool        ProcessFilter(tukk szClass, tukk szEntity);
	void        ProcessTrap(DebugAspectsEntity& ent);

private:
	void UpdateRMI(float fDiffTimeMSec);
	void DrawRMI();
};

#endif // ENABLE_NETDEBUG
#endif // __NETDEBUG_H__
