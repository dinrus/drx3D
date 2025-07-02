#if defined(_WIN32) || defined(__i386__)
#define DRX3D_USE_SSE_IN_API
#endif

#include <drx3D/Physics/Collision/Shapes/PolyhedralConvexShape.h>
#include <drx3D/Physics/Collision/Shapes/ConvexPolyhedron.h>
#include <drx3D/Maths/Linear/ConvexHullComputer.h>
#include <new>
#include <drx3D/Maths/Linear/GeometryUtil.h>
#include <drx3D/Maths/Linear/GrahamScan2dConvexHull.h>

PolyhedralConvexShape::PolyhedralConvexShape() : ConvexInternalShape(),
													 m_polyhedron(0)
{
}

PolyhedralConvexShape::~PolyhedralConvexShape()
{
	if (m_polyhedron)
	{
		m_polyhedron->~ConvexPolyhedron();
		AlignedFree(m_polyhedron);
	}
}

void PolyhedralConvexShape::setPolyhedralFeatures(ConvexPolyhedron& polyhedron)
{
	if (m_polyhedron)
	{
		*m_polyhedron = polyhedron;
	}
	else
	{
		uk mem = AlignedAlloc(sizeof(ConvexPolyhedron), 16);
		m_polyhedron = new (mem) ConvexPolyhedron(polyhedron);
	}
}

