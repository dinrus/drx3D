#ifndef D3_CONVEX_HULL_COMPUTER_H
#define D3_CONVEX_HULL_COMPUTER_H

#include <drx3D/Common/b3Vec3.h>
#include <drx3D/Common/b3AlignedObjectArray.h>

/// Convex hull implementation based on Preparata and Hong
/// See http://code.google.com/p/bullet/issues/detail?id=275
/// Ole Kniemeyer, MAXON Computer GmbH
class b3ConvexHullComputer
{
private:
	b3Scalar compute(ukk coords, bool doubleCoords, i32 stride, i32 count, b3Scalar shrink, b3Scalar shrinkClamp);

public:
	class Edge
	{
	private:
		i32 next;
		i32 reverse;
		i32 targetVertex;

		friend class b3ConvexHullComputer;

	public:
		i32 getSourceVertex() const
		{
			return (this + reverse)->targetVertex;
		}

		i32 getTargetVertex() const
		{
			return targetVertex;
		}

		const Edge* getNextEdgeOfVertex() const  // clockwise list of all edges of a vertex
		{
			return this + next;
		}

		const Edge* getNextEdgeOfFace() const  // counter-clockwise list of all edges of a face
		{
			return (this + reverse)->getNextEdgeOfVertex();
		}

		const Edge* getReverseEdge() const
		{
			return this + reverse;
		}
	};

	// Vertices of the output hull
	b3AlignedObjectArray<b3Vec3> vertices;

	// Edges of the output hull
	b3AlignedObjectArray<Edge> edges;

	// Faces of the convex hull. Each entry is an index into the "edges" array pointing to an edge of the face. Faces are planar n-gons
	b3AlignedObjectArray<i32> faces;

	/*
		Compute convex hull of "count" vertices stored in "coords". "stride" is the difference in bytes
		between the addresses of consecutive vertices. If "shrink" is positive, the convex hull is shrunken
		by that amount (each face is moved by "shrink" length units towards the center along its normal).
		If "shrinkClamp" is positive, "shrink" is clamped to not exceed "shrinkClamp * innerRadius", where "innerRadius"
		is the minimum distance of a face to the center of the convex hull.

		The returned value is the amount by which the hull has been shrunken. If it is negative, the amount was so large
		that the resulting convex hull is empty.

		The output convex hull can be found in the member variables "vertices", "edges", "faces".
		*/
	b3Scalar compute(const float* coords, i32 stride, i32 count, b3Scalar shrink, b3Scalar shrinkClamp)
	{
		return compute(coords, false, stride, count, shrink, shrinkClamp);
	}

	// same as above, but double precision
	b3Scalar compute(const double* coords, i32 stride, i32 count, b3Scalar shrink, b3Scalar shrinkClamp)
	{
		return compute(coords, true, stride, count, shrink, shrinkClamp);
	}
};

#endif  //D3_CONVEX_HULL_COMPUTER_H
