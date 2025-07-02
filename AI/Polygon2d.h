// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef POLYGON2D_H
#define POLYGON2D_H

#include <drx3D/AI/Utils.h>

#include <drx3D/AI/LineSeg.h>
#include <drx3D/AI/BiDirMap.h>

class BspLineSegSplitter;
class BspTree2d;

/**
 * @brief 2D polygon class with support for set operations and subsequent contour extraction.
 */
class Polygon2d
{
public:

	/**
	 * Not really a stand-alone polygon edge class, vertices are only stored as
	 * indices into the owning Polygon2d::m_vertices.
	 */
	class Edge
	{
	public:
		Edge(i32 ind0 = -1, i32 ind1 = -1);

		bool operator<(const Edge& rhs) const;
		bool operator==(const Edge& rhs) const;

		i32 m_vertIndex0;
		i32 m_vertIndex1;
	};

	Polygon2d();
	Polygon2d(const LineSegVec& edges);
	Polygon2d(const Polygon2d& rhs);
	~Polygon2d();

	Polygon2d& operator=(const Polygon2d& rhs);

	i32        AddVertex(const Vector2d&);
	i32        AddEdge(const Edge&);
	i32        AddEdge(const Vector2d& v0, const Vector2d& v1);

	bool       GetVertex(i32 i, Vector2d& vertex) const;
	bool       GetEdge(i32 i, Edge& edge) const;

	i32        NumVertices() const;
	i32        NumEdges() const;

	// calculates the contiguous pts from the edges. If removeInterior then any contours that are encircled
	// by another contour get removed. All are guaranteed to be anti-clockwise wound
	void     CalculateContours(bool removeInterior);
	// returns the number of unique contours
	unsigned GetContourQuantity() const;
	// gets the ith contour - returns the number of points
	unsigned GetContour(unsigned i, const Vector2d** ppPts) const;

	/// Set inversion operator.
	Polygon2d operator~() const;

	/// Set union operator.
	Polygon2d operator|(const Polygon2d& rhs) const;

	/// Set intersection operator.
	Polygon2d operator&(const Polygon2d& rhs) const;

	/// Symmetric difference (AKA xor) operator.
	Polygon2d operator^(const Polygon2d& rhs) const;

	/// Complement (or set difference) operator.
	Polygon2d operator-(const Polygon2d& rhs) const;

	// this only makes sense if there's only one contour
	bool IsWoundAnticlockwise() const;

	void CollapseVertices(double tol);

private:

	/// Computes BspTree2d using this polygon's edges as dividing hyperplanes and caches it in m_bsp.
	void ComputeBspTree() const;
	/// Cuts all of this polygon's edges by edge splitter passed in the argument.
	void Cut(BspLineSegSplitter&) const;

	BidirectionalMap<i32, Vector2d> m_vertices;
	BidirectionalMap<i32, Edge>     m_edges;

	/// Cached corresponding BSP tree, useful during set operations.
	mutable BspTree2d*                 m_bsp;

	std::vector<std::vector<Vector2d>> m_contours;
};

#endif
