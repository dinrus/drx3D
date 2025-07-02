
#ifndef COLLADA_GRAPHICS_INSTANCE_H
#define COLLADA_GRAPHICS_INSTANCE_H

#include <drx3D/Maths/Linear/Matrix4x4.h>

struct ColladaGraphicsInstance
{
	ColladaGraphicsInstance()
		: m_shapeIndex(-1)
	{
		m_worldTransform.setIdentity();
	}
	Matrix4x4 m_worldTransform;
	i32 m_shapeIndex;  //could be index into array of GLInstanceGraphicsShape
	float m_color[4];
};

#endif  //COLLADA_GRAPHICS_INSTANCE_H
