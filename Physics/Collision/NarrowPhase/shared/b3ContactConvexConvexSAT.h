
#ifndef D3_CONTACT_CONVEX_CONVEX_SAT_H
#define D3_CONTACT_CONVEX_CONVEX_SAT_H

#include <drx3D/Physics/Collision/NarrowPhase/shared/b3Contact4Data.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3FindSeparatingAxis.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3ReduceContacts.h>

#define D3_MAX_VERTS 1024

inline b3Float4 b3Lerp3(const b3Float4& a, const b3Float4& b, float t)
{
	return b3MakeVector3(a.x + (b.x - a.x) * t,
						 a.y + (b.y - a.y) * t,
						 a.z + (b.z - a.z) * t,
						 0.f);
}

// Clips a face to the back of a plane, return the number of vertices out, stored in ppVtxOut
inline i32 b3ClipFace(const b3Float4* pVtxIn, i32 numVertsIn, b3Float4& planeNormalWS, float planeEqWS, b3Float4* ppVtxOut)
{
	i32 ve;
	float ds, de;
	i32 numVertsOut = 0;
	if (numVertsIn < 2)
		return 0;

	b3Float4 firstVertex = pVtxIn[numVertsIn - 1];
	b3Float4 endVertex = pVtxIn[0];

	ds = b3Dot3F4(planeNormalWS, firstVertex) + planeEqWS;

	for (ve = 0; ve < numVertsIn; ve++)
	{
		endVertex = pVtxIn[ve];

		de = b3Dot3F4(planeNormalWS, endVertex) + planeEqWS;

		if (ds < 0)
		{
			if (de < 0)
			{
				// Start < 0, end < 0, so output endVertex
				ppVtxOut[numVertsOut++] = endVertex;
			}
			else
			{
				// Start < 0, end >= 0, so output intersection
				ppVtxOut[numVertsOut++] = b3Lerp3(firstVertex, endVertex, (ds * 1.f / (ds - de)));
			}
		}
		else
		{
			if (de < 0)
			{
				// Start >= 0, end < 0 so output intersection and end
				ppVtxOut[numVertsOut++] = b3Lerp3(firstVertex, endVertex, (ds * 1.f / (ds - de)));
				ppVtxOut[numVertsOut++] = endVertex;
			}
		}
		firstVertex = endVertex;
		ds = de;
	}
	return numVertsOut;
}

