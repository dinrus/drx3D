#ifndef DRX3D_POLYHEDRAL_CONTACT_CLIPPING_H
#define DRX3D_POLYHEDRAL_CONTACT_CLIPPING_H

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Physics/Collision/NarrowPhase/DiscreteCollisionDetectorInterface.h>

class ConvexPolyhedron;

typedef AlignedObjectArray<Vec3> VertexArray;

// Clips a face to the back of a plane
struct PolyhedralContactClipping
{
	static void clipHullAgainstHull(const Vec3& separatingNormal1, const ConvexPolyhedron& hullA, const ConvexPolyhedron& hullB, const Transform2& transA, const Transform2& transB, const Scalar minDist, Scalar maxDist, VertexArray& worldVertsB1, VertexArray& worldVertsB2, DiscreteCollisionDetectorInterface::Result& resultOut);

	static void clipFaceAgainstHull(const Vec3& separatingNormal, const ConvexPolyhedron& hullA, const Transform2& transA, VertexArray& worldVertsB1, VertexArray& worldVertsB2, const Scalar minDist, Scalar maxDist, DiscreteCollisionDetectorInterface::Result& resultOut);

	static bool findSeparatingAxis(const ConvexPolyhedron& hullA, const ConvexPolyhedron& hullB, const Transform2& transA, const Transform2& transB, Vec3& sep, DiscreteCollisionDetectorInterface::Result& resultOut);

	///the clipFace method is used internally
	static void clipFace(const VertexArray& pVtxIn, VertexArray& ppVtxOut, const Vec3& planeNormalWS, Scalar planeEqWS);
};

#endif  // DRX3D_POLYHEDRAL_CONTACT_CLIPPING_H
