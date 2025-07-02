
#ifdef _WIN32  //needed for glut.h
#include <windows.h>
#endif

#include <drx3D/OpenGLWindow/OpenGL2Include.h>

#include "../GL_ShapeDrawer.h"
#include <drx3D/Physics/Collision/Shapes/PolyhedralConvexShape.h>
#include <drx3D/Physics/Collision/Shapes/TriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/BoxShape.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>
#include <drx3D/Physics/Collision/Shapes/ConeShape.h>
#include <drx3D/Physics/Collision/Shapes/CylinderShape.h>
#include <drx3D/Physics/Collision/Shapes/TetrahedronShape.h>
#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/Physics/Collision/Shapes/CapsuleShape.h>
#include <drx3D/Physics/Collision/Shapes/ConvexTriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/UniformScalingShape.h>
#include <drx3D/Physics/Collision/Shapes/StaticPlaneShape.h>
#include <drx3D/Physics/Collision/Shapes/MultiSphereShape.h>
#include <drx3D/Physics/Collision/Shapes/ConvexPolyhedron.h>
#include <drx3D/Physics/Dynamics/DiscreteDynamicsWorld.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>
#include <drx3D/Maths/Linear/DefaultMotionState.h>

///
#include <drx3D/Physics/Collision/Shapes/ShapeHull.h>

#include <drx3D/Maths/Linear/Transform2Util.h>

#include <drx3D/Maths/Linear/IDebugDraw.h>
//for debugmodes

#include <stdio.h>  //printf debugging

#include <cmath>

#if defined(DRX3D_USE_DOUBLE_PRECISION)
#define btglLoadMatrix glLoadMatrixd
#define btglMultMatrix glMultMatrixd
#define btglColor3 glColor3d
#define btglVertex3 glVertex3d
#else
#define btglLoadMatrix glLoadMatrixf
#define btglMultMatrix glMultMatrixf
#define btglColor3 glColor3f
#define btglVertex3 glVertex3d
#endif

void GL_ShapeDrawer::drawCoordSystem()
{
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(1, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 1, 0);
	glColor3f(0, 0, 1);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, 1);
	glEnd();
}

class GlDrawcallback : public TriangleCallback
{
public:
	bool m_wireframe;

	GlDrawcallback()
		: m_wireframe(false)
	{
	}

	virtual void processTriangle(Vec3* triangle, i32 partId, i32 triangleIndex)
	{
		(void)triangleIndex;
		(void)partId;

		if (m_wireframe)
		{
			glBegin(GL_LINES);
			glColor3f(1, 0, 0);
			glVertex3d(triangle[0].getX(), triangle[0].getY(), triangle[0].getZ());
			glVertex3d(triangle[1].getX(), triangle[1].getY(), triangle[1].getZ());
			glColor3f(0, 1, 0);
			glVertex3d(triangle[2].getX(), triangle[2].getY(), triangle[2].getZ());
			glVertex3d(triangle[1].getX(), triangle[1].getY(), triangle[1].getZ());
			glColor3f(0, 0, 1);
			glVertex3d(triangle[2].getX(), triangle[2].getY(), triangle[2].getZ());
			glVertex3d(triangle[0].getX(), triangle[0].getY(), triangle[0].getZ());
			glEnd();
		}
		else
		{
			glBegin(GL_TRIANGLES);
			//glColor3f(1, 1, 1);

			glVertex3d(triangle[0].getX(), triangle[0].getY(), triangle[0].getZ());
			glVertex3d(triangle[1].getX(), triangle[1].getY(), triangle[1].getZ());
			glVertex3d(triangle[2].getX(), triangle[2].getY(), triangle[2].getZ());

			glVertex3d(triangle[2].getX(), triangle[2].getY(), triangle[2].getZ());
			glVertex3d(triangle[1].getX(), triangle[1].getY(), triangle[1].getZ());
			glVertex3d(triangle[0].getX(), triangle[0].getY(), triangle[0].getZ());
			glEnd();
		}
	}
};

