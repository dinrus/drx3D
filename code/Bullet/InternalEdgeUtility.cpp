#include <drx3D/Physics/Collision/Dispatch/InternalEdgeUtility.h>

#include <drx3D/Physics/Collision/Shapes/BvhTriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/HeightfieldTerrainShape.h>

#include <drx3D/Physics/Collision/Shapes/ScaledBvhTriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/TriangleShape.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/NarrowPhase/ManifoldPoint.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2Wrapper.h>

//#define DEBUG_INTERNAL_EDGE

#ifdef DEBUG_INTERNAL_EDGE
#include <stdio.h>
#endif  //DEBUG_INTERNAL_EDGE

#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
static IDebugDraw* gDebugDrawer = 0;

void SetDebugDrawer(IDebugDraw* debugDrawer)
{
	gDebugDrawer = debugDrawer;
}

static void DebugDrawLine(const Vec3& from, const Vec3& to, const Vec3& color)
{
	if (gDebugDrawer)
		gDebugDrawer->drawLine(from, to, color);
}
#endif  //DRX3D_INTERNAL_EDGE_DEBUG_DRAW

static i32 GetHash(i32 partId, i32 triangleIndex)
{
	i32 hash = (partId << (31 - MAX_NUM_PARTS_IN_BITS)) | triangleIndex;
	return hash;
}

static Scalar GetAngle(const Vec3& edgeA, const Vec3& normalA, const Vec3& normalB)
{
	const Vec3 refAxis0 = edgeA;
	const Vec3 refAxis1 = normalA;
	const Vec3 swingAxis = normalB;
	Scalar angle = Atan2(swingAxis.dot(refAxis0), swingAxis.dot(refAxis1));
	return angle;
}

struct ConnectivityProcessor : public TriangleCallback
{
	i32 m_partIdA;
	i32 m_triangleIndexA;
	Vec3* m_triangleVerticesA;
	TriangleInfoMap* m_triangleInfoMap;

