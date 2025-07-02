
#include <drx3D/Physics/Collision/Shapes/TriangleMesh.h>

#include <drx3D/CollisionCommon.h>
#include <drx3D/Physics/Collision/Shapes/ShapeHull.h>  //to create a tesselation of a generic ConvexShape
#include <drx3D/Physics/Collision/Shapes/ConvexPolyhedron.h>

void CollisionShape2TriangleMesh(CollisionShape* collisionShape, const Transform2& parentTransform, AlignedObjectArray<Vec3>& vertexPositions, AlignedObjectArray<Vec3>& vertexNormals, AlignedObjectArray<i32>& indicesOut)

{
	//todo: support all collision shape types
	switch (collisionShape->getShapeType())
	{
		case SOFTBODY_SHAPE_PROXYTYPE:
		{
			//skip the soft body collision shape for now
			break;
		}
		case STATIC_PLANE_PROXYTYPE:
		{
			//draw a box, oriented along the plane normal
			const StaticPlaneShape* staticPlaneShape = static_cast<const StaticPlaneShape*>(collisionShape);
			Scalar planeConst = staticPlaneShape->getPlaneConstant();
			const Vec3& planeNormal = staticPlaneShape->getPlaneNormal();
			Vec3 planeOrigin = planeNormal * planeConst;
			Vec3 vec0, vec1;
			PlaneSpace1(planeNormal, vec0, vec1);
			Scalar vecLen = 100.f;
			Vec3 verts[4];

			verts[0] = planeOrigin + vec0 * vecLen + vec1 * vecLen;
			verts[1] = planeOrigin - vec0 * vecLen + vec1 * vecLen;
			verts[2] = planeOrigin - vec0 * vecLen - vec1 * vecLen;
			verts[3] = planeOrigin + vec0 * vecLen - vec1 * vecLen;

			i32 startIndex = vertexPositions.size();
			indicesOut.push_back(startIndex + 0);
			indicesOut.push_back(startIndex + 1);
			indicesOut.push_back(startIndex + 2);
			indicesOut.push_back(startIndex + 0);
			indicesOut.push_back(startIndex + 2);
			indicesOut.push_back(startIndex + 3);

			Vec3 triNormal = parentTransform.getBasis() * planeNormal;

			for (i32 i = 0; i < 4; i++)
			{
				Vec3 vtxPos;
				Vec3 pos = parentTransform * verts[i];
				vertexPositions.push_back(pos);
				vertexNormals.push_back(triNormal);
			}
			break;
		}
		case TRIANGLE_MESH_SHAPE_PROXYTYPE:
		{
			BvhTriangleMeshShape* trimesh = (BvhTriangleMeshShape*)collisionShape;
			Vec3 trimeshScaling = trimesh->getLocalScaling();
			StridingMeshInterface* meshInterface = trimesh->getMeshInterface();
			AlignedObjectArray<Vec3> vertices;
			AlignedObjectArray<i32> indices;

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
								graphicsbase[0] * trimeshScaling.getX(),
								graphicsbase[1] * trimeshScaling.getY(),
								graphicsbase[2] * trimeshScaling.getZ());
						}
						else
						{
							double* graphicsbase = (double*)(vertexbase + graphicsindex * stride);
							triangleVerts[j] = Vec3(Scalar(graphicsbase[0] * trimeshScaling.getX()),
														 Scalar(graphicsbase[1] * trimeshScaling.getY()),
														 Scalar(graphicsbase[2] * trimeshScaling.getZ()));
						}
					}
					indices.push_back(vertices.size());
					vertices.push_back(triangleVerts[0]);
					indices.push_back(vertices.size());
					vertices.push_back(triangleVerts[1]);
					indices.push_back(vertices.size());
					vertices.push_back(triangleVerts[2]);

					Vec3 triNormal = (triangleVerts[1] - triangleVerts[0]).cross(triangleVerts[2] - triangleVerts[0]);
					Scalar dot = triNormal.dot(triNormal);

					//cull degenerate triangles
					if (dot >= SIMD_EPSILON * SIMD_EPSILON)
					{
						triNormal /= Sqrt(dot);
						for (i32 v = 0; v < 3; v++)
						{
							Vec3 pos = parentTransform * triangleVerts[v];
							indicesOut.push_back(vertexPositions.size());
							vertexPositions.push_back(pos);
							vertexNormals.push_back(triNormal);
						}
					}
				}
			}

			break;
		}
		default:
		{
			if (collisionShape->isConvex())
			{
				ConvexShape* convex = (ConvexShape*)collisionShape;
				{
					const ConvexPolyhedron* pol = 0;
					if (convex->isPolyhedral())
					{
						PolyhedralConvexShape* poly = (PolyhedralConvexShape*)convex;
						pol = poly->getConvexPolyhedron();
					}

					if (pol)
					{
						i32 baseIndex = vertexPositions.size();
						for (i32 v = 0; v < pol->m_vertices.size(); v++)
						{
							vertexPositions.push_back(pol->m_vertices[v]);
							Vec3 norm = pol->m_vertices[v];
							norm.safeNormalize();
							vertexNormals.push_back(norm);
						}
						for (i32 f = 0; f < pol->m_faces.size(); f++)
						{
							for (i32 ii = 2; ii < pol->m_faces[f].m_indices.size(); ii++)
							{
								indicesOut.push_back(baseIndex+pol->m_faces[f].m_indices[0]);
								indicesOut.push_back(baseIndex + pol->m_faces[f].m_indices[ii - 1]);
								indicesOut.push_back(baseIndex + pol->m_faces[f].m_indices[ii]);
							}
						}
					}
					else
					{
						ShapeHull* hull = new ShapeHull(convex);
						hull->buildHull(0.0, 1);

						{
							//i32 strideInBytes = 9*sizeof(float);
							//i32 numVertices = hull->numVertices();
							//i32 numIndices =hull->numIndices();

							for (i32 t = 0; t < hull->numTriangles(); t++)
							{
								Vec3 triNormal;

								i32 index0 = hull->getIndexPointer()[t * 3 + 0];
								i32 index1 = hull->getIndexPointer()[t * 3 + 1];
								i32 index2 = hull->getIndexPointer()[t * 3 + 2];
								Vec3 pos0 = parentTransform * hull->getVertexPointer()[index0];
								Vec3 pos1 = parentTransform * hull->getVertexPointer()[index1];
								Vec3 pos2 = parentTransform * hull->getVertexPointer()[index2];
								triNormal = (pos1 - pos0).cross(pos2 - pos0);
								triNormal.safeNormalize();

								for (i32 v = 0; v < 3; v++)
								{
									i32 index = hull->getIndexPointer()[t * 3 + v];
									Vec3 pos = parentTransform * hull->getVertexPointer()[index];
									indicesOut.push_back(vertexPositions.size());
									vertexPositions.push_back(pos);
									vertexNormals.push_back(triNormal);
								}
							}
						}
						delete hull;
					}
				}
			}
			else
			{
				if (collisionShape->isCompound())
				{
					CompoundShape* compound = (CompoundShape*)collisionShape;
					for (i32 i = 0; i < compound->getNumChildShapes(); i++)
					{
						Transform2 childWorldTrans = parentTransform * compound->getChildTransform(i);
						CollisionShape2TriangleMesh(compound->getChildShape(i), childWorldTrans, vertexPositions, vertexNormals, indicesOut);
					}
				}
				else
				{
					if (collisionShape->getShapeType() == SDF_SHAPE_PROXYTYPE)
					{
						//not yet
					}
					else
					{
						Assert(0);
					}
				}
			}
		}
	};
}
