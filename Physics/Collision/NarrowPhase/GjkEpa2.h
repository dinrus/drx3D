#ifndef DRX3D_GJK_EPA2_H
#define DRX3D_GJK_EPA2_H

#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>

//GjkEpaSolver contributed under zlib by Nathanael Presson
struct GjkEpaSolver2
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
		Vec3 witnesses[2];
		Vec3 normal;
		Scalar distance;
	};

	static i32 StackSizeRequirement();

	static bool Distance(const ConvexShape* shape0, const Transform2& wtrs0,
						 const ConvexShape* shape1, const Transform2& wtrs1,
						 const Vec3& guess,
						 sResults& results);

	static bool Penetration(const ConvexShape* shape0, const Transform2& wtrs0,
							const ConvexShape* shape1, const Transform2& wtrs1,
							const Vec3& guess,
							sResults& results,
							bool usemargins = true);
#ifndef __SPU__
	static Scalar SignedDistance(const Vec3& position,
								   Scalar margin,
								   const ConvexShape* shape,
								   const Transform2& wtrs,
								   sResults& results);

	static bool SignedDistance(const ConvexShape* shape0, const Transform2& wtrs0,
							   const ConvexShape* shape1, const Transform2& wtrs1,
							   const Vec3& guess,
							   sResults& results);
#endif  //__SPU__
};

#endif  //DRX3D_GJK_EPA2_H
