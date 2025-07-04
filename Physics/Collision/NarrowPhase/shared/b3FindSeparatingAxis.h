#ifndef D3_FIND_SEPARATING_AXIS_H
#define D3_FIND_SEPARATING_AXIS_H

inline void b3ProjectAxis(const b3ConvexPolyhedronData& hull, const b3Float4& pos, const b3Quat& orn, const b3Float4& dir, const b3AlignedObjectArray<b3Vec3>& vertices, b3Scalar& min, b3Scalar& max)
{
	min = FLT_MAX;
	max = -FLT_MAX;
	i32 numVerts = hull.m_numVertices;

	const b3Float4 localDir = b3QuatRotate(orn.inverse(), dir);

	b3Scalar offset = b3Dot3F4(pos, dir);

	for (i32 i = 0; i < numVerts; i++)
	{
		//b3Vec3 pt = trans * vertices[m_vertexOffset+i];
		//b3Scalar dp = pt.dot(dir);
		//b3Vec3 vertex = vertices[hull.m_vertexOffset+i];
		b3Scalar dp = b3Dot3F4((b3Float4&)vertices[hull.m_vertexOffset + i], localDir);
		//drx3DAssert(dp==dpL);
		if (dp < min) min = dp;
		if (dp > max) max = dp;
	}
	if (min > max)
	{
		b3Scalar tmp = min;
		min = max;
		max = tmp;
	}
	min += offset;
	max += offset;
}

inline bool b3TestSepAxis(const b3ConvexPolyhedronData& hullA, const b3ConvexPolyhedronData& hullB,
						  const b3Float4& posA, const b3Quat& ornA,
						  const b3Float4& posB, const b3Quat& ornB,
						  const b3Float4& sep_axis, const b3AlignedObjectArray<b3Vec3>& verticesA, const b3AlignedObjectArray<b3Vec3>& verticesB, b3Scalar& depth)
{
	b3Scalar Min0, Max0;
	b3Scalar Min1, Max1;
	b3ProjectAxis(hullA, posA, ornA, sep_axis, verticesA, Min0, Max0);
	b3ProjectAxis(hullB, posB, ornB, sep_axis, verticesB, Min1, Max1);

	if (Max0 < Min1 || Max1 < Min0)
		return false;

	b3Scalar d0 = Max0 - Min1;
	drx3DAssert(d0 >= 0.0f);
	b3Scalar d1 = Max1 - Min0;
	drx3DAssert(d1 >= 0.0f);
	depth = d0 < d1 ? d0 : d1;
	return true;
}