class TriangleGlDrawcallback : public InternalTriangleIndexCallback
{
public:
	virtual void internalProcessTriangleIndex(Vec3* triangle, i32 partId, i32 triangleIndex)
	{
		(void)triangleIndex;
		(void)partId;

		glBegin(GL_TRIANGLES);  //LINES);
		glColor3f(1, 0, 0);
		glVertex3d(triangle[0].getX(), triangle[0].getY(), triangle[0].getZ());
		glVertex3d(triangle[1].getX(), triangle[1].getY(), triangle[1].getZ());
		glColor3f(0, 1, 0);
		glVertex3d(triangle[2].getX(), triangle[2].getY(), triangle[2].getZ());
		glVertex3d(triangle[1].getX(), triangle[1].getY(), triangle[1].getZ());
		glColor3f(0, 0, 1);
		glVertex3d(triangle[2].getX(), triangle[2].getY(), triangle[2].getZ());
		glVertex3d(triangle[0].getX(), triangle[0].getY(), triangle[0].getZ());
		glEnd();
	}
};

void GL_ShapeDrawer::drawSphere(Scalar radius, i32 lats, i32 longs)
{
	i32 i, j;
	for (i = 0; i <= lats; i++)
	{
		Scalar lat0 = SIMD_PI * (-Scalar(0.5) + (Scalar)(i - 1) / lats);
		Scalar z0 = radius * std::sin(lat0);
		Scalar zr0 = radius * std::cos(lat0);

		Scalar lat1 = SIMD_PI * (-Scalar(0.5) + (Scalar)i / lats);
		Scalar z1 = radius * std::sin(lat1);
		Scalar zr1 = radius * std::cos(lat1);

		glBegin(GL_QUAD_STRIP);
		for (j = 0; j <= longs; j++)
		{
			Scalar lng = 2 * SIMD_PI * (Scalar)(j - 1) / longs;
			Scalar x = std::cos(lng);
			Scalar y = std::sin(lng);
			glNormal3f(x * zr1, y * zr1, z1);
			glVertex3f(x * zr1, y * zr1, z1);
			glNormal3f(x * zr0, y * zr0, z0);
			glVertex3f(x * zr0, y * zr0, z0);
		}
		glEnd();
	}
}

GL_ShapeDrawer::ShapeCache* GL_ShapeDrawer::cache(ConvexShape* shape)
{
	ShapeCache* sc = (ShapeCache*)shape->getUserPointer();
	if (!sc)
	{
		sc = new (AlignedAlloc(sizeof(ShapeCache), 16)) ShapeCache(shape);
		sc->m_shapehull.buildHull(shape->getMargin());
		m_shapecaches.push_back(sc);
		shape->setUserPointer(sc);
		/* Build edges	*/
		i32k ni = sc->m_shapehull.numIndices();
		i32k nv = sc->m_shapehull.numVertices();
		u32k* pi = sc->m_shapehull.getIndexPointer();
		const Vec3* pv = sc->m_shapehull.getVertexPointer();
		AlignedObjectArray<ShapeCache::Edge*> edges;
		sc->m_edges.reserve(ni);
		edges.resize(nv * nv, 0);
		for (i32 i = 0; i < ni; i += 3)
		{
			u32k* ti = pi + i;
			const Vec3 nrm = Cross(pv[ti[1]] - pv[ti[0]], pv[ti[2]] - pv[ti[0]]).normalized();
			for (i32 j = 2, k = 0; k < 3; j = k++)
			{
				u32k a = ti[j];
				u32k b = ti[k];
				ShapeCache::Edge*& e = edges[d3Min(a, b) * nv + d3Max(a, b)];
				if (!e)
				{
					sc->m_edges.push_back(ShapeCache::Edge());
					e = &sc->m_edges[sc->m_edges.size() - 1];
					e->n[0] = nrm;
					e->n[1] = -nrm;
					e->v[0] = a;
					e->v[1] = b;
				}
				else
				{
					e->n[1] = nrm;
				}
			}
		}
	}
	return (sc);
}

void renderSquareA(float x, float y, float z)
{
	glBegin(GL_LINE_LOOP);
	glVertex3f(x, y, z);
	glVertex3f(x + 10.f, y, z);
	glVertex3f(x + 10.f, y + 10.f, z);
	glVertex3f(x, y + 10.f, z);
	glEnd();
}

inline void glDrawVector(const Vec3& v) { glVertex3d(v[0], v[1], v[2]); }

