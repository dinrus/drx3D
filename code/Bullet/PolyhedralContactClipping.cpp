#include <drx3D/Physics/Collision/NarrowPhase/PolyhedralContactClipping.h>
#include <drx3D/Physics/Collision/Shapes/ConvexPolyhedron.h>

#include <float.h>  //for FLT_MAX

i32 gExpectedNbTests = 0;
i32 gActualNbTests = 0;
bool gUseInternalObject = true;

// Clips a face to the back of a plane
void PolyhedralContactClipping::clipFace(const VertexArray& pVtxIn, VertexArray& ppVtxOut, const Vec3& planeNormalWS, Scalar planeEqWS)
{
	i32 ve;
	Scalar ds, de;
	i32 numVerts = pVtxIn.size();
	if (numVerts < 2)
		return;

	Vec3 firstVertex = pVtxIn[pVtxIn.size() - 1];
	Vec3 endVertex = pVtxIn[0];

	ds = planeNormalWS.dot(firstVertex) + planeEqWS;

	for (ve = 0; ve < numVerts; ve++)
	{
		endVertex = pVtxIn[ve];

		de = planeNormalWS.dot(endVertex) + planeEqWS;

		if (ds < 0)
		{
			if (de < 0)
			{
				// Start < 0, end < 0, so output endVertex
				ppVtxOut.push_back(endVertex);
			}
			else
			{
				// Start < 0, end >= 0, so output intersection
				ppVtxOut.push_back(firstVertex.lerp(endVertex, Scalar(ds * 1.f / (ds - de))));
			}
		}
		else
		{
			if (de < 0)
			{
				// Start >= 0, end < 0 so output intersection and end
				ppVtxOut.push_back(firstVertex.lerp(endVertex, Scalar(ds * 1.f / (ds - de))));
				ppVtxOut.push_back(endVertex);
			}
		}
		firstVertex = endVertex;
		ds = de;
	}
}

static bool TestSepAxis(const ConvexPolyhedron& hullA, const ConvexPolyhedron& hullB, const Transform2& transA, const Transform2& transB, const Vec3& sep_axis, Scalar& depth, Vec3& witnessPointA, Vec3& witnessPointB)
{
	Scalar Min0, Max0;
	Scalar Min1, Max1;
	Vec3 witnesPtMinA, witnesPtMaxA;
	Vec3 witnesPtMinB, witnesPtMaxB;

	hullA.project(transA, sep_axis, Min0, Max0, witnesPtMinA, witnesPtMaxA);
	hullB.project(transB, sep_axis, Min1, Max1, witnesPtMinB, witnesPtMaxB);

	if (Max0 < Min1 || Max1 < Min0)
		return false;

	Scalar d0 = Max0 - Min1;
	Assert(d0 >= 0.0f);
	Scalar d1 = Max1 - Min0;
	Assert(d1 >= 0.0f);
	if (d0 < d1)
	{
		depth = d0;
		witnessPointA = witnesPtMaxA;
		witnessPointB = witnesPtMinB;
	}
	else
	{
		depth = d1;
		witnessPointA = witnesPtMinA;
		witnessPointB = witnesPtMaxB;
	}

	return true;
}

static i32 gActualSATPairTests = 0;

inline bool IsAlmostZero(const Vec3& v)
{
	if (Fabs(v.x()) > 1e-6 || Fabs(v.y()) > 1e-6 || Fabs(v.z()) > 1e-6) return false;
	return true;
}

#ifdef TEST_INTERNAL_OBJECTS

inline void BoxSupport(const Scalar extents[3], const Scalar sv[3], Scalar p[3])
{
	// This version is ~11.000 cycles (4%) faster overall in one of the tests.
	//	IR(p[0]) = IR(extents[0])|(IR(sv[0])&SIGN_BITMASK);
	//	IR(p[1]) = IR(extents[1])|(IR(sv[1])&SIGN_BITMASK);
	//	IR(p[2]) = IR(extents[2])|(IR(sv[2])&SIGN_BITMASK);
	p[0] = sv[0] < 0.0f ? -extents[0] : extents[0];
	p[1] = sv[1] < 0.0f ? -extents[1] : extents[1];
	p[2] = sv[2] < 0.0f ? -extents[2] : extents[2];
}

