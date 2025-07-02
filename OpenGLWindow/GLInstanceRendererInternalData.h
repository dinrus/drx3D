#ifndef GL_INSTANCE_RENDERER_INTERNAL_DATA_H
#define GL_INSTANCE_RENDERER_INTERNAL_DATA_H

#include "OpenGLInclude.h"
#include <drx3D/Common/b3AlignedObjectArray.h>

struct GLInstanceRendererInternalData
{
	b3AlignedObjectArray<GLfloat> m_instance_positions_ptr;
	b3AlignedObjectArray<GLfloat> m_instance_quaternion_ptr;
	b3AlignedObjectArray<GLfloat> m_instance_colors_ptr;
	b3AlignedObjectArray<GLfloat> m_instance_scale_ptr;

	i32 m_vboSize;
	GLuint m_vbo;
	i32 m_totalNumInstances;
	i32 m_maxNumObjectCapacity;
	i32 m_maxShapeCapacityInBytes;
};

#endif  //GL_INSTANCE_RENDERER_INTERNAL_DATA_H
