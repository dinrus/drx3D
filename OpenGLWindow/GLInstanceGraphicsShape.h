#ifndef GL_INSTANCE_GRAPHICS_SHAPE_H
#define GL_INSTANCE_GRAPHICS_SHAPE_H

#include <drx3D/Common/b3AlignedObjectArray.h>

struct GLInstanceVertex
{
	float xyzw[4];
	float normal[3];
	float uv[2];
};
struct GLInstanceGraphicsShape
{
	b3AlignedObjectArray<GLInstanceVertex>* m_vertices;
	i32 m_numvertices;
	b3AlignedObjectArray<i32>* m_indices;
	i32 m_numIndices;
	float m_scaling[4];

	GLInstanceGraphicsShape()
		: m_vertices(0),
		  m_indices(0)
	{
	}

	virtual ~GLInstanceGraphicsShape()
	{
		delete m_vertices;
		delete m_indices;
	}
};

#endif  //GL_INSTANCE_GRAPHICS_SHAPE_H
