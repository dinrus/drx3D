
#ifndef DRX3D_SIMPLEX_SOLVER_INTERFACE_H
#define DRX3D_SIMPLEX_SOLVER_INTERFACE_H

#include <drx3D/Maths/Linear/Vec3.h>

#define NO_VIRTUAL_INTERFACE 1
#ifdef NO_VIRTUAL_INTERFACE
#include <drx3D/Physics/Collision/NarrowPhase/VoronoiSimplexSolver.h>
#define SimplexSolverInterface VoronoiSimplexSolver
#else

/// SimplexSolverInterface can incrementally calculate distance between origin and up to 4 vertices
/// Used by GJK or Linear Casting. Can be implemented by the Johnson-algorithm or alternative approaches based on
/// voronoi regions or barycentric coordinates
class SimplexSolverInterface
{
public:
	virtual ~SimplexSolverInterface(){};

	virtual void reset() = 0;

	virtual void addVertex(const Vec3& w, const Vec3& p, const Vec3& q) = 0;

	virtual bool closest(Vec3& v) = 0;

	virtual Scalar maxVertex() = 0;

	virtual bool fullSimplex() const = 0;

	virtual i32 getSimplex(Vec3* pBuf, Vec3* qBuf, Vec3* yBuf) const = 0;

	virtual bool inSimplex(const Vec3& w) = 0;

	virtual void backup_closest(Vec3& v) = 0;

	virtual bool emptySimplex() const = 0;

	virtual void compute_points(Vec3& p1, Vec3& p2) = 0;

	virtual i32 numVertices() const = 0;
};
#endif
#endif  //DRX3D_SIMPLEX_SOLVER_INTERFACE_H