void GL_ShapeDrawer::drawOpenGL(Scalar* m, const CollisionShape* shape, const Vec3& color, i32 debugMode, const Vec3& worldBoundsMin, const Vec3& worldBoundsMax)
{
	if (shape->getShapeType() == CUSTOM_CONVEX_SHAPE_TYPE)
	{
		Vec3 org(m[12], m[13], m[14]);
		Vec3 dx(m[0], m[1], m[2]);
		Vec3 dy(m[4], m[5], m[6]);
		//		Vec3 dz(m[8], m[9], m[10]);
		const BoxShape* boxShape = static_cast<const BoxShape*>(shape);
		Vec3 halfExtent = boxShape->getHalfExtentsWithMargin();
		dx *= halfExtent[0];
		dy *= halfExtent[1];
		//		dz *= halfExtent[2];
		glColor3f(1, 1, 1);
		glDisable(GL_LIGHTING);
		glLineWidth(2);

		glBegin(GL_LINE_LOOP);
		glDrawVector(org - dx - dy);
		glDrawVector(org - dx + dy);
		glDrawVector(org + dx + dy);
		glDrawVector(org + dx - dy);
		glEnd();
		return;
	}
	else if ((shape->getShapeType() == BOX_SHAPE_PROXYTYPE) && (debugMode & IDebugDraw::DBG_FastWireframe))
	{
		Vec3 org(m[12], m[13], m[14]);
		Vec3 dx(m[0], m[1], m[2]);
		Vec3 dy(m[4], m[5], m[6]);
		Vec3 dz(m[8], m[9], m[10]);
		const BoxShape* boxShape = static_cast<const BoxShape*>(shape);
		Vec3 halfExtent = boxShape->getHalfExtentsWithMargin();
		dx *= halfExtent[0];
		dy *= halfExtent[1];
		dz *= halfExtent[2];
		glBegin(GL_LINE_LOOP);
		glDrawVector(org - dx - dy - dz);
		glDrawVector(org + dx - dy - dz);
		glDrawVector(org + dx + dy - dz);
		glDrawVector(org - dx + dy - dz);
		glDrawVector(org - dx + dy + dz);
		glDrawVector(org + dx + dy + dz);
		glDrawVector(org + dx - dy + dz);
		glDrawVector(org - dx - dy + dz);
		glEnd();
		glBegin(GL_LINES);
		glDrawVector(org + dx - dy - dz);
		glDrawVector(org + dx - dy + dz);
		glDrawVector(org + dx + dy - dz);
		glDrawVector(org + dx + dy + dz);
		glDrawVector(org - dx - dy - dz);
		glDrawVector(org - dx + dy - dz);
		glDrawVector(org - dx - dy + dz);
		glDrawVector(org - dx + dy + dz);
		glEnd();
		return;
	}

	glPushMatrix();
	btglMultMatrix(m);

	if (shape->getShapeType() == UNIFORM_SCALING_SHAPE_PROXYTYPE)
	{
		const UniformScalingShape* scalingShape = static_cast<const UniformScalingShape*>(shape);
		const ConvexShape* convexShape = scalingShape->getChildShape();
		float scalingFactor = (float)scalingShape->getUniformScalingFactor();
		{
			Scalar tmpScaling[4][4] = {{scalingFactor, 0, 0, 0},
										 {0, scalingFactor, 0, 0},
										 {0, 0, scalingFactor, 0},
										 {0, 0, 0, 1}};

			drawOpenGL((Scalar*)tmpScaling, convexShape, color, debugMode, worldBoundsMin, worldBoundsMax);
		}
		glPopMatrix();
		return;
	}

	if (shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
	{
		const CompoundShape* compoundShape = static_cast<const CompoundShape*>(shape);
		for (i32 i = compoundShape->getNumChildShapes() - 1; i >= 0; i--)
		{
			Transform2 childTrans = compoundShape->getChildTransform(i);
			const CollisionShape* colShape = compoundShape->getChildShape(i);
			ATTRIBUTE_ALIGNED16(Scalar)
			childMat[16];
			childTrans.getOpenGLMatrix(childMat);
			drawOpenGL(childMat, colShape, color, debugMode, worldBoundsMin, worldBoundsMax);
		}
	}
	else
	{
		if (m_textureenabled && (!m_textureinitialized))
		{
			GLubyte* image = new GLubyte[256 * 256 * 4];
			for (i32 y = 0; y < 256; ++y)
			{
				i32k t = y >> 4;
				GLubyte* pi = image + y * 256 * 3;
				for (i32 x = 0; x < 256; ++x)
				{
					i32k s = x >> 4;
					const GLubyte b = 180;
					GLubyte c = b + ((s + (t & 1)) & 1) * (255 - b);
					pi[0] = pi[1] = pi[2] = pi[3] = c;
					pi += 3;
				}
			}

			glGenTextures(1, (GLuint*)&m_texturehandle);
			glBindTexture(GL_TEXTURE_2D, m_texturehandle);

			glGenTextures(1, (GLuint*)&m_texturehandle);
			glBindTexture(GL_TEXTURE_2D, m_texturehandle);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
			//glGenerateMipmap(GL_TEXTURE_2D);
			delete[] image;
		}

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glScalef(0.025f, 0.025f, 0.025f);
		glMatrixMode(GL_MODELVIEW);

		static const GLfloat planex[] = {1, 0, 0, 0};
		//	static const GLfloat	planey[]={0,1,0,0};
		static const GLfloat planez[] = {0, 0, 1, 0};
		glTexGenfv(GL_S, GL_OBJECT_PLANE, planex);
		glTexGenfv(GL_T, GL_OBJECT_PLANE, planez);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		m_textureinitialized = true;

		//drawCoordSystem();

		//glPushMatrix();
		glEnable(GL_COLOR_MATERIAL);
		if (m_textureenabled)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, m_texturehandle);
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
		}

		glColor3f(color.x(), color.y(), color.z());

		//bool useWireframeFallback = true;

		if (!(debugMode & IDebugDraw::DBG_DrawWireframe))
		{
			///you can comment out any of the specific cases, and use the default

			///the benefit of 'default' is that it approximates the actual collision shape including collision margin
			//i32 shapetype=m_textureenabled?MAX_BROADPHASE_COLLISION_TYPES:shape->getShapeType();
			i32 shapetype = shape->getShapeType();
			switch (shapetype)
			{
				case SPHERE_SHAPE_PROXYTYPE:
				{
					const SphereShape* sphereShape = static_cast<const SphereShape*>(shape);
					float radius = sphereShape->getMargin();  //radius doesn't include the margin, so draw with margin
					drawSphere(radius, 10, 10);
					//useWireframeFallback = false;
					break;
				}

				case BOX_SHAPE_PROXYTYPE:
				{
					const BoxShape* boxShape = static_cast<const BoxShape*>(shape);
					Vec3 halfExtent = boxShape->getHalfExtentsWithMargin();

					static i32 indices[36] = {
						0, 1, 2,
						3, 2, 1,
						4, 0, 6,
						6, 0, 2,
						5, 1, 4,
						4, 1, 0,
						7, 3, 1,
						7, 1, 5,
						5, 4, 7,
						7, 4, 6,
						7, 2, 3,
						7, 6, 2};

					Vec3 vertices[8] = {
						Vec3(halfExtent[0], halfExtent[1], halfExtent[2]),
						Vec3(-halfExtent[0], halfExtent[1], halfExtent[2]),
						Vec3(halfExtent[0], -halfExtent[1], halfExtent[2]),
						Vec3(-halfExtent[0], -halfExtent[1], halfExtent[2]),
						Vec3(halfExtent[0], halfExtent[1], -halfExtent[2]),
						Vec3(-halfExtent[0], halfExtent[1], -halfExtent[2]),
						Vec3(halfExtent[0], -halfExtent[1], -halfExtent[2]),
						Vec3(-halfExtent[0], -halfExtent[1], -halfExtent[2])};
#if 1
					glBegin(GL_TRIANGLES);
					i32 si = 36;
					for (i32 i = 0; i < si; i += 3)
					{
						const Vec3& v1 = vertices[indices[i]];
						;
						const Vec3& v2 = vertices[indices[i + 1]];
						const Vec3& v3 = vertices[indices[i + 2]];
						Vec3 normal = (v3 - v1).cross(v2 - v1);
						normal.normalize();
						glNormal3f(normal.getX(), normal.getY(), normal.getZ());
						glVertex3f(v1.x(), v1.y(), v1.z());
						glVertex3f(v2.x(), v2.y(), v2.z());
						glVertex3f(v3.x(), v3.y(), v3.z());
					}
					glEnd();
#endif

					//useWireframeFallback = false;
					break;
				}

#if 0
			
			case CONE_SHAPE_PROXYTYPE:
				{
					const ConeShape* coneShape = static_cast<const ConeShape*>(shape);
					i32 upIndex = coneShape->getConeUpIndex();
					float radius = coneShape->getRadius();//+coneShape->getMargin();
					float height = coneShape->getHeight();//+coneShape->getMargin();
					switch (upIndex)
					{
					case 0:
						glRotatef(90.0, 0.0, 1.0, 0.0);
						break;
					case 1:
						glRotatef(-90.0, 1.0, 0.0, 0.0);
						break;
					case 2:
						break;
					default:
						{
						}
					};

					glTranslatef(0.0, 0.0, -0.5*height);
					glutSolidCone(radius,height,10,10);
					//useWireframeFallback = false;
					break;

				}
#endif

				case STATIC_PLANE_PROXYTYPE:
				{
					const StaticPlaneShape* staticPlaneShape = static_cast<const StaticPlaneShape*>(shape);
					Scalar planeConst = staticPlaneShape->getPlaneConstant();
					const Vec3& planeNormal = staticPlaneShape->getPlaneNormal();
					Vec3 planeOrigin = planeNormal * planeConst;
					Vec3 vec0, vec1;
					PlaneSpace1(planeNormal, vec0, vec1);
					Scalar vecLen = 100.f;
					Vec3 pt0 = planeOrigin + vec0 * vecLen;
					Vec3 pt1 = planeOrigin - vec0 * vecLen;
					Vec3 pt2 = planeOrigin + vec1 * vecLen;
					Vec3 pt3 = planeOrigin - vec1 * vecLen;
					glBegin(GL_LINES);
					glVertex3f(pt0.getX(), pt0.getY(), pt0.getZ());
					glVertex3f(pt1.getX(), pt1.getY(), pt1.getZ());
					glVertex3f(pt2.getX(), pt2.getY(), pt2.getZ());
					glVertex3f(pt3.getX(), pt3.getY(), pt3.getZ());
					glEnd();

					break;
				}

				case MULTI_SPHERE_SHAPE_PROXYTYPE:
				{
					const MultiSphereShape* multiSphereShape = static_cast<const MultiSphereShape*>(shape);

					Transform2 childTransform;
					childTransform.setIdentity();

					for (i32 i = multiSphereShape->getSphereCount() - 1; i >= 0; i--)
					{
						SphereShape sc(multiSphereShape->getSphereRadius(i));
						childTransform.setOrigin(multiSphereShape->getSpherePosition(i));
						ATTRIBUTE_ALIGNED16(Scalar)
						childMat[16];
						childTransform.getOpenGLMatrix(childMat);
						drawOpenGL(childMat, &sc, color, debugMode, worldBoundsMin, worldBoundsMax);
					}

					break;
				}

				default:
				{
					if (shape->isConvex())
					{
						const ConvexPolyhedron* poly = shape->isPolyhedral() ? ((PolyhedralConvexShape*)shape)->getConvexPolyhedron() : 0;
						if (poly)
						{
							i32 i;
							glBegin(GL_TRIANGLES);
							for (i = 0; i < poly->m_faces.size(); i++)
							{
								Vec3 centroid(0, 0, 0);
								i32 numVerts = poly->m_faces[i].m_indices.size();
								if (numVerts > 2)
								{
									Vec3 v1 = poly->m_vertices[poly->m_faces[i].m_indices[0]];
									for (i32 v = 0; v < poly->m_faces[i].m_indices.size() - 2; v++)
									{
										Vec3 v2 = poly->m_vertices[poly->m_faces[i].m_indices[v + 1]];
										Vec3 v3 = poly->m_vertices[poly->m_faces[i].m_indices[v + 2]];
										Vec3 normal = (v3 - v1).cross(v2 - v1);
										normal.normalize();
										glNormal3f(normal.getX(), normal.getY(), normal.getZ());
										glVertex3f(v1.x(), v1.y(), v1.z());
										glVertex3f(v2.x(), v2.y(), v2.z());
										glVertex3f(v3.x(), v3.y(), v3.z());
									}
								}
							}
							glEnd();
						}
						else
						{
							ShapeCache* sc = cache((ConvexShape*)shape);
							//glutSolidCube(1.0);
							ShapeHull* hull = &sc->m_shapehull /*(ShapeHull*)shape->getUserPointer()*/;

							if (hull->numTriangles() > 0)
							{
								i32 index = 0;
								u32k* idx = hull->getIndexPointer();
								const Vec3* vtx = hull->getVertexPointer();

								glBegin(GL_TRIANGLES);

								for (i32 i = 0; i < hull->numTriangles(); i++)
								{
									i32 i1 = index++;
									i32 i2 = index++;
									i32 i3 = index++;
									Assert(i1 < hull->numIndices() &&
											 i2 < hull->numIndices() &&
											 i3 < hull->numIndices());

									i32 index1 = idx[i1];
									i32 index2 = idx[i2];
									i32 index3 = idx[i3];
									Assert(index1 < hull->numVertices() &&
											 index2 < hull->numVertices() &&
											 index3 < hull->numVertices());

									Vec3 v1 = vtx[index1];
									Vec3 v2 = vtx[index2];
									Vec3 v3 = vtx[index3];
									Vec3 normal = (v3 - v1).cross(v2 - v1);
									normal.normalize();
									glNormal3f(normal.getX(), normal.getY(), normal.getZ());
									glVertex3f(v1.x(), v1.y(), v1.z());
									glVertex3f(v2.x(), v2.y(), v2.z());
									glVertex3f(v3.x(), v3.y(), v3.z());
								}
								glEnd();
							}
						}
					}
				}
			}
		}

		glNormal3f(0, 1, 0);

		/// for polyhedral shapes
		if (debugMode == IDebugDraw::DBG_DrawFeaturesText && (shape->isPolyhedral()))
		{
			PolyhedralConvexShape* polyshape = (PolyhedralConvexShape*)shape;

			{
				glColor3f(1.f, 1.f, 1.f);
				i32 i;
				for (i = 0; i < polyshape->getNumVertices(); i++)
				{
					Vec3 vtx;
					polyshape->getVertex(i, vtx);
					char buf[12];
					sprintf(buf, " %d", i);
					//DrawString(BMF_GetFont(BMF_kHelvetica10),buf);
				}

				for (i = 0; i < polyshape->getNumPlanes(); i++)
				{
					Vec3 normal;
					Vec3 vtx;
					polyshape->getPlane(normal, vtx, i);
					//Scalar d = vtx.dot(normal);

					//char buf[12];
					//sprintf(buf," plane %d",i);
					//DrawString(BMF_GetFont(BMF_kHelvetica10),buf);
				}
			}
		}
	}
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
}