void InverseTransform2Point3x3(Vec3& out, const Vec3& in, const Transform2& tr)
{
	const Matrix3x3& rot = tr.getBasis();
	const Vec3& r0 = rot[0];
	const Vec3& r1 = rot[1];
	const Vec3& r2 = rot[2];

	const Scalar x = r0.x() * in.x() + r1.x() * in.y() + r2.x() * in.z();
	const Scalar y = r0.y() * in.x() + r1.y() * in.y() + r2.y() * in.z();
	const Scalar z = r0.z() * in.x() + r1.z() * in.y() + r2.z() * in.z();

	out.setVal(x, y, z);
}

bool TestInternalObjects(const Transform2& trans0, const Transform2& trans1, const Vec3& delta_c, const Vec3& axis, const ConvexPolyhedron& convex0, const ConvexPolyhedron& convex1, Scalar dmin)
{
	const Scalar dp = delta_c.dot(axis);

	Vec3 localAxis0;
	InverseTransform2Point3x3(localAxis0, axis, trans0);
	Vec3 localAxis1;
	InverseTransform2Point3x3(localAxis1, axis, trans1);

	Scalar p0[3];
	BoxSupport(convex0.m_extents, localAxis0, p0);
	Scalar p1[3];
	BoxSupport(convex1.m_extents, localAxis1, p1);

	const Scalar Radius0 = p0[0] * localAxis0.x() + p0[1] * localAxis0.y() + p0[2] * localAxis0.z();
	const Scalar Radius1 = p1[0] * localAxis1.x() + p1[1] * localAxis1.y() + p1[2] * localAxis1.z();

	const Scalar MinRadius = Radius0 > convex0.m_radius ? Radius0 : convex0.m_radius;
	const Scalar MaxRadius = Radius1 > convex1.m_radius ? Radius1 : convex1.m_radius;

	const Scalar MinMaxRadius = MaxRadius + MinRadius;
	const Scalar d0 = MinMaxRadius + dp;
	const Scalar d1 = MinMaxRadius - dp;

	const Scalar depth = d0 < d1 ? d0 : d1;
	if (depth > dmin)
		return false;
	return true;
}
#endif  //TEST_INTERNAL_OBJECTS

SIMD_FORCE_INLINE void SegmentsClosestPoints(
	Vec3& ptsVector,
	Vec3& offsetA,
	Vec3& offsetB,
	Scalar& tA, Scalar& tB,
	const Vec3& translation,
	const Vec3& dirA, Scalar hlenA,
	const Vec3& dirB, Scalar hlenB)
{
	// compute the parameters of the closest points on each line segment

	Scalar dirA_dot_dirB = Dot(dirA, dirB);
	Scalar dirA_dot_trans = Dot(dirA, translation);
	Scalar dirB_dot_trans = Dot(dirB, translation);

	Scalar denom = 1.0f - dirA_dot_dirB * dirA_dot_dirB;

	if (denom == 0.0f)
	{
		tA = 0.0f;
	}
	else
	{
		tA = (dirA_dot_trans - dirB_dot_trans * dirA_dot_dirB) / denom;
		if (tA < -hlenA)
			tA = -hlenA;
		else if (tA > hlenA)
			tA = hlenA;
	}

	tB = tA * dirA_dot_dirB - dirB_dot_trans;

	if (tB < -hlenB)
	{
		tB = -hlenB;
		tA = tB * dirA_dot_dirB + dirA_dot_trans;

		if (tA < -hlenA)
			tA = -hlenA;
		else if (tA > hlenA)
			tA = hlenA;
	}
	else if (tB > hlenB)
	{
		tB = hlenB;
		tA = tB * dirA_dot_dirB + dirA_dot_trans;

		if (tA < -hlenA)
			tA = -hlenA;
		else if (tA > hlenA)
			tA = hlenA;
	}

	// compute the closest points relative to segment centers.

	offsetA = dirA * tA;
	offsetB = dirB * tB;

	ptsVector = translation - offsetA + offsetB;
}