bool PolyhedralConvexShape::initializePolyhedralFeatures(i32 shiftVerticesByMargin)
{
	if (m_polyhedron)
	{
		m_polyhedron->~ConvexPolyhedron();
		AlignedFree(m_polyhedron);
	}

	uk mem = AlignedAlloc(sizeof(ConvexPolyhedron), 16);
	m_polyhedron = new (mem) ConvexPolyhedron;

	AlignedObjectArray<Vec3> orgVertices;

	for (i32 i = 0; i < getNumVertices(); i++)
	{
		Vec3& newVertex = orgVertices.expand();
		getVertex(i, newVertex);
	}

	ConvexHullComputer conv;

	if (shiftVerticesByMargin)
	{
		AlignedObjectArray<Vec3> planeEquations;
		GeometryUtil::getPlaneEquationsFromVertices(orgVertices, planeEquations);

		AlignedObjectArray<Vec3> shiftedPlaneEquations;
		for (i32 p = 0; p < planeEquations.size(); p++)
		{
			Vec3 plane = planeEquations[p];
			//	   Scalar margin = getMargin();
			plane[3] -= getMargin();
			shiftedPlaneEquations.push_back(plane);
		}

		AlignedObjectArray<Vec3> tmpVertices;

		GeometryUtil::getVerticesFromPlaneEquations(shiftedPlaneEquations, tmpVertices);

		conv.compute(&tmpVertices[0].getX(), sizeof(Vec3), tmpVertices.size(), 0.f, 0.f);
	}
	else
	{
		conv.compute(&orgVertices[0].getX(), sizeof(Vec3), orgVertices.size(), 0.f, 0.f);
	}

#ifndef DRX3D_RECONSTRUCT_FACES

	i32 numVertices = conv.vertices.size();
	m_polyhedron->m_vertices.resize(numVertices);
	for (i32 p = 0; p < numVertices; p++)
	{
		m_polyhedron->m_vertices[p] = conv.vertices[p];
	}

	i32 v0, v1;
	for (i32 j = 0; j < conv.faces.size(); j++)
	{
		Vec3 edges[3];
		i32 numEdges = 0;
		Face combinedFace;
		const ConvexHullComputer::Edge* edge = &conv.edges[conv.faces[j]];
		v0 = edge->getSourceVertex();
		i32 prevVertex = v0;
		combinedFace.m_indices.push_back(v0);
		v1 = edge->getTargetVertex();
		while (v1 != v0)
		{
			Vec3 wa = conv.vertices[prevVertex];
			Vec3 wb = conv.vertices[v1];
			Vec3 newEdge = wb - wa;
			newEdge.normalize();
			if (numEdges < 2)
				edges[numEdges++] = newEdge;

			//face->addIndex(v1);
			combinedFace.m_indices.push_back(v1);
			edge = edge->getNextEdgeOfFace();
			prevVertex = v1;
			i32 v01 = edge->getSourceVertex();
			v1 = edge->getTargetVertex();
		}

		Assert(combinedFace.m_indices.size() > 2);

		Vec3 faceNormal = edges[0].cross(edges[1]);
		faceNormal.normalize();

		Scalar planeEq = 1e30f;

		for (i32 v = 0; v < combinedFace.m_indices.size(); v++)
		{
			Scalar eq = m_polyhedron->m_vertices[combinedFace.m_indices[v]].dot(faceNormal);
			if (planeEq > eq)
			{
				planeEq = eq;
			}
		}
		combinedFace.m_plane[0] = faceNormal.getX();
		combinedFace.m_plane[1] = faceNormal.getY();
		combinedFace.m_plane[2] = faceNormal.getZ();
		combinedFace.m_plane[3] = -planeEq;

		m_polyhedron->m_faces.push_back(combinedFace);
	}

#else  //DRX3D_RECONSTRUCT_FACES

	AlignedObjectArray<Vec3> faceNormals;
	i32 numFaces = conv.faces.size();
	faceNormals.resize(numFaces);
	ConvexHullComputer* convexUtil = &conv;

	AlignedObjectArray<btFace> tmpFaces;
	tmpFaces.resize(numFaces);

	i32 numVertices = convexUtil->vertices.size();
	m_polyhedron->m_vertices.resize(numVertices);
	for (i32 p = 0; p < numVertices; p++)
	{
		m_polyhedron->m_vertices[p] = convexUtil->vertices[p];
	}

	for (i32 i = 0; i < numFaces; i++)
	{
		i32 face = convexUtil->faces[i];
		//printf("face=%d\n",face);
		const ConvexHullComputer::Edge* firstEdge = &convexUtil->edges[face];
		const ConvexHullComputer::Edge* edge = firstEdge;

		Vec3 edges[3];
		i32 numEdges = 0;
		//compute face normals

		do
		{
			i32 src = edge->getSourceVertex();
			tmpFaces[i].m_indices.push_back(src);
			i32 targ = edge->getTargetVertex();
			Vec3 wa = convexUtil->vertices[src];

			Vec3 wb = convexUtil->vertices[targ];
			Vec3 newEdge = wb - wa;
			newEdge.normalize();
			if (numEdges < 2)
				edges[numEdges++] = newEdge;

			edge = edge->getNextEdgeOfFace();
		} while (edge != firstEdge);

		Scalar planeEq = 1e30f;

		if (numEdges == 2)
		{
			faceNormals[i] = edges[0].cross(edges[1]);
			faceNormals[i].normalize();
			tmpFaces[i].m_plane[0] = faceNormals[i].getX();
			tmpFaces[i].m_plane[1] = faceNormals[i].getY();
			tmpFaces[i].m_plane[2] = faceNormals[i].getZ();
			tmpFaces[i].m_plane[3] = planeEq;
		}
		else
		{
			Assert(0);  //degenerate?
			faceNormals[i].setZero();
		}

		for (i32 v = 0; v < tmpFaces[i].m_indices.size(); v++)
		{
			Scalar eq = m_polyhedron->m_vertices[tmpFaces[i].m_indices[v]].dot(faceNormals[i]);
			if (planeEq > eq)
			{
				planeEq = eq;
			}
		}
		tmpFaces[i].m_plane[3] = -planeEq;
	}

	//merge coplanar faces and copy them to m_polyhedron

	Scalar faceWeldThreshold = 0.999f;
	AlignedObjectArray<i32> todoFaces;
	for (i32 i = 0; i < tmpFaces.size(); i++)
		todoFaces.push_back(i);

	while (todoFaces.size())
	{
		AlignedObjectArray<i32> coplanarFaceGroup;
		i32 refFace = todoFaces[todoFaces.size() - 1];

		coplanarFaceGroup.push_back(refFace);
		btFace& faceA = tmpFaces[refFace];
		todoFaces.pop_back();

		Vec3 faceNormalA(faceA.m_plane[0], faceA.m_plane[1], faceA.m_plane[2]);
		for (i32 j = todoFaces.size() - 1; j >= 0; j--)
		{
			i32 i = todoFaces[j];
			btFace& faceB = tmpFaces[i];
			Vec3 faceNormalB(faceB.m_plane[0], faceB.m_plane[1], faceB.m_plane[2]);
			if (faceNormalA.dot(faceNormalB) > faceWeldThreshold)
			{
				coplanarFaceGroup.push_back(i);
				todoFaces.remove(i);
			}
		}

		bool did_merge = false;
		if (coplanarFaceGroup.size() > 1)
		{
			//do the merge: use Graham Scan 2d convex hull

			AlignedObjectArray<GrahamVector3> orgpoints;
			Vec3 averageFaceNormal(0, 0, 0);

			for (i32 i = 0; i < coplanarFaceGroup.size(); i++)
			{
				//				m_polyhedron->m_faces.push_back(tmpFaces[coplanarFaceGroup[i]]);

				btFace& face = tmpFaces[coplanarFaceGroup[i]];
				Vec3 faceNormal(face.m_plane[0], face.m_plane[1], face.m_plane[2]);
				averageFaceNormal += faceNormal;
				for (i32 f = 0; f < face.m_indices.size(); f++)
				{
					i32 orgIndex = face.m_indices[f];
					Vec3 pt = m_polyhedron->m_vertices[orgIndex];

					bool found = false;

					for (i32 i = 0; i < orgpoints.size(); i++)
					{
						//if ((orgpoints[i].m_orgIndex == orgIndex) || ((rotatedPt-orgpoints[i]).length2()<0.0001))
						if (orgpoints[i].m_orgIndex == orgIndex)
						{
							found = true;
							break;
						}
					}
					if (!found)
						orgpoints.push_back(GrahamVector3(pt, orgIndex));
				}
			}

			btFace combinedFace;
			for (i32 i = 0; i < 4; i++)
				combinedFace.m_plane[i] = tmpFaces[coplanarFaceGroup[0]].m_plane[i];

			AlignedObjectArray<GrahamVector3> hull;

			averageFaceNormal.normalize();
			GrahamScanConvexHull2D(orgpoints, hull, averageFaceNormal);

			for (i32 i = 0; i < hull.size(); i++)
			{
				combinedFace.m_indices.push_back(hull[i].m_orgIndex);
				for (i32 k = 0; k < orgpoints.size(); k++)
				{
					if (orgpoints[k].m_orgIndex == hull[i].m_orgIndex)
					{
						orgpoints[k].m_orgIndex = -1;  // invalidate...
						break;
					}
				}
			}

			// are there rejected vertices?
			bool reject_merge = false;

			for (i32 i = 0; i < orgpoints.size(); i++)
			{
				if (orgpoints[i].m_orgIndex == -1)
					continue;  // this is in the hull...
				// this vertex is rejected -- is anybody else using this vertex?
				for (i32 j = 0; j < tmpFaces.size(); j++)
				{
					btFace& face = tmpFaces[j];
					// is this a face of the current coplanar group?
					bool is_in_current_group = false;
					for (i32 k = 0; k < coplanarFaceGroup.size(); k++)
					{
						if (coplanarFaceGroup[k] == j)
						{
							is_in_current_group = true;
							break;
						}
					}
					if (is_in_current_group)  // ignore this face...
						continue;
					// does this face use this rejected vertex?
					for (i32 v = 0; v < face.m_indices.size(); v++)
					{
						if (face.m_indices[v] == orgpoints[i].m_orgIndex)
						{
							// this rejected vertex is used in another face -- reject merge
							reject_merge = true;
							break;
						}
					}
					if (reject_merge)
						break;
				}
				if (reject_merge)
					break;
			}

			if (!reject_merge)
			{
				// do this merge!
				did_merge = true;
				m_polyhedron->m_faces.push_back(combinedFace);
			}
		}
		if (!did_merge)
		{
			for (i32 i = 0; i < coplanarFaceGroup.size(); i++)
			{
				btFace face = tmpFaces[coplanarFaceGroup[i]];
				m_polyhedron->m_faces.push_back(face);
			}
		}
	}

#endif  //DRX3D_RECONSTRUCT_FACES

	m_polyhedron->initialize();

	return true;
}