	virtual void processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex)
	{
		//skip self-collisions
		if ((m_partIdA == partId) && (m_triangleIndexA == triangleIndex))
			return;

		//skip duplicates (disabled for now)
		//if ((m_partIdA <= partId) && (m_triangleIndexA <= triangleIndex))
		//	return;

		//search for shared vertices and edges
		i32 numshared = 0;
		i32 sharedVertsA[3] = {-1, -1, -1};
		i32 sharedVertsB[3] = {-1, -1, -1};

		///skip degenerate triangles
		Scalar crossBSqr = ((triangle[1] - triangle[0]).cross(triangle[2] - triangle[0])).length2();
		if (crossBSqr < m_triangleInfoMap->m_equalVertexThreshold)
			return;

		Scalar crossASqr = ((m_triangleVerticesA[1] - m_triangleVerticesA[0]).cross(m_triangleVerticesA[2] - m_triangleVerticesA[0])).length2();
		///skip degenerate triangles
		if (crossASqr < m_triangleInfoMap->m_equalVertexThreshold)
			return;

#if 0
		printf("triangle A[0]	=	(%f,%f,%f)\ntriangle A[1]	=	(%f,%f,%f)\ntriangle A[2]	=	(%f,%f,%f)\n",
			m_triangleVerticesA[0].getX(),m_triangleVerticesA[0].getY(),m_triangleVerticesA[0].getZ(),
			m_triangleVerticesA[1].getX(),m_triangleVerticesA[1].getY(),m_triangleVerticesA[1].getZ(),
			m_triangleVerticesA[2].getX(),m_triangleVerticesA[2].getY(),m_triangleVerticesA[2].getZ());

		printf("partId=%d, triangleIndex=%d\n",partId,triangleIndex);
		printf("triangle B[0]	=	(%f,%f,%f)\ntriangle B[1]	=	(%f,%f,%f)\ntriangle B[2]	=	(%f,%f,%f)\n",
			triangle[0].getX(),triangle[0].getY(),triangle[0].getZ(),
			triangle[1].getX(),triangle[1].getY(),triangle[1].getZ(),
			triangle[2].getX(),triangle[2].getY(),triangle[2].getZ());
#endif

		for (i32 i = 0; i < 3; i++)
		{
			for (i32 j = 0; j < 3; j++)
			{
				if ((m_triangleVerticesA[i] - triangle[j]).length2() < m_triangleInfoMap->m_equalVertexThreshold)
				{
					sharedVertsA[numshared] = i;
					sharedVertsB[numshared] = j;
					numshared++;
					///degenerate case
					if (numshared >= 3)
						return;
				}
			}
			///degenerate case
			if (numshared >= 3)
				return;
		}
		switch (numshared)
		{
			case 0:
			{
				break;
			}
			case 1:
			{
				//shared vertex
				break;
			}
			case 2:
			{
				//shared edge
				//we need to make sure the edge is in the order V2V0 and not V0V2 so that the signs are correct
				if (sharedVertsA[0] == 0 && sharedVertsA[1] == 2)
				{
					sharedVertsA[0] = 2;
					sharedVertsA[1] = 0;
					i32 tmp = sharedVertsB[1];
					sharedVertsB[1] = sharedVertsB[0];
					sharedVertsB[0] = tmp;
				}

				i32 hash = GetHash(m_partIdA, m_triangleIndexA);

				TriangleInfo* info = m_triangleInfoMap->find(hash);
				if (!info)
				{
					TriangleInfo tmp;
					m_triangleInfoMap->insert(hash, tmp);
					info = m_triangleInfoMap->find(hash);
				}

				i32 sumvertsA = sharedVertsA[0] + sharedVertsA[1];
				i32 otherIndexA = 3 - sumvertsA;

				Vec3 edge(m_triangleVerticesA[sharedVertsA[1]] - m_triangleVerticesA[sharedVertsA[0]]);

				TriangleShape tA(m_triangleVerticesA[0], m_triangleVerticesA[1], m_triangleVerticesA[2]);
				i32 otherIndexB = 3 - (sharedVertsB[0] + sharedVertsB[1]);

				TriangleShape tB(triangle[sharedVertsB[1]], triangle[sharedVertsB[0]], triangle[otherIndexB]);
				//TriangleShape tB(triangle[0],triangle[1],triangle[2]);

				Vec3 normalA;
				Vec3 normalB;
				tA.calcNormal(normalA);
				tB.calcNormal(normalB);
				edge.normalize();
				Vec3 edgeCrossA = edge.cross(normalA).normalize();

				{
					Vec3 tmp = m_triangleVerticesA[otherIndexA] - m_triangleVerticesA[sharedVertsA[0]];
					if (edgeCrossA.dot(tmp) < 0)
					{
						edgeCrossA *= -1;
					}
				}

				Vec3 edgeCrossB = edge.cross(normalB).normalize();

				{
					Vec3 tmp = triangle[otherIndexB] - triangle[sharedVertsB[0]];
					if (edgeCrossB.dot(tmp) < 0)
					{
						edgeCrossB *= -1;
					}
				}

				Scalar angle2 = 0;
				Scalar ang4 = 0.f;

				Vec3 calculatedEdge = edgeCrossA.cross(edgeCrossB);
				Scalar len2 = calculatedEdge.length2();

				Scalar correctedAngle(0);
				//Vec3 calculatedNormalB = normalA;
				bool isConvex = false;

				if (len2 < m_triangleInfoMap->m_planarEpsilon)
				{
					angle2 = 0.f;
					ang4 = 0.f;
				}
				else
				{
					calculatedEdge.normalize();
					Vec3 calculatedNormalA = calculatedEdge.cross(edgeCrossA);
					calculatedNormalA.normalize();
					angle2 = GetAngle(calculatedNormalA, edgeCrossA, edgeCrossB);
					ang4 = SIMD_PI - angle2;
					Scalar dotA = normalA.dot(edgeCrossB);
					///@todo: check if we need some epsilon, due to floating point imprecision
					isConvex = (dotA < 0.);

					correctedAngle = isConvex ? ang4 : -ang4;
				}

				//alternatively use
				//Vec3 calculatedNormalB2 = quatRotate(orn,normalA);

				switch (sumvertsA)
				{
					case 1:
					{
						Vec3 edge = m_triangleVerticesA[0] - m_triangleVerticesA[1];
						Quat orn(edge, -correctedAngle);
						Vec3 computedNormalB = quatRotate(orn, normalA);
						Scalar bla = computedNormalB.dot(normalB);
						if (bla < 0)
						{
							computedNormalB *= -1;
							info->m_flags |= TRI_INFO_V0V1_SWAP_NORMALB;
						}
#ifdef DEBUG_INTERNAL_EDGE
						if ((computedNormalB - normalB).length() > 0.0001)
						{
							printf("warning: normals not identical\n");
						}
#endif  //DEBUG_INTERNAL_EDGE

						info->m_edgeV0V1Angle = -correctedAngle;

						if (isConvex)
							info->m_flags |= TRI_INFO_V0V1_CONVEX;
						break;
					}
					case 2:
					{
						Vec3 edge = m_triangleVerticesA[2] - m_triangleVerticesA[0];
						Quat orn(edge, -correctedAngle);
						Vec3 computedNormalB = quatRotate(orn, normalA);
						if (computedNormalB.dot(normalB) < 0)
						{
							computedNormalB *= -1;
							info->m_flags |= TRI_INFO_V2V0_SWAP_NORMALB;
						}

#ifdef DEBUG_INTERNAL_EDGE
						if ((computedNormalB - normalB).length() > 0.0001)
						{
							printf("warning: normals not identical\n");
						}
#endif  //DEBUG_INTERNAL_EDGE
						info->m_edgeV2V0Angle = -correctedAngle;
						if (isConvex)
							info->m_flags |= TRI_INFO_V2V0_CONVEX;
						break;
					}
					case 3:
					{
						Vec3 edge = m_triangleVerticesA[1] - m_triangleVerticesA[2];
						Quat orn(edge, -correctedAngle);
						Vec3 computedNormalB = quatRotate(orn, normalA);
						if (computedNormalB.dot(normalB) < 0)
						{
							info->m_flags |= TRI_INFO_V1V2_SWAP_NORMALB;
							computedNormalB *= -1;
						}
#ifdef DEBUG_INTERNAL_EDGE
						if ((computedNormalB - normalB).length() > 0.0001)
						{
							printf("warning: normals not identical\n");
						}
#endif  //DEBUG_INTERNAL_EDGE
						info->m_edgeV1V2Angle = -correctedAngle;

						if (isConvex)
							info->m_flags |= TRI_INFO_V1V2_CONVEX;
						break;
					}
				}

				break;
			}
			default:
			{
				//				printf("warning: duplicate triangle\n");
			}
		}
	}
};