bool PolyhedralContactClipping::findSeparatingAxis(const ConvexPolyhedron& hullA, const ConvexPolyhedron& hullB, const Transform2& transA, const Transform2& transB, Vec3& sep, DiscreteCollisionDetectorInterface::Result& resultOut)
{
	gActualSATPairTests++;

	//#ifdef TEST_INTERNAL_OBJECTS
	const Vec3 c0 = transA * hullA.m_localCenter;
	const Vec3 c1 = transB * hullB.m_localCenter;
	const Vec3 DeltaC2 = c0 - c1;
	//#endif

	Scalar dmin = FLT_MAX;
	i32 curPlaneTests = 0;

	i32 numFacesA = hullA.m_faces.size();
	// Test normals from hullA
	for (i32 i = 0; i < numFacesA; i++)
	{
		const Vec3 Normal(hullA.m_faces[i].m_plane[0], hullA.m_faces[i].m_plane[1], hullA.m_faces[i].m_plane[2]);
		Vec3 faceANormalWS = transA.getBasis() * Normal;
		if (DeltaC2.dot(faceANormalWS) < 0)
			faceANormalWS *= -1.f;

		curPlaneTests++;
#ifdef TEST_INTERNAL_OBJECTS
		gExpectedNbTests++;
		if (gUseInternalObject && !TestInternalObjects(transA, transB, DeltaC2, faceANormalWS, hullA, hullB, dmin))
			continue;
		gActualNbTests++;
#endif

		Scalar d;
		Vec3 wA, wB;
		if (!TestSepAxis(hullA, hullB, transA, transB, faceANormalWS, d, wA, wB))
			return false;

		if (d < dmin)
		{
			dmin = d;
			sep = faceANormalWS;
		}
	}

	i32 numFacesB = hullB.m_faces.size();
	// Test normals from hullB
	for (i32 i = 0; i < numFacesB; i++)
	{
		const Vec3 Normal(hullB.m_faces[i].m_plane[0], hullB.m_faces[i].m_plane[1], hullB.m_faces[i].m_plane[2]);
		Vec3 WorldNormal = transB.getBasis() * Normal;
		if (DeltaC2.dot(WorldNormal) < 0)
			WorldNormal *= -1.f;

		curPlaneTests++;
#ifdef TEST_INTERNAL_OBJECTS
		gExpectedNbTests++;
		if (gUseInternalObject && !TestInternalObjects(transA, transB, DeltaC2, WorldNormal, hullA, hullB, dmin))
			continue;
		gActualNbTests++;
#endif

		Scalar d;
		Vec3 wA, wB;
		if (!TestSepAxis(hullA, hullB, transA, transB, WorldNormal, d, wA, wB))
			return false;

		if (d < dmin)
		{
			dmin = d;
			sep = WorldNormal;
		}
	}

	Vec3 edgeAstart, edgeAend, edgeBstart, edgeBend;
	i32 edgeA = -1;
	i32 edgeB = -1;
	Vec3 worldEdgeA;
	Vec3 worldEdgeB;
	Vec3 witnessPointA(0, 0, 0), witnessPointB(0, 0, 0);

	i32 curEdgeEdge = 0;
	// Test edges
	for (i32 e0 = 0; e0 < hullA.m_uniqueEdges.size(); e0++)
	{
		const Vec3 edge0 = hullA.m_uniqueEdges[e0];
		const Vec3 WorldEdge0 = transA.getBasis() * edge0;
		for (i32 e1 = 0; e1 < hullB.m_uniqueEdges.size(); e1++)
		{
			const Vec3 edge1 = hullB.m_uniqueEdges[e1];
			const Vec3 WorldEdge1 = transB.getBasis() * edge1;

			Vec3 Cross = WorldEdge0.cross(WorldEdge1);
			curEdgeEdge++;
			if (!IsAlmostZero(Cross))
			{
				Cross = Cross.normalize();
				if (DeltaC2.dot(Cross) < 0)
					Cross *= -1.f;

#ifdef TEST_INTERNAL_OBJECTS
				gExpectedNbTests++;
				if (gUseInternalObject && !TestInternalObjects(transA, transB, DeltaC2, Cross, hullA, hullB, dmin))
					continue;
				gActualNbTests++;
#endif

				Scalar dist;
				Vec3 wA, wB;
				if (!TestSepAxis(hullA, hullB, transA, transB, Cross, dist, wA, wB))
					return false;

				if (dist < dmin)
				{
					dmin = dist;
					sep = Cross;
					edgeA = e0;
					edgeB = e1;
					worldEdgeA = WorldEdge0;
					worldEdgeB = WorldEdge1;
					witnessPointA = wA;
					witnessPointB = wB;
				}
			}
		}
	}

	if (edgeA >= 0 && edgeB >= 0)
	{
		//		printf("edge-edge\n");
		//add an edge-edge contact

		Vec3 ptsVector;
		Vec3 offsetA;
		Vec3 offsetB;
		Scalar tA;
		Scalar tB;

		Vec3 translation = witnessPointB - witnessPointA;

		Vec3 dirA = worldEdgeA;
		Vec3 dirB = worldEdgeB;

		Scalar hlenB = 1e30f;
		Scalar hlenA = 1e30f;

		SegmentsClosestPoints(ptsVector, offsetA, offsetB, tA, tB,
								translation,
								dirA, hlenA,
								dirB, hlenB);

		Scalar nlSqrt = ptsVector.length2();
		if (nlSqrt > SIMD_EPSILON)
		{
			Scalar nl = Sqrt(nlSqrt);
			ptsVector *= 1.f / nl;
			if (ptsVector.dot(DeltaC2) < 0.f)
			{
				ptsVector *= -1.f;
			}
			Vec3 ptOnB = witnessPointB + offsetB;
			Scalar distance = nl;
			resultOut.addContactPoint(ptsVector, ptOnB, -distance);
		}
	}

	if ((DeltaC2.dot(sep)) < 0.0f)
		sep = -sep;

	return true;
}