inline i32 b3ClipFaceAgainstHull(const b3Float4& separatingNormal, const b3ConvexPolyhedronData* hullA,
								 const b3Float4& posA, const b3Quat& ornA, b3Float4* worldVertsB1, i32 numWorldVertsB1,
								 b3Float4* worldVertsB2, i32 capacityWorldVertsB2,
								 const float minDist, float maxDist,
								 const b3AlignedObjectArray<b3Float4>& verticesA, const b3AlignedObjectArray<b3GpuFace>& facesA, const b3AlignedObjectArray<i32>& indicesA,
								 //const b3Float4* verticesB,	const b3GpuFace* facesB,	i32k* indicesB,
								 b3Float4* contactsOut,
								 i32 contactCapacity)
{
	i32 numContactsOut = 0;

	b3Float4* pVtxIn = worldVertsB1;
	b3Float4* pVtxOut = worldVertsB2;

	i32 numVertsIn = numWorldVertsB1;
	i32 numVertsOut = 0;

	i32 closestFaceA = -1;
	{
		float dmin = FLT_MAX;
		for (i32 face = 0; face < hullA->m_numFaces; face++)
		{
			const b3Float4 Normal = b3MakeVector3(
				facesA[hullA->m_faceOffset + face].m_plane.x,
				facesA[hullA->m_faceOffset + face].m_plane.y,
				facesA[hullA->m_faceOffset + face].m_plane.z, 0.f);
			const b3Float4 faceANormalWS = b3QuatRotate(ornA, Normal);

			float d = b3Dot3F4(faceANormalWS, separatingNormal);
			if (d < dmin)
			{
				dmin = d;
				closestFaceA = face;
			}
		}
	}
	if (closestFaceA < 0)
		return numContactsOut;

	b3GpuFace polyA = facesA[hullA->m_faceOffset + closestFaceA];

	// clip polygon to back of planes of all faces of hull A that are adjacent to witness face
	//i32 numContacts = numWorldVertsB1;
	i32 numVerticesA = polyA.m_numIndices;
	for (i32 e0 = 0; e0 < numVerticesA; e0++)
	{
		const b3Float4 a = verticesA[hullA->m_vertexOffset + indicesA[polyA.m_indexOffset + e0]];
		const b3Float4 b = verticesA[hullA->m_vertexOffset + indicesA[polyA.m_indexOffset + ((e0 + 1) % numVerticesA)]];
		const b3Float4 edge0 = a - b;
		const b3Float4 WorldEdge0 = b3QuatRotate(ornA, edge0);
		b3Float4 planeNormalA = b3MakeFloat4(polyA.m_plane.x, polyA.m_plane.y, polyA.m_plane.z, 0.f);
		b3Float4 worldPlaneAnormal1 = b3QuatRotate(ornA, planeNormalA);

		b3Float4 planeNormalWS1 = -b3Cross3(WorldEdge0, worldPlaneAnormal1);
		b3Float4 worldA1 = b3TransformPoint(a, posA, ornA);
		float planeEqWS1 = -b3Dot3F4(worldA1, planeNormalWS1);

		b3Float4 planeNormalWS = planeNormalWS1;
		float planeEqWS = planeEqWS1;

		//clip face
		//clipFace(*pVtxIn, *pVtxOut,planeNormalWS,planeEqWS);
		numVertsOut = b3ClipFace(pVtxIn, numVertsIn, planeNormalWS, planeEqWS, pVtxOut);

		//Swap(pVtxIn,pVtxOut);
		b3Float4* tmp = pVtxOut;
		pVtxOut = pVtxIn;
		pVtxIn = tmp;
		numVertsIn = numVertsOut;
		numVertsOut = 0;
	}

	// only keep points that are behind the witness face
	{
		b3Float4 localPlaneNormal = b3MakeFloat4(polyA.m_plane.x, polyA.m_plane.y, polyA.m_plane.z, 0.f);
		float localPlaneEq = polyA.m_plane.w;
		b3Float4 planeNormalWS = b3QuatRotate(ornA, localPlaneNormal);
		float planeEqWS = localPlaneEq - b3Dot3F4(planeNormalWS, posA);
		for (i32 i = 0; i < numVertsIn; i++)
		{
			float depth = b3Dot3F4(planeNormalWS, pVtxIn[i]) + planeEqWS;
			if (depth <= minDist)
			{
				depth = minDist;
			}
			if (numContactsOut < contactCapacity)
			{
				if (depth <= maxDist)
				{
					b3Float4 pointInWorld = pVtxIn[i];
					//resultOut.addContactPoint(separatingNormal,point,depth);
					contactsOut[numContactsOut++] = b3MakeVector3(pointInWorld.x, pointInWorld.y, pointInWorld.z, depth);
					//printf("depth=%f\n",depth);
				}
			}
			else
			{
				drx3DError("exceeding contact capacity (%d,%df)\n", numContactsOut, contactCapacity);
			}
		}
	}

	return numContactsOut;
}