struct b3ProcessAllTrianglesHeightfield: public TriangleCallback
{
	HeightfieldTerrainShape* m_heightfieldShape;
	TriangleInfoMap* m_triangleInfoMap;
	

	b3ProcessAllTrianglesHeightfield(HeightfieldTerrainShape* heightFieldShape, TriangleInfoMap* triangleInfoMap)
		:m_heightfieldShape(heightFieldShape),
		m_triangleInfoMap(triangleInfoMap)
	{
	}
	virtual void processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex)
	{
		ConnectivityProcessor connectivityProcessor;
		connectivityProcessor.m_partIdA = partId;
		connectivityProcessor.m_triangleIndexA = triangleIndex;
		connectivityProcessor.m_triangleVerticesA = triangle;
		connectivityProcessor.m_triangleInfoMap = m_triangleInfoMap;
		Vec3 aabbMin, aabbMax;
		aabbMin.setVal(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT));
		aabbMax.setVal(Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT));
		aabbMin.setMin(triangle[0]);
		aabbMax.setMax(triangle[0]);
		aabbMin.setMin(triangle[1]);
		aabbMax.setMax(triangle[1]);
		aabbMin.setMin(triangle[2]);
		aabbMax.setMax(triangle[2]);

		m_heightfieldShape->processAllTriangles(&connectivityProcessor, aabbMin, aabbMax);
	}
};
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

void GenerateInternalEdgeInfo(BvhTriangleMeshShape* trimeshShape, TriangleInfoMap* triangleInfoMap)
{
	//the user pointer shouldn't already be used for other purposes, we intend to store connectivity info there!
	if (trimeshShape->getTriangleInfoMap())
		return;

	trimeshShape->setTriangleInfoMap(triangleInfoMap);

	StridingMeshInterface* meshInterface = trimeshShape->getMeshInterface();
	const Vec3& meshScaling = meshInterface->getScaling();

	for (i32 partId = 0; partId < meshInterface->getNumSubParts(); partId++)
	{
		u8k* vertexbase = 0;
		i32 numverts = 0;
		PHY_ScalarType type = PHY_INTEGER;
		i32 stride = 0;
		u8k* indexbase = 0;
		i32 indexstride = 0;
		i32 numfaces = 0;
		PHY_ScalarType indicestype = PHY_INTEGER;
		//PHY_ScalarType indexType=0;

		Vec3 triangleVerts[3];
		meshInterface->getLockedReadOnlyVertexIndexBase(&vertexbase, numverts, type, stride, &indexbase, indexstride, numfaces, indicestype, partId);
		Vec3 aabbMin, aabbMax;

		for (i32 triangleIndex = 0; triangleIndex < numfaces; triangleIndex++)
		{
			u32* gfxbase = (u32*)(indexbase + triangleIndex * indexstride);

			for (i32 j = 2; j >= 0; j--)
			{
				i32 graphicsindex;
                                switch (indicestype) {
                                        case PHY_INTEGER: graphicsindex = gfxbase[j]; break;
                                        case PHY_SHORT: graphicsindex = ((unsigned short*)gfxbase)[j]; break;
                                        case PHY_UCHAR: graphicsindex = ((u8*)gfxbase)[j]; break;
                                        default: Assert(0);
                                }
				if (type == PHY_FLOAT)
				{
					float* graphicsbase = (float*)(vertexbase + graphicsindex * stride);
					triangleVerts[j] = Vec3(
						graphicsbase[0] * meshScaling.getX(),
						graphicsbase[1] * meshScaling.getY(),
						graphicsbase[2] * meshScaling.getZ());
				}
				else
				{
					double* graphicsbase = (double*)(vertexbase + graphicsindex * stride);
					triangleVerts[j] = Vec3(Scalar(graphicsbase[0] * meshScaling.getX()), Scalar(graphicsbase[1] * meshScaling.getY()), Scalar(graphicsbase[2] * meshScaling.getZ()));
				}
			}
			aabbMin.setVal(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT));
			aabbMax.setVal(Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT));
			aabbMin.setMin(triangleVerts[0]);
			aabbMax.setMax(triangleVerts[0]);
			aabbMin.setMin(triangleVerts[1]);
			aabbMax.setMax(triangleVerts[1]);
			aabbMin.setMin(triangleVerts[2]);
			aabbMax.setMax(triangleVerts[2]);

			ConnectivityProcessor connectivityProcessor;
			connectivityProcessor.m_partIdA = partId;
			connectivityProcessor.m_triangleIndexA = triangleIndex;
			connectivityProcessor.m_triangleVerticesA = &triangleVerts[0];
			connectivityProcessor.m_triangleInfoMap = triangleInfoMap;

			trimeshShape->processAllTriangles(&connectivityProcessor, aabbMin, aabbMax);
		}
	}
}


