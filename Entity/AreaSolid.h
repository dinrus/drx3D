// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/AreaUtil.h>

class CBSPTree3D;
class CSegmentSet;

class CAreaSolid final
{
public:

	enum ESegmentType
	{
		eSegmentType_Open,
		eSegmentType_Close,
	};

	enum ESegmentQueryFlag
	{
		eSegmentQueryFlag_Obstruction         = 0x0001,
		eSegmentQueryFlag_Open                = 0x0002,
		eSegmentQueryFlag_All                 = eSegmentQueryFlag_Obstruction | eSegmentQueryFlag_Open,

		eSegmentQueryFlag_UsingReverseSegment = 0x0010
	};

	CAreaSolid();
	~CAreaSolid()
	{
		Clear();
	}

	void AddRef()
	{
		DrxInterlockedIncrement(&m_nRefCount);
	}
	void Release()
	{
		if (DrxInterlockedDecrement(&m_nRefCount) <= 0)
			delete this;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	bool QueryNearest(const Vec3& vPos, i32 queryFlag, Vec3& outNearestPos, float& outNearestDistance) const;
	bool IsInside(const Vec3& vPos) const;
	void Draw(const Matrix34& worldTM, const ColorB& color0, const ColorB& color1) const;

	/////////////////////////////////////////////////////////////////////////////////////////////
	void AddSegment(const Vec3* verticesOfConvexhull, bool bObstruction, i32 numberOfPoints);
	void BuildBSP();

	////////////////////////////////////////////////////////////////////////////////////////////
	const AABB& GetBoundBox()
	{
		return m_BoundBox;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const;

private:

	void        Clear();

	static bool IsQueryTypeIdenticalToSegmentType(i32 queryFlag, ESegmentType segmentType)
	{
		if ((queryFlag & eSegmentQueryFlag_All) != eSegmentQueryFlag_All)
		{
			if ((queryFlag & eSegmentQueryFlag_Obstruction) && segmentType != eSegmentType_Close)
				return false;
			if ((queryFlag & eSegmentQueryFlag_Open) && segmentType != eSegmentType_Open)
				return false;
		}
		return true;
	}

private:

	CBSPTree3D*               m_BSPTree;
	std::vector<CSegmentSet*> m_SegmentSets;
	AABB                      m_BoundBox;
	i32                       m_nRefCount;
};