//
void GL_ShapeDrawer::drawShadow(Scalar* m, const Vec3& extrusion, const CollisionShape* shape, const Vec3& worldBoundsMin, const Vec3& worldBoundsMax)
{
	glPushMatrix();
	btglMultMatrix(m);
	if (shape->getShapeType() == UNIFORM_SCALING_SHAPE_PROXYTYPE)
	{
		const UniformScalingShape* scalingShape = static_cast<const UniformScalingShape*>(shape);
		const ConvexShape* convexShape = scalingShape->getChildShape();
		float scalingFactor = (float)scalingShape->getUniformScalingFactor();
		Scalar tmpScaling[4][4] = {{scalingFactor, 0, 0, 0},
									 {0, scalingFactor, 0, 0},
									 {0, 0, scalingFactor, 0},
									 {0, 0, 0, 1}};
		drawShadow((Scalar*)tmpScaling, extrusion, convexShape, worldBoundsMin, worldBoundsMax);
		glPopMatrix();
		return;
	}
	else if (shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
	{
		const CompoundShape* compoundShape = static_cast<const CompoundShape*>(shape);
		for (i32 i = compoundShape->getNumChildShapes() - 1; i >= 0; i--)
		{
			Transform2 childTrans = compoundShape->getChildTransform(i);
			const CollisionShape* colShape = compoundShape->getChildShape(i);
			ATTRIBUTE_ALIGNED16(Scalar)
			childMat[16];
			childTrans.getOpenGLMatrix(childMat);
			drawShadow(childMat, extrusion * childTrans.getBasis(), colShape, worldBoundsMin, worldBoundsMax);
		}
	}
	else
	{
		//	bool useWireframeFallback = true;
		if (shape->isConvex())
		{
			ShapeCache* sc = cache((ConvexShape*)shape);
			ShapeHull* hull = &sc->m_shapehull;
			glBegin(GL_QUADS);
			for (i32 i = 0; i < sc->m_edges.size(); ++i)
			{
				const Scalar d = Dot(sc->m_edges[i].n[0], extrusion);
				if ((d * Dot(sc->m_edges[i].n[1], extrusion)) < 0)
				{
					i32k q = d < 0 ? 1 : 0;
					const Vec3& a = hull->getVertexPointer()[sc->m_edges[i].v[q]];
					const Vec3& b = hull->getVertexPointer()[sc->m_edges[i].v[1 - q]];
					glVertex3f(a[0], a[1], a[2]);
					glVertex3f(b[0], b[1], b[2]);
					glVertex3f(b[0] + extrusion[0], b[1] + extrusion[1], b[2] + extrusion[2]);
					glVertex3f(a[0] + extrusion[0], a[1] + extrusion[1], a[2] + extrusion[2]);
				}
			}
			glEnd();
		}
	}

	if (shape->isConcave())  //>getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE||shape->getShapeType() == GIMPACT_SHAPE_PROXYTYPE)
							 //		if (shape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
	{
		ConcaveShape* concaveMesh = (ConcaveShape*)shape;

		GlDrawcallback drawCallback;
		drawCallback.m_wireframe = false;

		concaveMesh->processAllTriangles(&drawCallback, worldBoundsMin, worldBoundsMax);
	}
	glPopMatrix();
}

//
GL_ShapeDrawer::GL_ShapeDrawer()
{
	m_texturehandle = 0;
	m_textureenabled = false;
	m_textureinitialized = false;
}

GL_ShapeDrawer::~GL_ShapeDrawer()
{
	i32 i;
	for (i = 0; i < m_shapecaches.size(); i++)
	{
		m_shapecaches[i]->~ShapeCache();
		AlignedFree(m_shapecaches[i]);
	}
	m_shapecaches.clear();
	if (m_textureinitialized)
	{
		glDeleteTextures(1, (const GLuint*)&m_texturehandle);
	}
}

void GL_ShapeDrawer::drawSceneInternal(const DiscreteDynamicsWorld* dynamicsWorld, i32 pass, i32 cameraUpAxis)
{
	Assert(dynamicsWorld);

	Scalar m[16];
	Matrix3x3 rot;
	rot.setIdentity();
	i32k numObjects = dynamicsWorld->getNumCollisionObjects();
	Vec3 wireColor(1, 0, 0);
	//glDisable(GL_CULL_FACE);

	for (i32 i = 0; i < numObjects; i++)
	{
		const CollisionObject2* colObj = dynamicsWorld->getCollisionObjectArray()[i];
		const RigidBody* body = RigidBody::upcast(colObj);
		if (body && body->getMotionState())
		{
			DefaultMotionState* myMotionState = (DefaultMotionState*)body->getMotionState();
			myMotionState->m_graphicsWorldTrans.getOpenGLMatrix(m);
			rot = myMotionState->m_graphicsWorldTrans.getBasis();
		}
		else
		{
			colObj->getWorldTransform().getOpenGLMatrix(m);
			rot = colObj->getWorldTransform().getBasis();
		}
		Vec3 wireColor(1.f, 1.0f, 0.5f);  //wants deactivation
		if (i & 1) wireColor = Vec3(0.f, 0.0f, 1.f);
		///color differently for active, sleeping, wantsdeactivation states
		if (colObj->getActivationState() == 1)  //active
		{
			if (i & 1)
			{
				wireColor += Vec3(1.f, 0.f, 0.f);
			}
			else
			{
				wireColor += Vec3(.5f, 0.f, 0.f);
			}
		}
		if (colObj->getActivationState() == 2)  //ISLAND_SLEEPING
		{
			if (i & 1)
			{
				wireColor += Vec3(0.f, 1.f, 0.f);
			}
			else
			{
				wireColor += Vec3(0.f, 0.5f, 0.f);
			}
		}

		Vec3 aabbMin(0, 0, 0), aabbMax(0, 0, 0);
		//m_dynamicsWorld->getBroadphase()->getBroadphaseAabb(aabbMin,aabbMax);

		aabbMin -= Vec3(DRX3D_LARGE_FLOAT, DRX3D_LARGE_FLOAT, DRX3D_LARGE_FLOAT);
		aabbMax += Vec3(DRX3D_LARGE_FLOAT, DRX3D_LARGE_FLOAT, DRX3D_LARGE_FLOAT);
		//		printf("aabbMin=(%f,%f,%f)\n",aabbMin.getX(),aabbMin.getY(),aabbMin.getZ());
		//		printf("aabbMax=(%f,%f,%f)\n",aabbMax.getX(),aabbMax.getY(),aabbMax.getZ());
		//		m_dynamicsWorld->getDebugDrawer()->drawAabb(aabbMin,aabbMax,Vec3(1,1,1));

		//switch(pass)

		//if (!(getDebugMode()& btIDebugDraw::DBG_DrawWireframe))
		i32 debugMode = 0;  //getDebugMode()
		//Vec3 m_sundirection(-1,-1,-1);

		Vec3 m_sundirection(Vec3(1, -2, 1) * 1000);
		if (cameraUpAxis == 2)
		{
			m_sundirection = Vec3(1, 1, -2) * 1000;
		}

		switch (pass)
		{
			case 0:
				drawOpenGL(m, colObj->getCollisionShape(), wireColor, debugMode, aabbMin, aabbMax);
				break;
			case 1:
				drawShadow(m, m_sundirection * rot, colObj->getCollisionShape(), aabbMin, aabbMax);
				break;
			case 2:
				drawOpenGL(m, colObj->getCollisionShape(), wireColor * Scalar(0.3), 0, aabbMin, aabbMax);
				break;
		}
	}
}

//this GL_ShapeDrawer will be removed, in the meanwhile directly access this global 'useShadoMaps'
extern bool useShadowMap;
void GL_ShapeDrawer::drawScene(const DiscreteDynamicsWorld* dynamicsWorld, bool useShadows1, i32 cameraUpAxis)
{
	bool useShadows = useShadowMap;
	GLfloat light_ambient[] = {Scalar(0.2), Scalar(0.2), Scalar(0.2), Scalar(1.0)};
	GLfloat light_diffuse[] = {Scalar(1.0), Scalar(1.0), Scalar(1.0), Scalar(1.0)};
	GLfloat light_specular[] = {Scalar(1.0), Scalar(1.0), Scalar(1.0), Scalar(1.0)};
	/*	light_position is NOT default value	*/
	GLfloat light_position0[] = {Scalar(1.0), Scalar(10.0), Scalar(1.0), Scalar(0.0)};
	GLfloat light_position1[] = {Scalar(-1.0), Scalar(-10.0), Scalar(-1.0), Scalar(0.0)};

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);

	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClearColor(Scalar(0.7), Scalar(0.7), Scalar(0.7), Scalar(0));

	if (useShadows)
	{
		glClear(GL_STENCIL_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		drawSceneInternal(dynamicsWorld, 0, cameraUpAxis);

		glDisable(GL_LIGHTING);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_STENCIL_TEST);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFFL);
		glFrontFace(GL_CCW);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		drawSceneInternal(dynamicsWorld, 1, cameraUpAxis);
		glFrontFace(GL_CW);
		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
		drawSceneInternal(dynamicsWorld, 1, cameraUpAxis);
		glFrontFace(GL_CCW);

		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
		glShadeModel(GL_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_LIGHTING);
		glDepthMask(GL_TRUE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glEnable(GL_CULL_FACE);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glDepthFunc(GL_LEQUAL);
		glStencilFunc(GL_NOTEQUAL, 0, 0xFFFFFFFFL);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glDisable(GL_LIGHTING);
		drawSceneInternal(dynamicsWorld, 2, cameraUpAxis);
		glEnable(GL_LIGHTING);
		glDepthFunc(GL_LESS);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
		drawSceneInternal(dynamicsWorld, 0, cameraUpAxis);
	}
}