inline i32 b3ClipHullAgainstHull(const b3Float4& separatingNormal,
								 const b3ConvexPolyhedronData& hullA, const b3ConvexPolyhedronData& hullB,
								 const b3Float4& posA, const b3Quat& ornA, const b3Float4& posB, const b3Quat& ornB,
								 b3Float4* worldVertsB1, b3Float4* worldVertsB2, i32 capacityWorldVerts,
								 const float minDist, float maxDist,
								 const b3AlignedObjectArray<b3Float4>& verticesA, const b3AlignedObjectArray<b3GpuFace>& facesA, const b3AlignedObjectArray<i32>& indicesA,
								 const b3AlignedObjectArray<b3Float4>& verticesB, const b3AlignedObjectArray<b3GpuFace>& facesB, const b3AlignedObjectArray<i32>& indicesB,

								 b3Float4* contactsOut,
								 i32 contactCapacity)
{
	i32 numContactsOut = 0;
	i32 numWorldVertsB1 = 0;

	D3_PROFILE("clipHullAgainstHull");

	//float curMaxDist=maxDist;
	i32 closestFaceB = -1;
	float dmax = -FLT_MAX;

	{
		//D3_PROFILE("closestFaceB");
		if (hullB.m_numFaces != 1)
		{
			//printf("wtf\n");
		}
		static bool once = true;
		//printf("separatingNormal=%f,%f,%f\n",separatingNormal.x,separatingNormal.y,separatingNormal.z);

		for (i32 face = 0; face < hullB.m_numFaces; face++)
		{
#ifdef DRX3D_DEBUG_SAT_FACE
			if (once)
				printf("face %d\n", face);
			const b3GpuFace* faceB = &facesB[hullB.m_faceOffset + face];
			if (once)
			{
				for (i32 i = 0; i < faceB->m_numIndices; i++)
				{
					b3Float4 vert = verticesB[hullB.m_vertexOffset + indicesB[faceB->m_indexOffset + i]];
					printf("vert[%d] = %f,%f,%f\n", i, vert.x, vert.y, vert.z);
				}
			}
#endif  //DRX3D_DEBUG_SAT_FACE \
	//if (facesB[hullB.m_faceOffset+face].m_numIndices>2)
			{
				const b3Float4 Normal = b3MakeVector3(facesB[hullB.m_faceOffset + face].m_plane.x,
													  facesB[hullB.m_faceOffset + face].m_plane.y, facesB[hullB.m_faceOffset + face].m_plane.z, 0.f);
				const b3Float4 WorldNormal = b3QuatRotate(ornB, Normal);
#ifdef DRX3D_DEBUG_SAT_FACE
				if (once)
					printf("faceNormal = %f,%f,%f\n", Normal.x, Normal.y, Normal.z);
#endif
				float d = b3Dot3F4(WorldNormal, separatingNormal);
				if (d > dmax)
				{
					dmax = d;
					closestFaceB = face;
				}
			}
		}
		once = false;
	}

	drx3DAssert(closestFaceB >= 0);
	{
		//D3_PROFILE("worldVertsB1");
		const b3GpuFace& polyB = facesB[hullB.m_faceOffset + closestFaceB];
		i32k numVertices = polyB.m_numIndices;
		for (i32 e0 = 0; e0 < numVertices; e0++)
		{
			const b3Float4& b = verticesB[hullB.m_vertexOffset + indicesB[polyB.m_indexOffset + e0]];
			worldVertsB1[numWorldVertsB1++] = b3TransformPoint(b, posB, ornB);
		}
	}

	if (closestFaceB >= 0)
	{
		//D3_PROFILE("clipFaceAgainstHull");
		numContactsOut = b3ClipFaceAgainstHull((b3Float4&)separatingNormal, &hullA,
											   posA, ornA,
											   worldVertsB1, numWorldVertsB1, worldVertsB2, capacityWorldVerts, minDist, maxDist,
											   verticesA, facesA, indicesA,
											   contactsOut, contactCapacity);
	}

	return numContactsOut;
}

