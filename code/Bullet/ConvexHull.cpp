#include <string.h>

#include <drx3D/Maths/Linear/ConvexHull.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/MinMax.h>
#include <drx3D/Maths/Linear/Vec3.h>

//----------------------------------

class int3
{
public:
	i32 x, y, z;
	int3(){};
	int3(i32 _x, i32 _y, i32 _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
	i32k &operator[](i32 i) const { return (&x)[i]; }
	i32 &operator[](i32 i) { return (&x)[i]; }
};

//------- Plane ----------

inline Plane PlaneFlip(const Plane &plane) { return Plane(-plane.normal, -plane.dist); }
inline i32 operator==(const Plane &a, const Plane &b) { return (a.normal == b.normal && a.dist == b.dist); }
inline i32 coplanar(const Plane &a, const Plane &b) { return (a == b || a == PlaneFlip(b)); }

//--------- Utility Functions ------

Vec3 PlaneLineIntersection(const Plane &plane, const Vec3 &p0, const Vec3 &p1);
Vec3 PlaneProject(const Plane &plane, const Vec3 &point);

Vec3 ThreePlaneIntersection(const Plane &p0, const Plane &p1, const Plane &p2);
Vec3 ThreePlaneIntersection(const Plane &p0, const Plane &p1, const Plane &p2)
{
	Vec3 N1 = p0.normal;
	Vec3 N2 = p1.normal;
	Vec3 N3 = p2.normal;

	Vec3 n2n3;
	n2n3 = N2.cross(N3);
	Vec3 n3n1;
	n3n1 = N3.cross(N1);
	Vec3 n1n2;
	n1n2 = N1.cross(N2);

	Scalar quotient = (N1.dot(n2n3));

	Assert(Fabs(quotient) > Scalar(0.000001));

	quotient = Scalar(-1.) / quotient;
	n2n3 *= p0.dist;
	n3n1 *= p1.dist;
	n1n2 *= p2.dist;
	Vec3 potentialVertex = n2n3;
	potentialVertex += n3n1;
	potentialVertex += n1n2;
	potentialVertex *= quotient;

	Vec3 result(potentialVertex.getX(), potentialVertex.getY(), potentialVertex.getZ());
	return result;
}

Scalar DistanceBetweenLines(const Vec3 &ustart, const Vec3 &udir, const Vec3 &vstart, const Vec3 &vdir, Vec3 *upoint = NULL, Vec3 *vpoint = NULL);
Vec3 TriNormal(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2);
Vec3 NormalOf(const Vec3 *vert, i32k n);

Vec3 PlaneLineIntersection(const Plane &plane, const Vec3 &p0, const Vec3 &p1)
{
	// returns the point where the line p0-p1 intersects the plane n&d
	Vec3 dif;
	dif = p1 - p0;
	Scalar dn = Dot(plane.normal, dif);
	Scalar t = -(plane.dist + Dot(plane.normal, p0)) / dn;
	return p0 + (dif * t);
}

Vec3 PlaneProject(const Plane &plane, const Vec3 &point)
{
	return point - plane.normal * (Dot(point, plane.normal) + plane.dist);
}

Vec3 TriNormal(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2)
{
	// return the normal of the triangle
	// inscribed by v0, v1, and v2
	Vec3 cp = Cross(v1 - v0, v2 - v1);
	Scalar m = cp.length();
	if (m == 0) return Vec3(1, 0, 0);
	return cp * (Scalar(1.0) / m);
}

Scalar DistanceBetweenLines(const Vec3 &ustart, const Vec3 &udir, const Vec3 &vstart, const Vec3 &vdir, Vec3 *upoint, Vec3 *vpoint)
{
	Vec3 cp;
	cp = Cross(udir, vdir).normalized();

	Scalar distu = -Dot(cp, ustart);
	Scalar distv = -Dot(cp, vstart);
	Scalar dist = (Scalar)fabs(distu - distv);
	if (upoint)
	{
		Plane plane;
		plane.normal = Cross(vdir, cp).normalized();
		plane.dist = -Dot(plane.normal, vstart);
		*upoint = PlaneLineIntersection(plane, ustart, ustart + udir);
	}
	if (vpoint)
	{
		Plane plane;
		plane.normal = Cross(udir, cp).normalized();
		plane.dist = -Dot(plane.normal, ustart);
		*vpoint = PlaneLineIntersection(plane, vstart, vstart + vdir);
	}
	return dist;
}

#define COPLANAR (0)
#define UNDER (1)
#define OVER (2)
#define SPLIT (OVER | UNDER)
#define PAPERWIDTH (Scalar(0.001))

Scalar planetestepsilon = PAPERWIDTH;

typedef ConvexH::HalfEdge HalfEdge;

ConvexH::ConvexH(i32 vertices_size, i32 edges_size, i32 facets_size)
{
	vertices.resize(vertices_size);
	edges.resize(edges_size);
	facets.resize(facets_size);
}

i32 PlaneTest(const Plane &p, const Vec3 &v);
i32 PlaneTest(const Plane &p, const Vec3 &v)
{
	Scalar a = Dot(v, p.normal) + p.dist;
	i32 flag = (a > planetestepsilon) ? OVER : ((a < -planetestepsilon) ? UNDER : COPLANAR);
	return flag;
}

i32 SplitTest(ConvexH &convex, const Plane &plane);
i32 SplitTest(ConvexH &convex, const Plane &plane)
{
	i32 flag = 0;
	for (i32 i = 0; i < convex.vertices.size(); i++)
	{
		flag |= PlaneTest(plane, convex.vertices[i]);
	}
	return flag;
}

class VertFlag
{
public:
	u8 planetest;
	u8 junk;
	u8 undermap;
	u8 overmap;
};
class EdgeFlag
{
public:
	u8 planetest;
	u8 fixes;
	short undermap;
	short overmap;
};
class PlaneFlag
{
public:
	u8 undermap;
	u8 overmap;
};
class Coplanar
{
public:
	unsigned short ea;
	u8 v0;
	u8 v1;
};

template <class T>
i32 maxdirfiltered(const T *p, i32 count, const T &dir, AlignedObjectArray<i32> &allow)
{
	Assert(count);
	i32 m = -1;
	for (i32 i = 0; i < count; i++)
		if (allow[i])
		{
			if (m == -1 || Dot(p[i], dir) > Dot(p[m], dir))
				m = i;
		}
	Assert(m != -1);
	return m;
}

Vec3 orth(const Vec3 &v);
Vec3 orth(const Vec3 &v)
{
	Vec3 a = Cross(v, Vec3(0, 0, 1));
	Vec3 b = Cross(v, Vec3(0, 1, 0));
	if (a.length() > b.length())
	{
		return a.normalized();
	}
	else
	{
		return b.normalized();
	}
}

template <class T>
i32 maxdirsterid(const T *p, i32 count, const T &dir, AlignedObjectArray<i32> &allow)
{
	i32 m = -1;
	while (m == -1)
	{
		m = maxdirfiltered(p, count, dir, allow);
		if (allow[m] == 3) return m;
		T u = orth(dir);
		T v = Cross(u, dir);
		i32 ma = -1;
		for (Scalar x = Scalar(0.0); x <= Scalar(360.0); x += Scalar(45.0))
		{
			Scalar s = Sin(SIMD_RADS_PER_DEG * (x));
			Scalar c = Cos(SIMD_RADS_PER_DEG * (x));
			i32 mb = maxdirfiltered(p, count, dir + (u * s + v * c) * Scalar(0.025), allow);
			if (ma == m && mb == m)
			{
				allow[m] = 3;
				return m;
			}
			if (ma != -1 && ma != mb)  // Yuck - this is really ugly
			{
				i32 mc = ma;
				for (Scalar xx = x - Scalar(40.0); xx <= x; xx += Scalar(5.0))
				{
					Scalar s = Sin(SIMD_RADS_PER_DEG * (xx));
					Scalar c = Cos(SIMD_RADS_PER_DEG * (xx));
					i32 md = maxdirfiltered(p, count, dir + (u * s + v * c) * Scalar(0.025), allow);
					if (mc == m && md == m)
					{
						allow[m] = 3;
						return m;
					}
					mc = md;
				}
			}
			ma = mb;
		}
		allow[m] = 0;
		m = -1;
	}
	Assert(0);
	return m;
}

i32 operator==(const int3 &a, const int3 &b);
i32 operator==(const int3 &a, const int3 &b)
{
	for (i32 i = 0; i < 3; i++)
	{
		if (a[i] != b[i]) return 0;
	}
	return 1;
}

i32 above(Vec3 *vertices, const int3 &t, const Vec3 &p, Scalar epsilon);
i32 above(Vec3 *vertices, const int3 &t, const Vec3 &p, Scalar epsilon)
{
	Vec3 n = TriNormal(vertices[t[0]], vertices[t[1]], vertices[t[2]]);
	return (Dot(n, p - vertices[t[0]]) > epsilon);  // EPSILON???
}
i32 hasedge(const int3 &t, i32 a, i32 b);
i32 hasedge(const int3 &t, i32 a, i32 b)
{
	for (i32 i = 0; i < 3; i++)
	{
		i32 i1 = (i + 1) % 3;
		if (t[i] == a && t[i1] == b) return 1;
	}
	return 0;
}
i32 hasvert(const int3 &t, i32 v);
i32 hasvert(const int3 &t, i32 v)
{
	return (t[0] == v || t[1] == v || t[2] == v);
}
i32 shareedge(const int3 &a, const int3 &b);
i32 shareedge(const int3 &a, const int3 &b)
{
	i32 i;
	for (i = 0; i < 3; i++)
	{
		i32 i1 = (i + 1) % 3;
		if (hasedge(a, b[i1], b[i])) return 1;
	}
	return 0;
}

class HullTriangle;

class HullTriangle : public int3
{
public:
	int3 n;
	i32 id;
	i32 vmax;
	Scalar rise;
	HullTriangle(i32 a, i32 b, i32 c) : int3(a, b, c), n(-1, -1, -1)
	{
		vmax = -1;
		rise = Scalar(0.0);
	}
	~HullTriangle()
	{
	}
	i32 &neib(i32 a, i32 b);
};

i32 &HullTriangle::neib(i32 a, i32 b)
{
	static i32 er = -1;
	i32 i;
	for (i = 0; i < 3; i++)
	{
		i32 i1 = (i + 1) % 3;
		i32 i2 = (i + 2) % 3;
		if ((*this)[i] == a && (*this)[i1] == b) return n[i2];
		if ((*this)[i] == b && (*this)[i1] == a) return n[i2];
	}
	Assert(0);
	return er;
}
void HullLibrary::b2bfix(HullTriangle *s, HullTriangle *t)
{
	i32 i;
	for (i = 0; i < 3; i++)
	{
		i32 i1 = (i + 1) % 3;
		i32 i2 = (i + 2) % 3;
		i32 a = (*s)[i1];
		i32 b = (*s)[i2];
		Assert(m_tris[s->neib(a, b)]->neib(b, a) == s->id);
		Assert(m_tris[t->neib(a, b)]->neib(b, a) == t->id);
		m_tris[s->neib(a, b)]->neib(b, a) = t->neib(b, a);
		m_tris[t->neib(b, a)]->neib(a, b) = s->neib(a, b);
	}
}

void HullLibrary::removeb2b(HullTriangle *s, HullTriangle *t)
{
	b2bfix(s, t);
	deAllocateTriangle(s);

	deAllocateTriangle(t);
}

void HullLibrary::checkit(HullTriangle *t)
{
	(void)t;

	i32 i;
	Assert(m_tris[t->id] == t);
	for (i = 0; i < 3; i++)
	{
		i32 i1 = (i + 1) % 3;
		i32 i2 = (i + 2) % 3;
		i32 a = (*t)[i1];
		i32 b = (*t)[i2];

		// release compile fix
		(void)i1;
		(void)i2;
		(void)a;
		(void)b;

		Assert(a != b);
		Assert(m_tris[t->n[i]]->neib(b, a) == t->id);
	}
}

HullTriangle *HullLibrary::allocateTriangle(i32 a, i32 b, i32 c)
{
	uk mem = AlignedAlloc(sizeof(HullTriangle), 16);
	HullTriangle *tr = new (mem) HullTriangle(a, b, c);
	tr->id = m_tris.size();
	m_tris.push_back(tr);

	return tr;
}

void HullLibrary::deAllocateTriangle(HullTriangle *tri)
{
	Assert(m_tris[tri->id] == tri);
	m_tris[tri->id] = NULL;
	tri->~HullTriangle();
	AlignedFree(tri);
}

void HullLibrary::extrude(HullTriangle *t0, i32 v)
{
	int3 t = *t0;
	i32 n = m_tris.size();
	HullTriangle *ta = allocateTriangle(v, t[1], t[2]);
	ta->n = int3(t0->n[0], n + 1, n + 2);
	m_tris[t0->n[0]]->neib(t[1], t[2]) = n + 0;
	HullTriangle *tb = allocateTriangle(v, t[2], t[0]);
	tb->n = int3(t0->n[1], n + 2, n + 0);
	m_tris[t0->n[1]]->neib(t[2], t[0]) = n + 1;
	HullTriangle *tc = allocateTriangle(v, t[0], t[1]);
	tc->n = int3(t0->n[2], n + 0, n + 1);
	m_tris[t0->n[2]]->neib(t[0], t[1]) = n + 2;
	checkit(ta);
	checkit(tb);
	checkit(tc);
	if (hasvert(*m_tris[ta->n[0]], v)) removeb2b(ta, m_tris[ta->n[0]]);
	if (hasvert(*m_tris[tb->n[0]], v)) removeb2b(tb, m_tris[tb->n[0]]);
	if (hasvert(*m_tris[tc->n[0]], v)) removeb2b(tc, m_tris[tc->n[0]]);
	deAllocateTriangle(t0);
}

HullTriangle *HullLibrary::extrudable(Scalar epsilon)
{
	i32 i;
	HullTriangle *t = NULL;
	for (i = 0; i < m_tris.size(); i++)
	{
		if (!t || (m_tris[i] && t->rise < m_tris[i]->rise))
		{
			t = m_tris[i];
		}
	}
	return (t->rise > epsilon) ? t : NULL;
}

int4 HullLibrary::FindSimplex(Vec3 *verts, i32 verts_count, AlignedObjectArray<i32> &allow)
{
	Vec3 basis[3];
	basis[0] = Vec3(Scalar(0.01), Scalar(0.02), Scalar(1.0));
	i32 p0 = maxdirsterid(verts, verts_count, basis[0], allow);
	i32 p1 = maxdirsterid(verts, verts_count, -basis[0], allow);
	basis[0] = verts[p0] - verts[p1];
	if (p0 == p1 || basis[0] == Vec3(0, 0, 0))
		return int4(-1, -1, -1, -1);
	basis[1] = Cross(Vec3(Scalar(1), Scalar(0.02), Scalar(0)), basis[0]);
	basis[2] = Cross(Vec3(Scalar(-0.02), Scalar(1), Scalar(0)), basis[0]);
	if (basis[1].length() > basis[2].length())
	{
		basis[1].normalize();
	}
	else
	{
		basis[1] = basis[2];
		basis[1].normalize();
	}
	i32 p2 = maxdirsterid(verts, verts_count, basis[1], allow);
	if (p2 == p0 || p2 == p1)
	{
		p2 = maxdirsterid(verts, verts_count, -basis[1], allow);
	}
	if (p2 == p0 || p2 == p1)
		return int4(-1, -1, -1, -1);
	basis[1] = verts[p2] - verts[p0];
	basis[2] = Cross(basis[1], basis[0]).normalized();
	i32 p3 = maxdirsterid(verts, verts_count, basis[2], allow);
	if (p3 == p0 || p3 == p1 || p3 == p2) p3 = maxdirsterid(verts, verts_count, -basis[2], allow);
	if (p3 == p0 || p3 == p1 || p3 == p2)
		return int4(-1, -1, -1, -1);
	Assert(!(p0 == p1 || p0 == p2 || p0 == p3 || p1 == p2 || p1 == p3 || p2 == p3));
	if (Dot(verts[p3] - verts[p0], Cross(verts[p1] - verts[p0], verts[p2] - verts[p0])) < 0)
	{
		Swap(p2, p3);
	}
	return int4(p0, p1, p2, p3);
}

i32 HullLibrary::calchullgen(Vec3 *verts, i32 verts_count, i32 vlimit)
{
	if (verts_count < 4) return 0;
	if (vlimit == 0) vlimit = 1000000000;
	i32 j;
	Vec3 bmin(*verts), bmax(*verts);
	AlignedObjectArray<i32> isextreme;
	isextreme.reserve(verts_count);
	AlignedObjectArray<i32> allow;
	allow.reserve(verts_count);

	for (j = 0; j < verts_count; j++)
	{
		allow.push_back(1);
		isextreme.push_back(0);
		bmin.setMin(verts[j]);
		bmax.setMax(verts[j]);
	}
	Scalar epsilon = (bmax - bmin).length() * Scalar(0.001);
	Assert(epsilon != 0.0);

	int4 p = FindSimplex(verts, verts_count, allow);
	if (p.x == -1) return 0;  // simplex failed

	Vec3 center = (verts[p[0]] + verts[p[1]] + verts[p[2]] + verts[p[3]]) / Scalar(4.0);  // a valid interior point
	HullTriangle *t0 = allocateTriangle(p[2], p[3], p[1]);
	t0->n = int3(2, 3, 1);
	HullTriangle *t1 = allocateTriangle(p[3], p[2], p[0]);
	t1->n = int3(3, 2, 0);
	HullTriangle *t2 = allocateTriangle(p[0], p[1], p[3]);
	t2->n = int3(0, 1, 3);
	HullTriangle *t3 = allocateTriangle(p[1], p[0], p[2]);
	t3->n = int3(1, 0, 2);
	isextreme[p[0]] = isextreme[p[1]] = isextreme[p[2]] = isextreme[p[3]] = 1;
	checkit(t0);
	checkit(t1);
	checkit(t2);
	checkit(t3);

	for (j = 0; j < m_tris.size(); j++)
	{
		HullTriangle *t = m_tris[j];
		Assert(t);
		Assert(t->vmax < 0);
		Vec3 n = TriNormal(verts[(*t)[0]], verts[(*t)[1]], verts[(*t)[2]]);
		t->vmax = maxdirsterid(verts, verts_count, n, allow);
		t->rise = Dot(n, verts[t->vmax] - verts[(*t)[0]]);
	}
	HullTriangle *te;
	vlimit -= 4;
	while (vlimit > 0 && ((te = extrudable(epsilon)) != 0))
	{
		//int3 ti=*te;
		i32 v = te->vmax;
		Assert(v != -1);
		Assert(!isextreme[v]);  // wtf we've already done this vertex
		isextreme[v] = 1;
		//if(v==p0 || v==p1 || v==p2 || v==p3) continue; // done these already
		j = m_tris.size();
		while (j--)
		{
			if (!m_tris[j]) continue;
			int3 t = *m_tris[j];
			if (above(verts, t, verts[v], Scalar(0.01) * epsilon))
			{
				extrude(m_tris[j], v);
			}
		}
		// now check for those degenerate cases where we have a flipped triangle or a really skinny triangle
		j = m_tris.size();
		while (j--)
		{
			if (!m_tris[j]) continue;
			if (!hasvert(*m_tris[j], v)) break;
			int3 nt = *m_tris[j];
			if (above(verts, nt, center, Scalar(0.01) * epsilon) || Cross(verts[nt[1]] - verts[nt[0]], verts[nt[2]] - verts[nt[1]]).length() < epsilon * epsilon * Scalar(0.1))
			{
				HullTriangle *nb = m_tris[m_tris[j]->n[0]];
				Assert(nb);
				Assert(!hasvert(*nb, v));
				Assert(nb->id < j);
				extrude(nb, v);
				j = m_tris.size();
			}
		}
		j = m_tris.size();
		while (j--)
		{
			HullTriangle *t = m_tris[j];
			if (!t) continue;
			if (t->vmax >= 0) break;
			Vec3 n = TriNormal(verts[(*t)[0]], verts[(*t)[1]], verts[(*t)[2]]);
			t->vmax = maxdirsterid(verts, verts_count, n, allow);
			if (isextreme[t->vmax])
			{
				t->vmax = -1;  // already done that vertex - algorithm needs to be able to terminate.
			}
			else
			{
				t->rise = Dot(n, verts[t->vmax] - verts[(*t)[0]]);
			}
		}
		vlimit--;
	}
	return 1;
}

i32 HullLibrary::calchull(Vec3 *verts, i32 verts_count, TUIntArray &tris_out, i32 &tris_count, i32 vlimit)
{
	i32 rc = calchullgen(verts, verts_count, vlimit);
	if (!rc) return 0;
	AlignedObjectArray<i32> ts;
	i32 i;

	for (i = 0; i < m_tris.size(); i++)
	{
		if (m_tris[i])
		{
			for (i32 j = 0; j < 3; j++)
				ts.push_back((*m_tris[i])[j]);
			deAllocateTriangle(m_tris[i]);
		}
	}
	tris_count = ts.size() / 3;
	tris_out.resize(ts.size());

	for (i = 0; i < ts.size(); i++)
	{
		tris_out[i] = static_cast<u32>(ts[i]);
	}
	m_tris.resize(0);

	return 1;
}

bool HullLibrary::ComputeHull(u32 vcount, const Vec3 *vertices, PHullResult &result, u32 vlimit)
{
	i32 tris_count;
	i32 ret = calchull((Vec3 *)vertices, (i32)vcount, result.m_Indices, tris_count, static_cast<i32>(vlimit));
	if (!ret) return false;
	result.mIndexCount = (u32)(tris_count * 3);
	result.mFaceCount = (u32)tris_count;
	result.mVertices = (Vec3 *)vertices;
	result.mVcount = (u32)vcount;
	return true;
}

void ReleaseHull(PHullResult &result);
void ReleaseHull(PHullResult &result)
{
	if (result.m_Indices.size())
	{
		result.m_Indices.clear();
	}

	result.mVcount = 0;
	result.mIndexCount = 0;
	result.mVertices = 0;
}

//*********************************************************************
//*********************************************************************
//********  HullLib header
//*********************************************************************
//*********************************************************************

//*********************************************************************
//*********************************************************************
//********  HullLib implementation
//*********************************************************************
//*********************************************************************

HullError HullLibrary::CreateConvexHull(const HullDesc &desc,  // describes the input request
										HullResult &result)    // contains the resulst
{
	HullError ret = QE_FAIL;

	PHullResult hr;

	u32 vcount = desc.mVcount;
	if (vcount < 8) vcount = 8;

	AlignedObjectArray<Vec3> vertexSource;
	Vec3 zero;
	zero.setZero();
	vertexSource.resize(static_cast<i32>(vcount), zero);

	Vec3 scale;

	u32 ovcount;

	bool ok = CleanupVertices(desc.mVcount, desc.mVertices, desc.mVertexStride, ovcount, &vertexSource[0], desc.mNormalEpsilon, scale);  // normalize point cloud, remove duplicates!

	if (ok)
	{
		//		if ( 1 ) // scale vertices back to their original size.
		{
			for (u32 i = 0; i < ovcount; i++)
			{
				Vec3 &v = vertexSource[static_cast<i32>(i)];
				v[0] *= scale[0];
				v[1] *= scale[1];
				v[2] *= scale[2];
			}
		}

		ok = ComputeHull(ovcount, &vertexSource[0], hr, desc.mMaxVertices);

		if (ok)
		{
			// re-index triangle mesh so it refers to only used vertices, rebuild a new vertex table.
			AlignedObjectArray<Vec3> vertexScratch;
			vertexScratch.resize(static_cast<i32>(hr.mVcount));

			BringOutYourDead(hr.mVertices, hr.mVcount, &vertexScratch[0], ovcount, &hr.m_Indices[0], hr.mIndexCount);

			ret = QE_OK;

			if (desc.HasHullFlag(QF_TRIANGLES))  // if he wants the results as triangle!
			{
				result.mPolygons = false;
				result.mNumOutputVertices = ovcount;
				result.m_OutputVertices.resize(static_cast<i32>(ovcount));
				result.mNumFaces = hr.mFaceCount;
				result.mNumIndices = hr.mIndexCount;

				result.m_Indices.resize(static_cast<i32>(hr.mIndexCount));

				memcpy(&result.m_OutputVertices[0], &vertexScratch[0], sizeof(Vec3) * ovcount);

				if (desc.HasHullFlag(QF_REVERSE_ORDER))
				{
					u32k *source = &hr.m_Indices[0];
					u32 *dest = &result.m_Indices[0];

					for (u32 i = 0; i < hr.mFaceCount; i++)
					{
						dest[0] = source[2];
						dest[1] = source[1];
						dest[2] = source[0];
						dest += 3;
						source += 3;
					}
				}
				else
				{
					memcpy(&result.m_Indices[0], &hr.m_Indices[0], sizeof(u32) * hr.mIndexCount);
				}
			}
			else
			{
				result.mPolygons = true;
				result.mNumOutputVertices = ovcount;
				result.m_OutputVertices.resize(static_cast<i32>(ovcount));
				result.mNumFaces = hr.mFaceCount;
				result.mNumIndices = hr.mIndexCount + hr.mFaceCount;
				result.m_Indices.resize(static_cast<i32>(result.mNumIndices));
				memcpy(&result.m_OutputVertices[0], &vertexScratch[0], sizeof(Vec3) * ovcount);

				//				if ( 1 )
				{
					u32k *source = &hr.m_Indices[0];
					u32 *dest = &result.m_Indices[0];
					for (u32 i = 0; i < hr.mFaceCount; i++)
					{
						dest[0] = 3;
						if (desc.HasHullFlag(QF_REVERSE_ORDER))
						{
							dest[1] = source[2];
							dest[2] = source[1];
							dest[3] = source[0];
						}
						else
						{
							dest[1] = source[0];
							dest[2] = source[1];
							dest[3] = source[2];
						}

						dest += 4;
						source += 3;
					}
				}
			}
			ReleaseHull(hr);
		}
	}

	return ret;
}

HullError HullLibrary::ReleaseResult(HullResult &result)  // release memory allocated for this result, we are done with it.
{
	if (result.m_OutputVertices.size())
	{
		result.mNumOutputVertices = 0;
		result.m_OutputVertices.clear();
	}
	if (result.m_Indices.size())
	{
		result.mNumIndices = 0;
		result.m_Indices.clear();
	}
	return QE_OK;
}

static void addPoint(u32 &vcount, Vec3 *p, Scalar x, Scalar y, Scalar z)
{
	// XXX, might be broken
	Vec3 &dest = p[vcount];
	dest[0] = x;
	dest[1] = y;
	dest[2] = z;
	vcount++;
}

Scalar GetDist(Scalar px, Scalar py, Scalar pz, const Scalar *p2);
Scalar GetDist(Scalar px, Scalar py, Scalar pz, const Scalar *p2)
{
	Scalar dx = px - p2[0];
	Scalar dy = py - p2[1];
	Scalar dz = pz - p2[2];

	return dx * dx + dy * dy + dz * dz;
}

bool HullLibrary::CleanupVertices(u32 svcount,
								  const Vec3 *svertices,
								  u32 stride,
								  u32 &vcount,  // output number of vertices
								  Vec3 *vertices,   // location to store the results.
								  Scalar normalepsilon,
								  Vec3 &scale)
{
	if (svcount == 0) return false;

	m_vertexIndexMapping.resize(0);

#define EPSILON Scalar(0.000001) /* close enough to consider two Scalaring point numbers to be 'the same'. */

	vcount = 0;

	Scalar recip[3] = {0.f, 0.f, 0.f};

	if (scale)
	{
		scale[0] = 1;
		scale[1] = 1;
		scale[2] = 1;
	}

	Scalar bmin[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
	Scalar bmax[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

	tukk vtx = (tukk)svertices;

	//	if ( 1 )
	{
		for (u32 i = 0; i < svcount; i++)
		{
			const Scalar *p = (const Scalar *)vtx;

			vtx += stride;

			for (i32 j = 0; j < 3; j++)
			{
				if (p[j] < bmin[j]) bmin[j] = p[j];
				if (p[j] > bmax[j]) bmax[j] = p[j];
			}
		}
	}

	Scalar dx = bmax[0] - bmin[0];
	Scalar dy = bmax[1] - bmin[1];
	Scalar dz = bmax[2] - bmin[2];

	Vec3 center;

	center[0] = dx * Scalar(0.5) + bmin[0];
	center[1] = dy * Scalar(0.5) + bmin[1];
	center[2] = dz * Scalar(0.5) + bmin[2];

	if (dx < EPSILON || dy < EPSILON || dz < EPSILON || svcount < 3)
	{
		Scalar len = FLT_MAX;

		if (dx > EPSILON && dx < len) len = dx;
		if (dy > EPSILON && dy < len) len = dy;
		if (dz > EPSILON && dz < len) len = dz;

		if (len == FLT_MAX)
		{
			dx = dy = dz = Scalar(0.01);  // one centimeter
		}
		else
		{
			if (dx < EPSILON) dx = len * Scalar(0.05);  // 1/5th the shortest non-zero edge.
			if (dy < EPSILON) dy = len * Scalar(0.05);
			if (dz < EPSILON) dz = len * Scalar(0.05);
		}

		Scalar x1 = center[0] - dx;
		Scalar x2 = center[0] + dx;

		Scalar y1 = center[1] - dy;
		Scalar y2 = center[1] + dy;

		Scalar z1 = center[2] - dz;
		Scalar z2 = center[2] + dz;

		addPoint(vcount, vertices, x1, y1, z1);
		addPoint(vcount, vertices, x2, y1, z1);
		addPoint(vcount, vertices, x2, y2, z1);
		addPoint(vcount, vertices, x1, y2, z1);
		addPoint(vcount, vertices, x1, y1, z2);
		addPoint(vcount, vertices, x2, y1, z2);
		addPoint(vcount, vertices, x2, y2, z2);
		addPoint(vcount, vertices, x1, y2, z2);

		return true;  // return cube
	}
	else
	{
		if (scale)
		{
			scale[0] = dx;
			scale[1] = dy;
			scale[2] = dz;

			recip[0] = 1 / dx;
			recip[1] = 1 / dy;
			recip[2] = 1 / dz;

			center[0] *= recip[0];
			center[1] *= recip[1];
			center[2] *= recip[2];
		}
	}

	vtx = (tukk)svertices;

	for (u32 i = 0; i < svcount; i++)
	{
		const Vec3 *p = (const Vec3 *)vtx;
		vtx += stride;

		Scalar px = p->getX();
		Scalar py = p->getY();
		Scalar pz = p->getZ();

		if (scale)
		{
			px = px * recip[0];  // normalize
			py = py * recip[1];  // normalize
			pz = pz * recip[2];  // normalize
		}

		//		if ( 1 )
		{
			u32 j;

			for (j = 0; j < vcount; j++)
			{
				/// XXX might be broken
				Vec3 &v = vertices[j];

				Scalar x = v[0];
				Scalar y = v[1];
				Scalar z = v[2];

				Scalar dx = Fabs(x - px);
				Scalar dy = Fabs(y - py);
				Scalar dz = Fabs(z - pz);

				if (dx < normalepsilon && dy < normalepsilon && dz < normalepsilon)
				{
					// ok, it is close enough to the old one
					// now let us see if it is further from the center of the point cloud than the one we already recorded.
					// in which case we keep this one instead.

					Scalar dist1 = GetDist(px, py, pz, center);
					Scalar dist2 = GetDist(v[0], v[1], v[2], center);

					if (dist1 > dist2)
					{
						v[0] = px;
						v[1] = py;
						v[2] = pz;
					}

					break;
				}
			}

			if (j == vcount)
			{
				Vec3 &dest = vertices[vcount];
				dest[0] = px;
				dest[1] = py;
				dest[2] = pz;
				vcount++;
			}
			m_vertexIndexMapping.push_back(j);
		}
	}

	// ok..now make sure we didn't prune so many vertices it is now invalid.
	//	if ( 1 )
	{
		Scalar bmin[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
		Scalar bmax[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

		for (u32 i = 0; i < vcount; i++)
		{
			const Vec3 &p = vertices[i];
			for (i32 j = 0; j < 3; j++)
			{
				if (p[j] < bmin[j]) bmin[j] = p[j];
				if (p[j] > bmax[j]) bmax[j] = p[j];
			}
		}

		Scalar dx = bmax[0] - bmin[0];
		Scalar dy = bmax[1] - bmin[1];
		Scalar dz = bmax[2] - bmin[2];

		if (dx < EPSILON || dy < EPSILON || dz < EPSILON || vcount < 3)
		{
			Scalar cx = dx * Scalar(0.5) + bmin[0];
			Scalar cy = dy * Scalar(0.5) + bmin[1];
			Scalar cz = dz * Scalar(0.5) + bmin[2];

			Scalar len = FLT_MAX;

			if (dx >= EPSILON && dx < len) len = dx;
			if (dy >= EPSILON && dy < len) len = dy;
			if (dz >= EPSILON && dz < len) len = dz;

			if (len == FLT_MAX)
			{
				dx = dy = dz = Scalar(0.01);  // one centimeter
			}
			else
			{
				if (dx < EPSILON) dx = len * Scalar(0.05);  // 1/5th the shortest non-zero edge.
				if (dy < EPSILON) dy = len * Scalar(0.05);
				if (dz < EPSILON) dz = len * Scalar(0.05);
			}

			Scalar x1 = cx - dx;
			Scalar x2 = cx + dx;

			Scalar y1 = cy - dy;
			Scalar y2 = cy + dy;

			Scalar z1 = cz - dz;
			Scalar z2 = cz + dz;

			vcount = 0;  // add box

			addPoint(vcount, vertices, x1, y1, z1);
			addPoint(vcount, vertices, x2, y1, z1);
			addPoint(vcount, vertices, x2, y2, z1);
			addPoint(vcount, vertices, x1, y2, z1);
			addPoint(vcount, vertices, x1, y1, z2);
			addPoint(vcount, vertices, x2, y1, z2);
			addPoint(vcount, vertices, x2, y2, z2);
			addPoint(vcount, vertices, x1, y2, z2);

			return true;
		}
	}

	return true;
}

void HullLibrary::BringOutYourDead(const Vec3 *verts, u32 vcount, Vec3 *overts, u32 &ocount, u32 *indices, unsigned indexcount)
{
	AlignedObjectArray<i32> tmpIndices;
	tmpIndices.resize(m_vertexIndexMapping.size());
	i32 i;

	for (i = 0; i < m_vertexIndexMapping.size(); i++)
	{
		tmpIndices[i] = m_vertexIndexMapping[i];
	}

	TUIntArray usedIndices;
	usedIndices.resize(static_cast<i32>(vcount));
	memset(&usedIndices[0], 0, sizeof(u32) * vcount);

	ocount = 0;

	for (i = 0; i < i32(indexcount); i++)
	{
		u32 v = indices[i];  // original array index

		Assert(v >= 0 && v < vcount);

		if (usedIndices[static_cast<i32>(v)])  // if already remapped
		{
			indices[i] = usedIndices[static_cast<i32>(v)] - 1;  // index to new array
		}
		else
		{
			indices[i] = ocount;  // new index mapping

			overts[ocount][0] = verts[v][0];  // copy old vert to new vert array
			overts[ocount][1] = verts[v][1];
			overts[ocount][2] = verts[v][2];

			for (i32 k = 0; k < m_vertexIndexMapping.size(); k++)
			{
				if (tmpIndices[k] == i32(v))
					m_vertexIndexMapping[k] = ocount;
			}

			ocount++;  // increment output vert count

			Assert(ocount >= 0 && ocount <= vcount);

			usedIndices[static_cast<i32>(v)] = ocount;  // assign new index remapping
		}
	}
}
