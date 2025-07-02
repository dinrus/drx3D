
#ifndef DRX3D_VORONOI_SIMPLEX_SOLVER_H
#define DRX3D_VORONOI_SIMPLEX_SOLVER_H

#include <drx3D/Physics/Collision/NarrowPhase/SimplexSolverInterface.h>

#define VORONOI_SIMPLEX_MAX_VERTS 5

///disable next define, or use defaultCollisionConfiguration->getSimplexSolver()->setEqualVertexThreshold(0.f) to disable/configure
#define DRX3D_USE_EQUAL_VERTEX_THRESHOLD

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define VORONOI_DEFAULT_EQUAL_VERTEX_THRESHOLD 1e-12f
#else
#define VORONOI_DEFAULT_EQUAL_VERTEX_THRESHOLD 0.0001f
#endif  //DRX3D_USE_DOUBLE_PRECISION

struct UsageBitfield
{
	UsageBitfield()
	{
		reset();
	}

	void reset()
	{
		usedVertexA = false;
		usedVertexB = false;
		usedVertexC = false;
		usedVertexD = false;
	}
	unsigned short usedVertexA : 1;
	unsigned short usedVertexB : 1;
	unsigned short usedVertexC : 1;
	unsigned short usedVertexD : 1;
	unsigned short unused1 : 1;
	unsigned short unused2 : 1;
	unsigned short unused3 : 1;
	unsigned short unused4 : 1;
};

struct SubSimplexClosestResult
{
	Vec3 m_closestPointOnSimplex;
	//MASK for m_usedVertices
	//stores the simplex vertex-usage, using the MASK,
	// if m_usedVertices & MASK then the related vertex is used
	UsageBitfield m_usedVertices;
	Scalar m_barycentricCoords[4];
	bool m_degenerate;

	void reset()
	{
		m_degenerate = false;
		setBarycentricCoordinates();
		m_usedVertices.reset();
	}
	bool isValid()
	{
		bool valid = (m_barycentricCoords[0] >= Scalar(0.)) &&
					 (m_barycentricCoords[1] >= Scalar(0.)) &&
					 (m_barycentricCoords[2] >= Scalar(0.)) &&
					 (m_barycentricCoords[3] >= Scalar(0.));

		return valid;
	}
	void setBarycentricCoordinates(Scalar a = Scalar(0.), Scalar b = Scalar(0.), Scalar c = Scalar(0.), Scalar d = Scalar(0.))
	{
		m_barycentricCoords[0] = a;
		m_barycentricCoords[1] = b;
		m_barycentricCoords[2] = c;
		m_barycentricCoords[3] = d;
	}
};

/// VoronoiSimplexSolver is an implementation of the closest point distance algorithm from a 1-4 points simplex to the origin.
/// Can be used with GJK, as an alternative to Johnson distance algorithm.
#ifdef NO_VIRTUAL_INTERFACE
ATTRIBUTE_ALIGNED16(class)
VoronoiSimplexSolver
#else
ATTRIBUTE_ALIGNED16(class)
VoronoiSimplexSolver : public SimplexSolverInterface
#endif
{
public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	i32 m_numVertices;

	Vec3 m_simplexVectorW[VORONOI_SIMPLEX_MAX_VERTS];
	Vec3 m_simplexPointsP[VORONOI_SIMPLEX_MAX_VERTS];
	Vec3 m_simplexPointsQ[VORONOI_SIMPLEX_MAX_VERTS];

	Vec3 m_cachedP1;
	Vec3 m_cachedP2;
	Vec3 m_cachedV;
	Vec3 m_lastW;

	Scalar m_equalVertexThreshold;
	bool m_cachedValidClosest;

	SubSimplexClosestResult m_cachedBC;

	bool m_needsUpdate;

	void removeVertex(i32 index);
	void reduceVertices(const UsageBitfield& usedVerts);
	bool updateClosestVectorAndPoints();

	bool closestPtPointTetrahedron(const Vec3& p, const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, SubSimplexClosestResult& finalResult);
	i32 pointOutsideOfPlane(const Vec3& p, const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d);
	bool closestPtPointTriangle(const Vec3& p, const Vec3& a, const Vec3& b, const Vec3& c, SubSimplexClosestResult& result);

public:
	VoronoiSimplexSolver()
		: m_equalVertexThreshold(VORONOI_DEFAULT_EQUAL_VERTEX_THRESHOLD)
	{
	}
	void reset();

	void addVertex(const Vec3& w, const Vec3& p, const Vec3& q);

	void setEqualVertexThreshold(Scalar threshold)
	{
		m_equalVertexThreshold = threshold;
	}

	Scalar getEqualVertexThreshold() const
	{
		return m_equalVertexThreshold;
	}

	bool closest(Vec3 & v);

	Scalar maxVertex();

	bool fullSimplex() const
	{
		return (m_numVertices == 4);
	}

	i32 getSimplex(Vec3 * pBuf, Vec3 * qBuf, Vec3 * yBuf) const;

	bool inSimplex(const Vec3& w);

	void backup_closest(Vec3 & v);

	bool emptySimplex() const;

	void compute_points(Vec3 & p1, Vec3 & p2);

	i32 numVertices() const
	{
		return m_numVertices;
	}
};

#endif  //DRX3D_VORONOI_SIMPLEX_SOLVER_H