#ifndef MIN
#define MIN(_a, _b) ((_a) < (_b) ? (_a) : (_b))
#endif

Vec3 PolyhedralConvexShape::localGetSupportingVertexWithoutMargin(const Vec3& vec0) const
{
	Vec3 supVec(0, 0, 0);
#ifndef __SPU__
	i32 i;
	Scalar maxDot(Scalar(-DRX3D_LARGE_FLOAT));

	Vec3 vec = vec0;
	Scalar lenSqr = vec.length2();
	if (lenSqr < Scalar(0.0001))
	{
		vec.setVal(1, 0, 0);
	}
	else
	{
		Scalar rlen = Scalar(1.) / Sqrt(lenSqr);
		vec *= rlen;
	}

	Vec3 vtx;
	Scalar newDot;

	for (i32 k = 0; k < getNumVertices(); k += 128)
	{
		Vec3 temp[128];
		i32 inner_count = MIN(getNumVertices() - k, 128);
		for (i = 0; i < inner_count; i++)
			getVertex(i, temp[i]);
		i = (i32)vec.maxDot(temp, inner_count, newDot);
		if (newDot > maxDot)
		{
			maxDot = newDot;
			supVec = temp[i];
		}
	}

#endif  //__SPU__
	return supVec;
}

void PolyhedralConvexShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const Vec3* vectors, Vec3* supportVerticesOut, i32 numVectors) const
{
#ifndef __SPU__
	i32 i;

	Vec3 vtx;
	Scalar newDot;

	for (i = 0; i < numVectors; i++)
	{
		supportVerticesOut[i][3] = Scalar(-DRX3D_LARGE_FLOAT);
	}

	for (i32 j = 0; j < numVectors; j++)
	{
		const Vec3& vec = vectors[j];

		for (i32 k = 0; k < getNumVertices(); k += 128)
		{
			Vec3 temp[128];
			i32 inner_count = MIN(getNumVertices() - k, 128);
			for (i = 0; i < inner_count; i++)
				getVertex(i, temp[i]);
			i = (i32)vec.maxDot(temp, inner_count, newDot);
			if (newDot > supportVerticesOut[j][3])
			{
				supportVerticesOut[j] = temp[i];
				supportVerticesOut[j][3] = newDot;
			}
		}
	}

#endif  //__SPU__
}

