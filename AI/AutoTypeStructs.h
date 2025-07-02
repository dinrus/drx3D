// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef AUTOTYPESTRUCT_H
#define AUTOTYPESTRUCT_H

#if _MSC_VER > 1000
	#pragma once
#endif

/// All structures that use auto-type info should go in here - a single header
/// that can get #included by AutoTypeInfo.cpp. This file should not include
/// any other private headers, but interface headers are OK

#include <drx3D/AI/IAISystem.h>
#include <drx3D/AI/IAgent.h>

//====================================================================
// SLinkRecord
// used in CWaypoint3DSurfaceNavRegion
//====================================================================
struct SLinkRecord
{
	SLinkRecord(i32 nodeIndex = 0, float radiusOut = 0, float radiusIn = 0) : nodeIndex(nodeIndex), radiusOut(radiusOut), radiusIn(radiusIn) {}
	i32   nodeIndex;
	float radiusOut, radiusIn;
	AUTO_STRUCT_INFO;
};

struct SFlightLinkDesc
{
	SFlightLinkDesc(i32 i1, i32 i2) : index1(i1), index2(i2) {}
	i32 index1, index2;
	AUTO_STRUCT_INFO;
};

//====================================================================
// NodeDescriptor - used for saving/loading
//====================================================================
struct NodeDescriptor
{
	// the node ID used during quick load/save. Guaranteed to be unique (by resetting just before saving)
	// and ID 1 is special = it signifies m_pSafeFirst
	unsigned ID;
	Vec3     dir;
	Vec3     up;
	Vec3     pos;
	i32      index; // building, span or volume
	i32      obstacle[3];

	u16   navType; // cast from IAISystem::ENavigationType

	u8    type               : 4; // 0-4 from EWaypointNodeType
	u8    bForbidden         : 1; // bool - just for triangular and forbidden areas
	u8    bForbiddenDesigner : 1; // bool - just for triangular and designer forbidden
	u8    bRemovable         : 1; // bool

	AUTO_STRUCT_INFO;

	NodeDescriptor() : ID(0), navType(0), bRemovable(0), bForbidden(0), type(0), dir(ZERO), up(ZERO), pos(ZERO), index(0) { obstacle[0] = obstacle[1] = obstacle[2] = 0; }
};

//====================================================================
// LinkDescriptor - used for saving/loading
//====================================================================
struct LinkDescriptor
{
	unsigned nSourceNode;
	unsigned nTargetNode;
	Vec3     vEdgeCenter;
	float    fMaxPassRadius;
	float    fExposure;
	float    fLength;
	float    fMaxWaterDepth;
	float    fMinWaterDepth;
	u8    nStartIndex, nEndIndex;
	u8    bIsPureTriangularLink   : 1;
	u8    bSimplePassabilityCheck : 1;
	AUTO_STRUCT_INFO;
};

struct SVolumeHideSpot
{
	SVolumeHideSpot(const Vec3& pos = Vec3(0, 0, 0), const Vec3& dir = Vec3(0, 0, 0)) : pos(pos), dir(dir) {}
	const Vec3& GetPos() const                          { return pos; }
	void        GetMemoryUsage(IDrxSizer* pSizer) const {}
	Vec3        pos;
	Vec3        dir;
	AUTO_STRUCT_INFO;
};

struct ObstacleDataDesc
{
	Vec3 vPos;
	Vec3 vDir;
	/// this radius is approximate - it is estimated during link generation. if -ve it means
	/// that it shouldn't be used (i.e. object is significantly non-circular)
	float fApproxRadius;
	u8 flags;
	u8 approxHeight;

	void  Serialize(TSerialize ser);

	AUTO_STRUCT_INFO;
};

struct SFlightDesc
{
	float x, y;
	float minz, maxz;
	float maxRadius;
	i32   classification;
	i32   childIdx;
	i32   nextIdx;
	AUTO_STRUCT_INFO;
};

#endif