void GenerateInternalEdgeInfo(HeightfieldTerrainShape* heightfieldShape, TriangleInfoMap* triangleInfoMap)
{

	//the user pointer shouldn't already be used for other purposes, we intend to store connectivity info there!
	if (heightfieldShape->getTriangleInfoMap())
		return;

	heightfieldShape->setTriangleInfoMap(triangleInfoMap);

	//get all the triangles of the heightfield

	Vec3 aabbMin, aabbMax;

	aabbMax.setVal(Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT), Scalar(DRX3D_LARGE_FLOAT));
	aabbMin.setVal(Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT), Scalar(-DRX3D_LARGE_FLOAT));

	b3ProcessAllTrianglesHeightfield processHeightfield(heightfieldShape, triangleInfoMap);
	heightfieldShape->processAllTriangles(&processHeightfield, aabbMin, aabbMax);

}

// Given a point and a line segment (defined by two points), compute the closest point
// in the line.  Cap the point at the endpoints of the line segment.
void NearestPointInLineSegment(const Vec3& point, const Vec3& line0, const Vec3& line1, Vec3& nearestPoint)
{
	Vec3 lineDelta = line1 - line0;

	// Handle degenerate lines
	if (lineDelta.fuzzyZero())
	{
		nearestPoint = line0;
	}
	else
	{
		Scalar delta = (point - line0).dot(lineDelta) / (lineDelta).dot(lineDelta);

		// Clamp the point to conform to the segment's endpoints
		if (delta < 0)
			delta = 0;
		else if (delta > 1)
			delta = 1;

		nearestPoint = line0 + lineDelta * delta;
	}
}

bool ClampNormal(const Vec3& edge, const Vec3& tri_normal_org, const Vec3& localContactNormalOnB, Scalar correctedEdgeAngle, Vec3& clampedLocalNormal)
{
	Vec3 tri_normal = tri_normal_org;
	//we only have a local triangle normal, not a local contact normal -> only normal in world space...
	//either compute the current angle all in local space, or all in world space

	Vec3 edgeCross = edge.cross(tri_normal).normalize();
	Scalar curAngle = GetAngle(edgeCross, tri_normal, localContactNormalOnB);

	if (correctedEdgeAngle < 0)
	{
		if (curAngle < correctedEdgeAngle)
		{
			Scalar diffAngle = correctedEdgeAngle - curAngle;
			Quat rotation(edge, diffAngle);
			clampedLocalNormal = Matrix3x3(rotation) * localContactNormalOnB;
			return true;
		}
	}

	if (correctedEdgeAngle >= 0)
	{
		if (curAngle > correctedEdgeAngle)
		{
			Scalar diffAngle = correctedEdgeAngle - curAngle;
			Quat rotation(edge, diffAngle);
			clampedLocalNormal = Matrix3x3(rotation) * localContactNormalOnB;
			return true;
		}
	}
	return false;
}