inline i32 b3ClipHullHullSingle(
	i32 bodyIndexA, i32 bodyIndexB,
	const b3Float4& posA,
	const b3Quat& ornA,
	const b3Float4& posB,
	const b3Quat& ornB,

	i32 collidableIndexA, i32 collidableIndexB,

	const b3AlignedObjectArray<b3RigidBodyData>* bodyBuf,
	b3AlignedObjectArray<b3Contact4Data>* globalContactOut,
	i32& nContacts,

	const b3AlignedObjectArray<b3ConvexPolyhedronData>& hostConvexDataA,
	const b3AlignedObjectArray<b3ConvexPolyhedronData>& hostConvexDataB,

	const b3AlignedObjectArray<b3Vec3>& verticesA,
	const b3AlignedObjectArray<b3Vec3>& uniqueEdgesA,
	const b3AlignedObjectArray<b3GpuFace>& facesA,
	const b3AlignedObjectArray<i32>& indicesA,

	const b3AlignedObjectArray<b3Vec3>& verticesB,
	const b3AlignedObjectArray<b3Vec3>& uniqueEdgesB,
	const b3AlignedObjectArray<b3GpuFace>& facesB,
	const b3AlignedObjectArray<i32>& indicesB,

	const b3AlignedObjectArray<b3Collidable>& hostCollidablesA,
	const b3AlignedObjectArray<b3Collidable>& hostCollidablesB,
	const b3Vec3& sepNormalWorldSpace,
	i32 maxContactCapacity)
{
	i32 contactIndex = -1;
	b3ConvexPolyhedronData hullA, hullB;

	b3Collidable colA = hostCollidablesA[collidableIndexA];
	hullA = hostConvexDataA[colA.m_shapeIndex];
	//printf("numvertsA = %d\n",hullA.m_numVertices);

	b3Collidable colB = hostCollidablesB[collidableIndexB];
	hullB = hostConvexDataB[colB.m_shapeIndex];
	//printf("numvertsB = %d\n",hullB.m_numVertices);

	b3Float4 contactsOut[D3_MAX_VERTS];
	i32 localContactCapacity = D3_MAX_VERTS;

#ifdef _WIN32
	drx3DAssert(_finite(bodyBuf->at(bodyIndexA).m_pos.x));
	drx3DAssert(_finite(bodyBuf->at(bodyIndexB).m_pos.x));
#endif

	{
		b3Float4 worldVertsB1[D3_MAX_VERTS];
		b3Float4 worldVertsB2[D3_MAX_VERTS];
		i32 capacityWorldVerts = D3_MAX_VERTS;

		b3Float4 hostNormal = b3MakeFloat4(sepNormalWorldSpace.x, sepNormalWorldSpace.y, sepNormalWorldSpace.z, 0.f);
		i32 shapeA = hostCollidablesA[collidableIndexA].m_shapeIndex;
		i32 shapeB = hostCollidablesB[collidableIndexB].m_shapeIndex;

		b3Scalar minDist = -1;
		b3Scalar maxDist = 0.;

		b3Transform trA, trB;
		{
			//D3_PROFILE("b3TransformPoint computation");
			//trA.setIdentity();
			trA.setOrigin(b3MakeVector3(posA.x, posA.y, posA.z));
			trA.setRotation(b3Quat(ornA.x, ornA.y, ornA.z, ornA.w));

			//trB.setIdentity();
			trB.setOrigin(b3MakeVector3(posB.x, posB.y, posB.z));
			trB.setRotation(b3Quat(ornB.x, ornB.y, ornB.z, ornB.w));
		}

		b3Quat trAorn = trA.getRotation();
		b3Quat trBorn = trB.getRotation();

		i32 numContactsOut = b3ClipHullAgainstHull(hostNormal,
												   hostConvexDataA.at(shapeA),
												   hostConvexDataB.at(shapeB),
												   (b3Float4&)trA.getOrigin(), (b3Quat&)trAorn,
												   (b3Float4&)trB.getOrigin(), (b3Quat&)trBorn,
												   worldVertsB1, worldVertsB2, capacityWorldVerts,
												   minDist, maxDist,
												   verticesA, facesA, indicesA,
												   verticesB, facesB, indicesB,

												   contactsOut, localContactCapacity);

		if (numContactsOut > 0)
		{
			D3_PROFILE("overlap");

			b3Float4 normalOnSurfaceB = (b3Float4&)hostNormal;
			//			b3Float4 centerOut;

			b3Int4 contactIdx;
			contactIdx.x = 0;
			contactIdx.y = 1;
			contactIdx.z = 2;
			contactIdx.w = 3;

			i32 numPoints = 0;

			{
				D3_PROFILE("extractManifold");
				numPoints = b3ReduceContacts(contactsOut, numContactsOut, normalOnSurfaceB, &contactIdx);
			}

			drx3DAssert(numPoints);

			if (nContacts < maxContactCapacity)
			{
				contactIndex = nContacts;
				globalContactOut->expand();
				b3Contact4Data& contact = globalContactOut->at(nContacts);
				contact.m_batchIdx = 0;  //i;
				contact.m_bodyAPtrAndSignBit = (bodyBuf->at(bodyIndexA).m_invMass == 0) ? -bodyIndexA : bodyIndexA;
				contact.m_bodyBPtrAndSignBit = (bodyBuf->at(bodyIndexB).m_invMass == 0) ? -bodyIndexB : bodyIndexB;

				contact.m_frictionCoeffCmp = 45874;
				contact.m_restituitionCoeffCmp = 0;

				//	float distance = 0.f;
				for (i32 p = 0; p < numPoints; p++)
				{
					contact.m_worldPosB[p] = contactsOut[contactIdx.s[p]];  //check if it is actually on B
					contact.m_worldNormalOnB = normalOnSurfaceB;
				}
				//printf("bodyIndexA %d,bodyIndexB %d,normal=%f,%f,%f numPoints %d\n",bodyIndexA,bodyIndexB,normalOnSurfaceB.x,normalOnSurfaceB.y,normalOnSurfaceB.z,numPoints);
				contact.m_worldNormalOnB.w = (b3Scalar)numPoints;
				nContacts++;
			}
			else
			{
				drx3DError("Ошибка: exceeding contact capacity (%d/%d)\n", nContacts, maxContactCapacity);
			}
		}
	}
	return contactIndex;
}