void PolyhedralContactClipping::clipFaceAgainstHull(const Vec3& separatingNormal, const ConvexPolyhedron& hullA, const Transform2& transA, VertexArray& worldVertsB1, VertexArray& worldVertsB2, const Scalar minDist, Scalar maxDist, DiscreteCollisionDetectorInterface::Result& resultOut)
{
	worldVertsB2.resize(0);
	VertexArray* pVtxIn = &worldVertsB1;
	VertexArray* pVtxOut = &worldVertsB2;
	pVtxOut->reserve(pVtxIn->size());

	i32 closestFaceA = -1;
	{
		Scalar dmin = FLT_MAX;
		for (i32 face = 0; face < hullA.m_faces.size(); face++)
		{
			const Vec3 Normal(hullA.m_faces[face].m_plane[0], hullA.m_faces[face].m_plane[1], hullA.m_faces[face].m_plane[2]);
			const Vec3 faceANormalWS = transA.getBasis() * Normal;

			Scalar d = faceANormalWS.dot(separatingNormal);
			if (d < dmin)
			{
				dmin = d;
				closestFaceA = face;
			}
		}
	}
	if (closestFaceA < 0)
		return;

	const Face& polyA = hullA.m_faces[closestFaceA];

	// clip polygon to back of planes of all faces of hull A that are adjacent to witness face
	i32 numVerticesA = polyA.m_indices.size();
	for (i32 e0 = 0; e0 < numVerticesA; e0++)
	{
		const Vec3& a = hullA.m_vertices[polyA.m_indices[e0]];
		const Vec3& b = hullA.m_vertices[polyA.m_indices[(e0 + 1) % numVerticesA]];
		const Vec3 edge0 = a - b;
		const Vec3 WorldEdge0 = transA.getBasis() * edge0;
		Vec3 worldPlaneAnormal1 = transA.getBasis() * Vec3(polyA.m_plane[0], polyA.m_plane[1], polyA.m_plane[2]);

		Vec3 planeNormalWS1 = -WorldEdge0.cross(worldPlaneAnormal1);  //.cross(WorldEdge0);
		Vec3 worldA1 = transA * a;
		Scalar planeEqWS1 = -worldA1.dot(planeNormalWS1);

//i32 otherFace=0;
#ifdef BLA1
		i32 otherFace = polyA.m_connectedFaces[e0];
		Vec3 localPlaneNormal(hullA.m_faces[otherFace].m_plane[0], hullA.m_faces[otherFace].m_plane[1], hullA.m_faces[otherFace].m_plane[2]);
		Scalar localPlaneEq = hullA.m_faces[otherFace].m_plane[3];

		Vec3 planeNormalWS = transA.getBasis() * localPlaneNormal;
		Scalar planeEqWS = localPlaneEq - planeNormalWS.dot(transA.getOrigin());
#else
		Vec3 planeNormalWS = planeNormalWS1;
		Scalar planeEqWS = planeEqWS1;

#endif
		//clip face

		clipFace(*pVtxIn, *pVtxOut, planeNormalWS, planeEqWS);
		Swap(pVtxIn, pVtxOut);
		pVtxOut->resize(0);
	}

	//#define ONLY_REPORT_DEEPEST_POINT

	Vec3 point;

	// only keep points that are behind the witness face
	{
		Vec3 localPlaneNormal(polyA.m_plane[0], polyA.m_plane[1], polyA.m_plane[2]);
		Scalar localPlaneEq = polyA.m_plane[3];
		Vec3 planeNormalWS = transA.getBasis() * localPlaneNormal;
		Scalar planeEqWS = localPlaneEq - planeNormalWS.dot(transA.getOrigin());
		for (i32 i = 0; i < pVtxIn->size(); i++)
		{
			Vec3 vtx = pVtxIn->at(i);
			Scalar depth = planeNormalWS.dot(vtx) + planeEqWS;
			if (depth <= minDist)
			{
				//				printf("clamped: depth=%f to minDist=%f\n",depth,minDist);
				depth = minDist;
			}

			if (depth <= maxDist)
			{
				Vec3 point = pVtxIn->at(i);
#ifdef ONLY_REPORT_DEEPEST_POINT
				curMaxDist = depth;
#else
#if 0
				if (depth<-3)
				{
					printf("error in PolyhedralContactClipping depth = %f\n", depth);
					printf("likely wrong separatingNormal passed in\n");
				}
#endif
				resultOut.addContactPoint(separatingNormal, point, depth);
#endif
			}
		}
	}
#ifdef ONLY_REPORT_DEEPEST_POINT
	if (curMaxDist < maxDist)
	{
		resultOut.addContactPoint(separatingNormal, point, curMaxDist);
	}
#endif  //ONLY_REPORT_DEEPEST_POINT
}

