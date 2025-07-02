#ifndef D3_VORONOI_SIMPLEX_SOLVER_H
#define D3_VORONOI_SIMPLEX_SOLVER_H

#include <drx3D/Common/b3Vec3.h>

#define VORONOI_SIMPLEX_MAX_VERTS 5

///disable next define, or use defaultCollisionConfiguration->getSimplexSolver()->setEqualVertexThreshold(0.f) to disable/configure
//#define DRX3D_USE_EQUAL_VERTEX_THRESHOLD
#define VORONOI_DEFAULT_EQUAL_VERTEX_THRESHOLD 0.0001f

struct b3UsageBitfield
{
	b3UsageBitfield()
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

struct b3SubSimplexClosestResult
{
	b3Vec3 m_closestPointOnSimplex;
	//MASK for m_usedVertices
	//stores the simplex vertex-usage, using the MASK,
	// if m_usedVertices & MASK then the related vertex is used
	b3UsageBitfield m_usedVertices;
	b3Scalar m_barycentricCoords[4];
	bool m_degenerate;

	void reset()
	{
		m_degenerate = false;
		setBarycentricCoordinates();
		m_usedVertices.reset();
	}
	bool isValid()
	{
		bool valid = (m_barycentricCoords[0] >= b3Scalar(0.)) &&
					 (m_barycentricCoords[1] >= b3Scalar(0.)) &&
					 (m_barycentricCoords[2] >= b3Scalar(0.)) &&
					 (m_barycentricCoords[3] >= b3Scalar(0.));

		return valid;
	}
	void setBarycentricCoordinates(b3Scalar a = b3Scalar(0.), b3Scalar b = b3Scalar(0.), b3Scalar c = b3Scalar(0.), b3Scalar d = b3Scalar(0.))
	{
		m_barycentricCoords[0] = a;
		m_barycentricCoords[1] = b;
		m_barycentricCoords[2] = c;
		m_barycentricCoords[3] = d;
	}
};

/// b3VoronoiSimplexSolver is an implementation of the closest point distance algorithm from a 1-4 points simplex to the origin.
/// Can be used with GJK, as an alternative to Johnson distance algorithm.

D3_ATTRIBUTE_ALIGNED16(class)
b3VoronoiSimplexSolver
{
public:
	D3_DECLARE_ALIGNED_ALLOCATOR();

	i32 m_numVertices;

	b3Vec3 m_simplexVectorW[VORONOI_SIMPLEX_MAX_VERTS];
	b3Vec3 m_simplexPointsP[VORONOI_SIMPLEX_MAX_VERTS];
	b3Vec3 m_simplexPointsQ[VORONOI_SIMPLEX_MAX_VERTS];

	b3Vec3 m_cachedP1;
	b3Vec3 m_cachedP2;
	b3Vec3 m_cachedV;
	b3Vec3 m_lastW;

	b3Scalar m_equalVertexThreshold;
	bool m_cachedValidClosest;

	b3SubSimplexClosestResult m_cachedBC;

	bool m_needsUpdate;

	void removeVertex(i32 index);
	void reduceVertices(const b3UsageBitfield& usedVerts);
	bool updateClosestVectorAndPoints();

	bool closestPtPointTetrahedron(const b3Vec3& p, const b3Vec3& a, const b3Vec3& b, const b3Vec3& c, const b3Vec3& d, b3SubSimplexClosestResult& finalResult);
	i32 pointOutsideOfPlane(const b3Vec3& p, const b3Vec3& a, const b3Vec3& b, const b3Vec3& c, const b3Vec3& d);
	bool closestPtPointTriangle(const b3Vec3& p, const b3Vec3& a, const b3Vec3& b, const b3Vec3& c, b3SubSimplexClosestResult& result);

public:
	b3VoronoiSimplexSolver()
		: m_equalVertexThreshold(VORONOI_DEFAULT_EQUAL_VERTEX_THRESHOLD)
	{
	}
	void reset();

	void addVertex(const b3Vec3& w, const b3Vec3& p, const b3Vec3& q);

	void setEqualVertexThreshold(b3Scalar threshold)
	{
		m_equalVertexThreshold = threshold;
	}

	b3Scalar getEqualVertexThreshold() const
	{
		return m_equalVertexThreshold;
	}

	bool closest(b3Vec3 & v);

	b3Scalar maxVertex();

	bool fullSimplex() const
	{
		return (m_numVertices == 4);
	}

	i32 getSimplex(b3Vec3 * pBuf, b3Vec3 * qBuf, b3Vec3 * yBuf) const;

	bool inSimplex(const b3Vec3& w);

	void backup_closest(b3Vec3 & v);

	bool emptySimplex() const;

	void compute_points(b3Vec3 & p1, b3Vec3 & p2);

	i32 numVertices() const
	{
		return m_numVertices;
	}
};

#endif  //D3_VORONOI_SIMPLEX_SOLVER_H
