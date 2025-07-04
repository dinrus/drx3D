
#ifndef D3_COLLIDABLE_H
#define D3_COLLIDABLE_H

#include <drx3D/Common/shared/b3Float4.h>
#include <drx3D/Common/shared/b3Quat.h>

enum b3ShapeTypes
{
	SHAPE_HEIGHT_FIELD = 1,

	SHAPE_CONVEX_HULL = 3,
	SHAPE_PLANE = 4,
	SHAPE_CONCAVE_TRIMESH = 5,
	SHAPE_COMPOUND_OF_CONVEX_HULLS = 6,
	SHAPE_SPHERE = 7,
	MAX_NUM_SHAPE_TYPES,
};

typedef struct b3Collidable b3Collidable_t;

struct b3Collidable
{
	union {
		i32 m_numChildShapes;
		i32 m_bvhIndex;
	};
	union {
		float m_radius;
		i32 m_compoundBvhIndex;
	};

	i32 m_shapeType;
	union {
		i32 m_shapeIndex;
		float m_height;
	};
};

typedef struct b3GpuChildShape b3GpuChildShape_t;
struct b3GpuChildShape
{
	b3Float4 m_childPosition;
	b3Quat m_childOrientation;
	union {
		i32 m_shapeIndex;  //used for SHAPE_COMPOUND_OF_CONVEX_HULLS
		i32 m_capsuleAxis;
	};
	union {
		float m_radius;        //used for childshape of SHAPE_COMPOUND_OF_SPHERES or SHAPE_COMPOUND_OF_CAPSULES
		i32 m_numChildShapes;  //used for compound shape
	};
	union {
		float m_height;  //used for childshape of SHAPE_COMPOUND_OF_CAPSULES
		i32 m_collidableShapeIndex;
	};
	i32 m_shapeType;
};

struct b3CompoundOverlappingPair
{
	i32 m_bodyIndexA;
	i32 m_bodyIndexB;
	//	i32	m_pairType;
	i32 m_childShapeIndexA;
	i32 m_childShapeIndexB;
};

#endif  //D3_COLLIDABLE_H