void PolyhedralContactClipping::clipHullAgainstHull(const Vec3& separatingNormal1, const ConvexPolyhedron& hullA, const ConvexPolyhedron& hullB, const Transform2& transA, const Transform2& transB, const Scalar minDist, Scalar maxDist, VertexArray& worldVertsB1, VertexArray& worldVertsB2, DiscreteCollisionDetectorInterface::Result& resultOut)
{
	Vec3 separatingNormal = separatingNormal1.normalized();
	//	const Vec3 c0 = transA * hullA.m_localCenter;
	//	const Vec3 c1 = transB * hullB.m_localCenter;
	//const Vec3 DeltaC2 = c0 - c1;

	i32 closestFaceB = -1;
	Scalar dmax = -FLT_MAX;
	{
		for (i32 face = 0; face < hullB.m_faces.size(); face++)
		{
			const Vec3 Normal(hullB.m_faces[face].m_plane[0], hullB.m_faces[face].m_plane[1], hullB.m_faces[face].m_plane[2]);
			const Vec3 WorldNormal = transB.getBasis() * Normal;
			Scalar d = WorldNormal.dot(separatingNormal);
			if (d > dmax)
			{
				dmax = d;
				closestFaceB = face;
			}
		}
	}
	worldVertsB1.resize(0);
	{
		const Face& polyB = hullB.m_faces[closestFaceB];
		i32k numVertices = polyB.m_indices.size();
		for (i32 e0 = 0; e0 < numVertices; e0++)
		{
			const Vec3& b = hullB.m_vertices[polyB.m_indices[e0]];
			worldVertsB1.push_back(transB * b);
		}
	}

	if (closestFaceB >= 0)
		clipFaceAgainstHull(separatingNormal, hullA, transA, worldVertsB1, worldVertsB2, minDist, maxDist, resultOut);
}
