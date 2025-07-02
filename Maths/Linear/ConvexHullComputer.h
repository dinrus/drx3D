#ifndef DRX3D_CONVEX_HULL_COMPUTER_H
#define DRX3D_CONVEX_HULL_COMPUTER_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

/// Convex hull implementation based on Preparata and Hong
/// See http://code.google.com/p/bullet/issues/detail?id=275
/// Ole Kniemeyer, MAXON Computer GmbH
class ConvexHullComputer
{
private:
	Scalar compute(ukk coords, bool doubleCoords, i32 stride, i32 count, Scalar shrink, Scalar shrinkClamp);

public:
	class Edge
	{
	private:
		i32 next;
		i32 reverse;
		i32 targetVertex;

		friend class ConvexHullComputer;

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
	AlignedObjectArray<Vec3> vertices;

	// The original vertex index in the input coords array
	AlignedObjectArray<i32> original_vertex_index;

	// Edges of the output hull
	AlignedObjectArray<Edge> edges;

	// Faces of the convex hull. Each entry is an index into the "edges" array pointing to an edge of the face. Faces are planar n-gons
	AlignedObjectArray<i32> faces;

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
	Scalar compute(const float* coords, i32 stride, i32 count, Scalar shrink, Scalar shrinkClamp)
	{
		return compute(coords, false, stride, count, shrink, shrinkClamp);
	}

	// same as above, but double precision
	Scalar compute(const double* coords, i32 stride, i32 count, Scalar shrink, Scalar shrinkClamp)
	{
		return compute(coords, true, stride, count, shrink, shrinkClamp);
	}
};

#endif  //DRX3D_CONVEX_HULL_COMPUTER_H
