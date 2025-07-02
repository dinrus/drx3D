#ifndef D3_GJK_EPA2_H
#define D3_GJK_EPA2_H

#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/b3Transform.h>
#include <drx3D/Physics/Collision/NarrowPhase/shared/b3ConvexPolyhedronData.h>

//GjkEpaSolver contributed under zlib by Nathanael Presson
struct b3GjkEpaSolver2
{
	struct sResults
	{
		enum eStatus
		{
			Separated,   /* Shapes doesnt penetrate												*/
			Penetrating, /* Shapes are penetrating												*/
			GJK_Failed,  /* GJK phase fail, no big issue, shapes are probably just 'touching'	*/
			EPA_Failed   /* EPA phase fail, bigger problem, need to save parameters, and debug	*/
		} status;
		b3Vec3 witnesses[2];
		b3Vec3 normal;
		b3Scalar distance;
	};

	static i32 StackSizeRequirement();

	static bool Distance(const b3Transform& transA, const b3Transform& transB,
						 const b3ConvexPolyhedronData* hullA, const b3ConvexPolyhedronData* hullB,
						 const b3AlignedObjectArray<b3Vec3>& verticesA,
						 const b3AlignedObjectArray<b3Vec3>& verticesB,
						 const b3Vec3& guess,
						 sResults& results);

	static bool Penetration(const b3Transform& transA, const b3Transform& transB,
							const b3ConvexPolyhedronData* hullA, const b3ConvexPolyhedronData* hullB,
							const b3AlignedObjectArray<b3Vec3>& verticesA,
							const b3AlignedObjectArray<b3Vec3>& verticesB,
							const b3Vec3& guess,
							sResults& results,
							bool usemargins = true);
#if 0
static b3Scalar	SignedDistance(	const b3Vec3& position,
								b3Scalar margin,
								const ConvexShape* shape,
								const Transform& wtrs,
								sResults& results);
							
static bool		SignedDistance(	const ConvexShape* shape0,const Transform& wtrs0,
								const ConvexShape* shape1,const Transform& wtrs1,
								const b3Vec3& guess,
								sResults& results);
#endif
};

#endif  //D3_GJK_EPA2_H