void PolyhedralConvexShape::calculateLocalInertia(Scalar mass, Vec3& inertia) const
{
#ifndef __SPU__
	//not yet, return box inertia

	Scalar margin = getMargin();

	Transform2 ident;
	ident.setIdentity();
	Vec3 aabbMin, aabbMax;
	getAabb(ident, aabbMin, aabbMax);
	Vec3 halfExtents = (aabbMax - aabbMin) * Scalar(0.5);

	Scalar lx = Scalar(2.) * (halfExtents.x() + margin);
	Scalar ly = Scalar(2.) * (halfExtents.y() + margin);
	Scalar lz = Scalar(2.) * (halfExtents.z() + margin);
	const Scalar x2 = lx * lx;
	const Scalar y2 = ly * ly;
	const Scalar z2 = lz * lz;
	const Scalar scaledmass = mass * Scalar(0.08333333);

	inertia = scaledmass * (Vec3(y2 + z2, x2 + z2, x2 + y2));
#endif  //__SPU__
}

void PolyhedralConvexAabbCachingShape::setLocalScaling(const Vec3& scaling)
{
	ConvexInternalShape::setLocalScaling(scaling);
	recalcLocalAabb();
}

PolyhedralConvexAabbCachingShape::PolyhedralConvexAabbCachingShape()
	: PolyhedralConvexShape(),
	  m_localAabbMin(1, 1, 1),
	  m_localAabbMax(-1, -1, -1),
	  m_isLocalAabbValid(false)
{
}

void PolyhedralConvexAabbCachingShape::getAabb(const Transform2& trans, Vec3& aabbMin, Vec3& aabbMax) const
{
	getNonvirtualAabb(trans, aabbMin, aabbMax, getMargin());
}

void PolyhedralConvexAabbCachingShape::recalcLocalAabb()
{
	m_isLocalAabbValid = true;

#if 1
	static const Vec3 _directions[] =
		{
			Vec3(1., 0., 0.),
			Vec3(0., 1., 0.),
			Vec3(0., 0., 1.),
			Vec3(-1., 0., 0.),
			Vec3(0., -1., 0.),
			Vec3(0., 0., -1.)};

	Vec3 _supporting[] =
		{
			Vec3(0., 0., 0.),
			Vec3(0., 0., 0.),
			Vec3(0., 0., 0.),
			Vec3(0., 0., 0.),
			Vec3(0., 0., 0.),
			Vec3(0., 0., 0.)};

	batchedUnitVectorGetSupportingVertexWithoutMargin(_directions, _supporting, 6);

	for (i32 i = 0; i < 3; ++i)
	{
		m_localAabbMax[i] = _supporting[i][i] + m_collisionMargin;
		m_localAabbMin[i] = _supporting[i + 3][i] - m_collisionMargin;
	}

#else

	for (i32 i = 0; i < 3; i++)
	{
		Vec3 vec(Scalar(0.), Scalar(0.), Scalar(0.));
		vec[i] = Scalar(1.);
		Vec3 tmp = localGetSupportingVertex(vec);
		m_localAabbMax[i] = tmp[i];
		vec[i] = Scalar(-1.);
		tmp = localGetSupportingVertex(vec);
		m_localAabbMin[i] = tmp[i];
	}
#endif
}