/// Changes a ManifoldPoint collision normal to the normal from the mesh.
void AdjustInternalEdgeContacts(ManifoldPoint& cp, const CollisionObject2Wrapper* colObj0Wrap, const CollisionObject2Wrapper* colObj1Wrap, i32 partId0, i32 index0, i32 normalAdjustFlags)
{
	//Assert(colObj0->getCollisionShape()->getShapeType() == TRIANGLE_SHAPE_PROXYTYPE);
	if (colObj0Wrap->getCollisionShape()->getShapeType() != TRIANGLE_SHAPE_PROXYTYPE)
		return;

	
	TriangleInfoMap* triangleInfoMapPtr = 0;

	if (colObj0Wrap->getCollisionObject()->getCollisionShape()->getShapeType() == TERRAIN_SHAPE_PROXYTYPE)
	{
		HeightfieldTerrainShape* heightfield = (HeightfieldTerrainShape*)colObj0Wrap->getCollisionObject()->getCollisionShape();
		triangleInfoMapPtr = heightfield->getTriangleInfoMap();

//#define USE_HEIGHTFIELD_TRIANGLES
#ifdef USE_HEIGHTFIELD_TRIANGLES
		Vec3 newNormal = Vec3(0, 0, 1);

		const TriangleShape* tri_shape = static_cast<const TriangleShape*>(colObj0Wrap->getCollisionShape());
		Vec3 tri_normal;
		tri_shape->calcNormal(tri_normal);
		newNormal = tri_normal;
		//					cp.m_distance1 = cp.m_distance1 * newNormal.dot(cp.m_normalWorldOnB);
		cp.m_normalWorldOnB = newNormal;
		// Reproject collision point along normal. (what about cp.m_distance1?)
		cp.m_positionWorldOnB = cp.m_positionWorldOnA - cp.m_normalWorldOnB * cp.m_distance1;
		cp.m_localPointB = colObj0Wrap->getWorldTransform().invXform(cp.m_positionWorldOnB);
		return;
#endif
	}


	BvhTriangleMeshShape* trimesh = 0;

	if (colObj0Wrap->getCollisionObject()->getCollisionShape()->getShapeType() == SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE)
	{
		trimesh = ((ScaledBvhTriangleMeshShape*)colObj0Wrap->getCollisionObject()->getCollisionShape())->getChildShape();
	}
	else
	{
		if (colObj0Wrap->getCollisionObject()->getCollisionShape()->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
		{
			trimesh = (BvhTriangleMeshShape*)colObj0Wrap->getCollisionObject()->getCollisionShape();
		}
	}
	if (trimesh)
	{
		triangleInfoMapPtr = (TriangleInfoMap*)trimesh->getTriangleInfoMap();
	}
	
	
	if (!triangleInfoMapPtr)
		return;

	i32 hash = GetHash(partId0, index0);

	TriangleInfo* info = triangleInfoMapPtr->find(hash);
	if (!info)
		return;

	Scalar frontFacing = (normalAdjustFlags & DRX3D_TRIANGLE_CONVEX_BACKFACE_MODE) == 0 ? 1.f : -1.f;

	const TriangleShape* tri_shape = static_cast<const TriangleShape*>(colObj0Wrap->getCollisionShape());
	Vec3 v0, v1, v2;
	tri_shape->getVertex(0, v0);
	tri_shape->getVertex(1, v1);
	tri_shape->getVertex(2, v2);

	//Vec3 center = (v0+v1+v2)*Scalar(1./3.);

	Vec3 red(1, 0, 0), green(0, 1, 0), blue(0, 0, 1), white(1, 1, 1), black(0, 0, 0);
	Vec3 tri_normal;
	tri_shape->calcNormal(tri_normal);

	//Scalar dot = tri_normal.dot(cp.m_normalWorldOnB);
	Vec3 nearest;
	NearestPointInLineSegment(cp.m_localPointB, v0, v1, nearest);

	Vec3 contact = cp.m_localPointB;
#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
	const Transform2& tr = colObj0->getWorldTransform();
	DebugDrawLine(tr * nearest, tr * cp.m_localPointB, red);
#endif  //DRX3D_INTERNAL_EDGE_DEBUG_DRAW

	bool isNearEdge = false;

	i32 numConcaveEdgeHits = 0;
	i32 numConvexEdgeHits = 0;

	Vec3 localContactNormalOnB = colObj0Wrap->getWorldTransform().getBasis().transpose() * cp.m_normalWorldOnB;
	localContactNormalOnB.normalize();  //is this necessary?

	// Get closest edge
	i32 bestedge = -1;
	Scalar disttobestedge = DRX3D_LARGE_FLOAT;
	//
	// Edge 0 -> 1
	if (Fabs(info->m_edgeV0V1Angle) < triangleInfoMapPtr->m_maxEdgeAngleThreshold)
	{
		Vec3 nearest;
		NearestPointInLineSegment(cp.m_localPointB, v0, v1, nearest);
		Scalar len = (contact - nearest).length();
		//
		if (len < disttobestedge)
		{
			bestedge = 0;
			disttobestedge = len;
		}
	}
	// Edge 1 -> 2
	if (Fabs(info->m_edgeV1V2Angle) < triangleInfoMapPtr->m_maxEdgeAngleThreshold)
	{
		Vec3 nearest;
		NearestPointInLineSegment(cp.m_localPointB, v1, v2, nearest);
		Scalar len = (contact - nearest).length();
		//
		if (len < disttobestedge)
		{
			bestedge = 1;
			disttobestedge = len;
		}
	}
	// Edge 2 -> 0
	if (Fabs(info->m_edgeV2V0Angle) < triangleInfoMapPtr->m_maxEdgeAngleThreshold)
	{
		Vec3 nearest;
		NearestPointInLineSegment(cp.m_localPointB, v2, v0, nearest);
		Scalar len = (contact - nearest).length();
		//
		if (len < disttobestedge)
		{
			bestedge = 2;
			disttobestedge = len;
		}
	}

#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
	Vec3 upfix = tri_normal * Vec3(0.1f, 0.1f, 0.1f);
	DebugDrawLine(tr * v0 + upfix, tr * v1 + upfix, red);
#endif
	if (Fabs(info->m_edgeV0V1Angle) < triangleInfoMapPtr->m_maxEdgeAngleThreshold)
	{
#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
		DebugDrawLine(tr * contact, tr * (contact + cp.m_normalWorldOnB * 10), black);
#endif
		Scalar len = (contact - nearest).length();
		if (len < triangleInfoMapPtr->m_edgeDistanceThreshold)
			if (bestedge == 0)
			{
				Vec3 edge(v0 - v1);
				isNearEdge = true;

				if (info->m_edgeV0V1Angle == Scalar(0))
				{
					numConcaveEdgeHits++;
				}
				else
				{
					bool isEdgeConvex = (info->m_flags & TRI_INFO_V0V1_CONVEX);
					Scalar swapFactor = isEdgeConvex ? Scalar(1) : Scalar(-1);
#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
					DebugDrawLine(tr * nearest, tr * (nearest + swapFactor * tri_normal * 10), white);
#endif  //DRX3D_INTERNAL_EDGE_DEBUG_DRAW

					Vec3 nA = swapFactor * tri_normal;

					Quat orn(edge, info->m_edgeV0V1Angle);
					Vec3 computedNormalB = quatRotate(orn, tri_normal);
					if (info->m_flags & TRI_INFO_V0V1_SWAP_NORMALB)
						computedNormalB *= -1;
					Vec3 nB = swapFactor * computedNormalB;

					Scalar NdotA = localContactNormalOnB.dot(nA);
					Scalar NdotB = localContactNormalOnB.dot(nB);
					bool backFacingNormal = (NdotA < triangleInfoMapPtr->m_convexEpsilon) && (NdotB < triangleInfoMapPtr->m_convexEpsilon);

#ifdef DEBUG_INTERNAL_EDGE
					{
						DebugDrawLine(cp.getPositionWorldOnB(), cp.getPositionWorldOnB() + tr.getBasis() * (nB * 20), red);
					}
#endif  //DEBUG_INTERNAL_EDGE

					if (backFacingNormal)
					{
						numConcaveEdgeHits++;
					}
					else
					{
						numConvexEdgeHits++;
						Vec3 clampedLocalNormal;
						bool isClamped = ClampNormal(edge, swapFactor * tri_normal, localContactNormalOnB, info->m_edgeV0V1Angle, clampedLocalNormal);
						if (isClamped)
						{
							if (((normalAdjustFlags & DRX3D_TRIANGLE_CONVEX_DOUBLE_SIDED) != 0) || (clampedLocalNormal.dot(frontFacing * tri_normal) > 0))
							{
								Vec3 newNormal = colObj0Wrap->getWorldTransform().getBasis() * clampedLocalNormal;
								//					cp.m_distance1 = cp.m_distance1 * newNormal.dot(cp.m_normalWorldOnB);
								cp.m_normalWorldOnB = newNormal;
								// Reproject collision point along normal. (what about cp.m_distance1?)
								cp.m_positionWorldOnB = cp.m_positionWorldOnA - cp.m_normalWorldOnB * cp.m_distance1;
								cp.m_localPointB = colObj0Wrap->getWorldTransform().invXform(cp.m_positionWorldOnB);
							}
						}
					}
				}
			}
	}

	NearestPointInLineSegment(contact, v1, v2, nearest);
#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
	DebugDrawLine(tr * nearest, tr * cp.m_localPointB, green);
#endif  //DRX3D_INTERNAL_EDGE_DEBUG_DRAW

#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
	DebugDrawLine(tr * v1 + upfix, tr * v2 + upfix, green);
#endif

	if (Fabs(info->m_edgeV1V2Angle) < triangleInfoMapPtr->m_maxEdgeAngleThreshold)
	{
#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
		DebugDrawLine(tr * contact, tr * (contact + cp.m_normalWorldOnB * 10), black);
#endif  //DRX3D_INTERNAL_EDGE_DEBUG_DRAW

		Scalar len = (contact - nearest).length();
		if (len < triangleInfoMapPtr->m_edgeDistanceThreshold)
			if (bestedge == 1)
			{
				isNearEdge = true;
#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
				DebugDrawLine(tr * nearest, tr * (nearest + tri_normal * 10), white);
#endif  //DRX3D_INTERNAL_EDGE_DEBUG_DRAW

				Vec3 edge(v1 - v2);

				isNearEdge = true;

				if (info->m_edgeV1V2Angle == Scalar(0))
				{
					numConcaveEdgeHits++;
				}
				else
				{
					bool isEdgeConvex = (info->m_flags & TRI_INFO_V1V2_CONVEX) != 0;
					Scalar swapFactor = isEdgeConvex ? Scalar(1) : Scalar(-1);
#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
					DebugDrawLine(tr * nearest, tr * (nearest + swapFactor * tri_normal * 10), white);
#endif  //DRX3D_INTERNAL_EDGE_DEBUG_DRAW

					Vec3 nA = swapFactor * tri_normal;

					Quat orn(edge, info->m_edgeV1V2Angle);
					Vec3 computedNormalB = quatRotate(orn, tri_normal);
					if (info->m_flags & TRI_INFO_V1V2_SWAP_NORMALB)
						computedNormalB *= -1;
					Vec3 nB = swapFactor * computedNormalB;

#ifdef DEBUG_INTERNAL_EDGE
					{
						DebugDrawLine(cp.getPositionWorldOnB(), cp.getPositionWorldOnB() + tr.getBasis() * (nB * 20), red);
					}
#endif  //DEBUG_INTERNAL_EDGE

					Scalar NdotA = localContactNormalOnB.dot(nA);
					Scalar NdotB = localContactNormalOnB.dot(nB);
					bool backFacingNormal = (NdotA < triangleInfoMapPtr->m_convexEpsilon) && (NdotB < triangleInfoMapPtr->m_convexEpsilon);

					if (backFacingNormal)
					{
						numConcaveEdgeHits++;
					}
					else
					{
						numConvexEdgeHits++;
						Vec3 localContactNormalOnB = colObj0Wrap->getWorldTransform().getBasis().transpose() * cp.m_normalWorldOnB;
						Vec3 clampedLocalNormal;
						bool isClamped = ClampNormal(edge, swapFactor * tri_normal, localContactNormalOnB, info->m_edgeV1V2Angle, clampedLocalNormal);
						if (isClamped)
						{
							if (((normalAdjustFlags & DRX3D_TRIANGLE_CONVEX_DOUBLE_SIDED) != 0) || (clampedLocalNormal.dot(frontFacing * tri_normal) > 0))
							{
								Vec3 newNormal = colObj0Wrap->getWorldTransform().getBasis() * clampedLocalNormal;
								//					cp.m_distance1 = cp.m_distance1 * newNormal.dot(cp.m_normalWorldOnB);
								cp.m_normalWorldOnB = newNormal;
								// Reproject collision point along normal.
								cp.m_positionWorldOnB = cp.m_positionWorldOnA - cp.m_normalWorldOnB * cp.m_distance1;
								cp.m_localPointB = colObj0Wrap->getWorldTransform().invXform(cp.m_positionWorldOnB);
							}
						}
					}
				}
			}
	}

	NearestPointInLineSegment(contact, v2, v0, nearest);
#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
	DebugDrawLine(tr * nearest, tr * cp.m_localPointB, blue);
#endif  //DRX3D_INTERNAL_EDGE_DEBUG_DRAW
#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
	DebugDrawLine(tr * v2 + upfix, tr * v0 + upfix, blue);
#endif

	if (Fabs(info->m_edgeV2V0Angle) < triangleInfoMapPtr->m_maxEdgeAngleThreshold)
	{
#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
		DebugDrawLine(tr * contact, tr * (contact + cp.m_normalWorldOnB * 10), black);
#endif  //DRX3D_INTERNAL_EDGE_DEBUG_DRAW

		Scalar len = (contact - nearest).length();
		if (len < triangleInfoMapPtr->m_edgeDistanceThreshold)
			if (bestedge == 2)
			{
				isNearEdge = true;
#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
				DebugDrawLine(tr * nearest, tr * (nearest + tri_normal * 10), white);
#endif  //DRX3D_INTERNAL_EDGE_DEBUG_DRAW

				Vec3 edge(v2 - v0);

				if (info->m_edgeV2V0Angle == Scalar(0))
				{
					numConcaveEdgeHits++;
				}
				else
				{
					bool isEdgeConvex = (info->m_flags & TRI_INFO_V2V0_CONVEX) != 0;
					Scalar swapFactor = isEdgeConvex ? Scalar(1) : Scalar(-1);
#ifdef DRX3D_INTERNAL_EDGE_DEBUG_DRAW
					DebugDrawLine(tr * nearest, tr * (nearest + swapFactor * tri_normal * 10), white);
#endif  //DRX3D_INTERNAL_EDGE_DEBUG_DRAW

					Vec3 nA = swapFactor * tri_normal;
					Quat orn(edge, info->m_edgeV2V0Angle);
					Vec3 computedNormalB = quatRotate(orn, tri_normal);
					if (info->m_flags & TRI_INFO_V2V0_SWAP_NORMALB)
						computedNormalB *= -1;
					Vec3 nB = swapFactor * computedNormalB;

#ifdef DEBUG_INTERNAL_EDGE
					{
						DebugDrawLine(cp.getPositionWorldOnB(), cp.getPositionWorldOnB() + tr.getBasis() * (nB * 20), red);
					}
#endif  //DEBUG_INTERNAL_EDGE

					Scalar NdotA = localContactNormalOnB.dot(nA);
					Scalar NdotB = localContactNormalOnB.dot(nB);
					bool backFacingNormal = (NdotA < triangleInfoMapPtr->m_convexEpsilon) && (NdotB < triangleInfoMapPtr->m_convexEpsilon);

					if (backFacingNormal)
					{
						numConcaveEdgeHits++;
					}
					else
					{
						numConvexEdgeHits++;
						//				printf("hitting convex edge\n");

						Vec3 localContactNormalOnB = colObj0Wrap->getWorldTransform().getBasis().transpose() * cp.m_normalWorldOnB;
						Vec3 clampedLocalNormal;
						bool isClamped = ClampNormal(edge, swapFactor * tri_normal, localContactNormalOnB, info->m_edgeV2V0Angle, clampedLocalNormal);
						if (isClamped)
						{
							if (((normalAdjustFlags & DRX3D_TRIANGLE_CONVEX_DOUBLE_SIDED) != 0) || (clampedLocalNormal.dot(frontFacing * tri_normal) > 0))
							{
								Vec3 newNormal = colObj0Wrap->getWorldTransform().getBasis() * clampedLocalNormal;
								//					cp.m_distance1 = cp.m_distance1 * newNormal.dot(cp.m_normalWorldOnB);
								cp.m_normalWorldOnB = newNormal;
								// Reproject collision point along normal.
								cp.m_positionWorldOnB = cp.m_positionWorldOnA - cp.m_normalWorldOnB * cp.m_distance1;
								cp.m_localPointB = colObj0Wrap->getWorldTransform().invXform(cp.m_positionWorldOnB);
							}
						}
					}
				}
			}
	}

#ifdef DEBUG_INTERNAL_EDGE
	{
		Vec3 color(0, 1, 1);
		DebugDrawLine(cp.getPositionWorldOnB(), cp.getPositionWorldOnB() + cp.m_normalWorldOnB * 10, color);
	}
#endif  //DEBUG_INTERNAL_EDGE

	if (isNearEdge)
	{
		if (numConcaveEdgeHits > 0)
		{
			if ((normalAdjustFlags & DRX3D_TRIANGLE_CONCAVE_DOUBLE_SIDED) != 0)
			{
				//fix tri_normal so it pointing the same direction as the current local contact normal
				if (tri_normal.dot(localContactNormalOnB) < 0)
				{
					tri_normal *= -1;
				}
				cp.m_normalWorldOnB = colObj0Wrap->getWorldTransform().getBasis() * tri_normal;
			}
			else
			{
				Vec3 newNormal = tri_normal * frontFacing;
				//if the tri_normal is pointing opposite direction as the current local contact normal, skip it
				Scalar d = newNormal.dot(localContactNormalOnB);
				if (d < 0)
				{
					return;
				}
				//modify the normal to be the triangle normal (or backfacing normal)
				cp.m_normalWorldOnB = colObj0Wrap->getWorldTransform().getBasis() * newNormal;
			}

			// Reproject collision point along normal.
			cp.m_positionWorldOnB = cp.m_positionWorldOnA - cp.m_normalWorldOnB * cp.m_distance1;
			cp.m_localPointB = colObj0Wrap->getWorldTransform().invXform(cp.m_positionWorldOnB);
		}
	}
}