inline bool b3FindSeparatingAxis(const b3ConvexPolyhedronData& hullA, const b3ConvexPolyhedronData& hullB,
								 const b3Float4& posA1,
								 const b3Quat& ornA,
								 const b3Float4& posB1,
								 const b3Quat& ornB,
								 const b3AlignedObjectArray<b3Vec3>& verticesA,
								 const b3AlignedObjectArray<b3Vec3>& uniqueEdgesA,
								 const b3AlignedObjectArray<b3GpuFace>& facesA,
								 const b3AlignedObjectArray<i32>& indicesA,
								 const b3AlignedObjectArray<b3Vec3>& verticesB,
								 const b3AlignedObjectArray<b3Vec3>& uniqueEdgesB,
								 const b3AlignedObjectArray<b3GpuFace>& facesB,
								 const b3AlignedObjectArray<i32>& indicesB,

								 b3Vec3& sep)
{
	D3_PROFILE("findSeparatingAxis");

	b3Float4 posA = posA1;
	posA.w = 0.f;
	b3Float4 posB = posB1;
	posB.w = 0.f;
	//#ifdef TEST_INTERNAL_OBJECTS
	b3Float4 c0local = (b3Float4&)hullA.m_localCenter;

	b3Float4 c0 = b3TransformPoint(c0local, posA, ornA);
	b3Float4 c1local = (b3Float4&)hullB.m_localCenter;
	b3Float4 c1 = b3TransformPoint(c1local, posB, ornB);
	const b3Float4 deltaC2 = c0 - c1;
	//#endif

	b3Scalar dmin = FLT_MAX;
	i32 curPlaneTests = 0;

	i32 numFacesA = hullA.m_numFaces;
	// Test normals from hullA
	for (i32 i = 0; i < numFacesA; i++)
	{
		const b3Float4& normal = (b3Float4&)facesA[hullA.m_faceOffset + i].m_plane;
		b3Float4 faceANormalWS = b3QuatRotate(ornA, normal);

		if (b3Dot3F4(deltaC2, faceANormalWS) < 0)
			faceANormalWS *= -1.f;

		curPlaneTests++;
#ifdef TEST_INTERNAL_OBJECTS
		gExpectedNbTests++;
		if (gUseInternalObject && !TestInternalObjects(transA, transB, DeltaC2, faceANormalWS, hullA, hullB, dmin))
			continue;
		gActualNbTests++;
#endif

		b3Scalar d;
		if (!b3TestSepAxis(hullA, hullB, posA, ornA, posB, ornB, faceANormalWS, verticesA, verticesB, d))
			return false;

		if (d < dmin)
		{
			dmin = d;
			sep = (b3Vec3&)faceANormalWS;
		}
	}

	i32 numFacesB = hullB.m_numFaces;
	// Test normals from hullB
	for (i32 i = 0; i < numFacesB; i++)
	{
		b3Float4 normal = (b3Float4&)facesB[hullB.m_faceOffset + i].m_plane;
		b3Float4 WorldNormal = b3QuatRotate(ornB, normal);

		if (b3Dot3F4(deltaC2, WorldNormal) < 0)
		{
			WorldNormal *= -1.f;
		}
		curPlaneTests++;
#ifdef TEST_INTERNAL_OBJECTS
		gExpectedNbTests++;
		if (gUseInternalObject && !TestInternalObjects(transA, transB, DeltaC2, WorldNormal, hullA, hullB, dmin))
			continue;
		gActualNbTests++;
#endif

		b3Scalar d;
		if (!b3TestSepAxis(hullA, hullB, posA, ornA, posB, ornB, WorldNormal, verticesA, verticesB, d))
			return false;

		if (d < dmin)
		{
			dmin = d;
			sep = (b3Vec3&)WorldNormal;
		}
	}

	//	b3Vec3 edgeAstart,edgeAend,edgeBstart,edgeBend;

	i32 curEdgeEdge = 0;
	// Test edges
	for (i32 e0 = 0; e0 < hullA.m_numUniqueEdges; e0++)
	{
		const b3Float4& edge0 = (b3Float4&)uniqueEdgesA[hullA.m_uniqueEdgesOffset + e0];
		b3Float4 edge0World = b3QuatRotate(ornA, (b3Float4&)edge0);

		for (i32 e1 = 0; e1 < hullB.m_numUniqueEdges; e1++)
		{
			const b3Vec3 edge1 = uniqueEdgesB[hullB.m_uniqueEdgesOffset + e1];
			b3Float4 edge1World = b3QuatRotate(ornB, (b3Float4&)edge1);

			b3Float4 crossje = b3Cross3(edge0World, edge1World);

			curEdgeEdge++;
			if (!b3IsAlmostZero((b3Vec3&)crossje))
			{
				crossje = b3FastNormalized3(crossje);
				if (b3Dot3F4(deltaC2, crossje) < 0)
					crossje *= -1.f;

#ifdef TEST_INTERNAL_OBJECTS
				gExpectedNbTests++;
				if (gUseInternalObject && !TestInternalObjects(transA, transB, DeltaC2, Cross, hullA, hullB, dmin))
					continue;
				gActualNbTests++;
#endif

				b3Scalar dist;
				if (!b3TestSepAxis(hullA, hullB, posA, ornA, posB, ornB, crossje, verticesA, verticesB, dist))
					return false;

				if (dist < dmin)
				{
					dmin = dist;
					sep = (b3Vec3&)crossje;
				}
			}
		}
	}

	if ((b3Dot3F4(-deltaC2, (b3Float4&)sep)) > 0.0f)
		sep = -sep;

	return true;
}

#endif  //D3_FIND_SEPARATING_AXIS_H
