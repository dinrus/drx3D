// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _POLYGON_MATH_2D_
#define _POLYGON_MATH_2D_
#pragma once

#include <drx3D/CoreX/Containers/DrxFixedArray.h>

//==================================================================================================
// Name: FixedPodArray
// Desc: Extension of DrxFixedArray to add useful functionality
// Note: This should be promoted into DrxFixedArray in the long term
// Author: Chris Bunner
//==================================================================================================
template<class T, u32 N>
class FixedPodArray : public DrxFixedArray<T, N>
{
	typedef DrxFixedArray<T, N> CFA;

public:
	ILINE void resize(const uint size)
	{
		if (size <= N)
		{
			CFA::m_curSize[0] = size;
		}
		else
		{
			DrxLogAlways("FixedPodArray::resize() failing as size too large - NOT resizing array");
			DRX_ASSERT_MESSAGE(0, "FixedPodArray::resize() failing as size too large - NOT resizing array");
		}
	}

	ILINE void erase(const uint index, const uint count = 1)
	{
		const uint size = CFA::m_curSize[0];

		if (index == size - count)
		{
			// Simple remove
			CFA::m_curSize[0] -= count;
		}
		else if (index + count < size)
		{
			// Shuffle set of elements along
			const uint range = size - (index + count);
			memmove(&CFA::at(index), &CFA::at(index + count), sizeof(T) * range);

			CFA::m_curSize[0] -= count;
		}
		else
		{
			DrxLogAlways("FixedPodArray::erase() failing as element range invalid - NOT removing element");
			DRX_ASSERT_MESSAGE(0, "FixedPodArray::erase() failing as element range invalid - NOT removing element");
		}
	}

	ILINE void insert_before(const T& elem, const uint index)
	{
		const uint size = CFA::m_curSize[0];

		if (index < size && size < N)
		{
			// Shuffle set of elements along
			const uint count = size - index;
			memmove(&CFA::at(index + 1), &CFA::at(index), sizeof(T) * count);

			// Add new element
			memcpy(&CFA::at(index), &elem, sizeof(T));
			++CFA::m_curSize[0];
		}
		else
		{
			DrxLogAlways("FixedPodArray::insert_before() failing as element index invalid - NOT inserting element");
			DRX_ASSERT_MESSAGE(0, "FixedPodArray::insert_before() failing as element index invalid - NOT inserting element");
		}
	}

	ILINE i32 find(const T& elem)
	{
		const uint size = CFA::m_curSize[0];
		i32k invalid = -1;

		i32 index = invalid;

		for (uint i = 0; i < size; ++i)
		{
			if (CFA::at(i) == elem)
			{
				index = i;
				break;
			}
		}

		return index;
	}
};

//==================================================================================================
// Name: PolygonMath2D
// Desc: Polygon math helper functions optimized for breakable glass sim
// Note: Functions often assume clockwise polygon point order
// Author: Chris Bunner
//==================================================================================================

// Common polygon array types
#define POLY_ARRAY_SIZE 45

typedef FixedPodArray<Vec2, POLY_ARRAY_SIZE> TPolygonArray;
typedef FixedPodArray<i32, POLY_ARRAY_SIZE>  TPolyIndexArray;

// 2D triangle helpers
bool PointInTriangle2D(const Vec2& pt, const Vec2& a, const Vec2& b, const Vec2& c);
bool TriangleIsConvex2D(const Vec2& a, const Vec2& b, const Vec2& c);

// 2D intersection helpers
i32 LineCircleIntersect2D(const Vec2& pt0, const Vec2& pt1, const Vec2& center, const float radiusSq, float& intersectA, float& intersectB);

// 2D polygon helpers
bool  PointInPolygon2D(const Vec2& pt, const Vec2* pPolygon, i32k numPts);

float CalculatePolygonArea2D(const Vec2* pPolygon, i32k numPts);
Vec2  CalculatePolygonBounds2D(const Vec2* pPolygon, i32k numPts);
Vec2  CalculatePolygonCenter2D(const Vec2* pPolygon, i32k numPts);
bool  CalculatePolygonSharedPerimeter2D(const Vec2* pPolygonA, i32k numPtsA, const Vec2* pPolygonB, i32k numPtsB, float& connectionA, float& connectionB);

void  TriangulatePolygon2D(const Vec2* pPolygon, i32k numPolyPts, PodArray<Vec2>* pVertices, PodArray<u8>* pIndices);
void  SplitPolygonAroundPoint(TPolygonArray& outline, const Vec2& splitPt, const float splitRadius, TPolygonArray& splitInnerOutline);
void  SplitPolygonAroundPoint(PodArray<Vec2>& outline, const Vec2& splitPt, const float splitRadius, TPolygonArray& splitInnerOutline);

// Helper with result enum
enum EPolygonInCircle2D
{
	EPolygonInCircle2D_Inside = 0,
	EPolygonInCircle2D_Outside,
	EPolygonInCircle2D_Overlap
};

EPolygonInCircle2D PolygonInCircle2D(const Vec2& center, const float radius, const Vec2* pPolygon, i32k numPts);

#endif // _POLYGON_MATH_2D_