inline i32 b3ContactConvexConvexSAT(
	i32 pairIndex,
	i32 bodyIndexA, i32 bodyIndexB,
	i32 collidableIndexA, i32 collidableIndexB,
	const b3AlignedObjectArray<b3RigidBodyData>& rigidBodies,
	const b3AlignedObjectArray<b3Collidable>& collidables,
	const b3AlignedObjectArray<b3ConvexPolyhedronData>& convexShapes,
	const b3AlignedObjectArray<b3Float4>& convexVertices,
	const b3AlignedObjectArray<b3Float4>& uniqueEdges,
	const b3AlignedObjectArray<i32>& convexIndices,
	const b3AlignedObjectArray<b3GpuFace>& faces,
	b3AlignedObjectArray<b3Contact4Data>& globalContactsOut,
	i32& nGlobalContactsOut,
	i32 maxContactCapacity)
{
	i32 contactIndex = -1;

	b3Float4 posA = rigidBodies[bodyIndexA].m_pos;
	b3Quat ornA = rigidBodies[bodyIndexA].m_quat;
	b3Float4 posB = rigidBodies[bodyIndexB].m_pos;
	b3Quat ornB = rigidBodies[bodyIndexB].m_quat;

	b3ConvexPolyhedronData hullA, hullB;

	b3Float4 sepNormalWorldSpace;

	b3Collidable colA = collidables[collidableIndexA];
	hullA = convexShapes[colA.m_shapeIndex];
	//printf("numvertsA = %d\n",hullA.m_numVertices);

	b3Collidable colB = collidables[collidableIndexB];
	hullB = convexShapes[colB.m_shapeIndex];
	//printf("numvertsB = %d\n",hullB.m_numVertices);

#ifdef _WIN32
	drx3DAssert(_finite(rigidBodies[bodyIndexA].m_pos.x));
	drx3DAssert(_finite(rigidBodies[bodyIndexB].m_pos.x));
#endif

	bool foundSepAxis = b3FindSeparatingAxis(hullA, hullB,
											 posA,
											 ornA,
											 posB,
											 ornB,

											 convexVertices, uniqueEdges, faces, convexIndices,
											 convexVertices, uniqueEdges, faces, convexIndices,

											 sepNormalWorldSpace);

	if (foundSepAxis)
	{
		contactIndex = b3ClipHullHullSingle(
			bodyIndexA, bodyIndexB,
			posA, ornA,
			posB, ornB,
			collidableIndexA, collidableIndexB,
			&rigidBodies,
			&globalContactsOut,
			nGlobalContactsOut,

			convexShapes,
			convexShapes,

			convexVertices,
			uniqueEdges,
			faces,
			convexIndices,

			convexVertices,
			uniqueEdges,
			faces,
			convexIndices,

			collidables,
			collidables,
			sepNormalWorldSpace,
			maxContactCapacity);
	}

	return contactIndex;
}

#endif  //D3_CONTACT_CONVEX_CONVEX_SAT_H
