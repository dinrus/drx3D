#ifndef BSP_CONVERTER_H
#define BSP_CONVERTER_H

class BspLoader;
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

///BspConverter turns a loaded bsp level into convex parts (vertices)
class BspConverter
{
public:
	void convertBsp(BspLoader& bspLoader, float scaling);
	virtual ~BspConverter()
	{
	}

	///this callback is called for each brush that succesfully converted into vertices
	virtual void addConvexVerticesCollider(AlignedObjectArray<Vec3>& vertices, bool isEntity, const Vec3& entityTargetLocation) = 0;
};

#endif  //BSP_CONVERTER_H
