#ifndef LOAD_MESH_FROM_STL_H
#define LOAD_MESH_FROM_STL_H

#include "../../OpenGLWindow/GLInstanceGraphicsShape.h"
#include <stdio.h>  //fopen
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Common/Interfaces/CommonFileIOInterface.h>
#include <string.h> //memcpy
struct MySTLTriangle
{
	float normal[3];
	float vertex0[3];
	float vertex1[3];
	float vertex2[3];
};

static GLInstanceGraphicsShape* LoadMeshFromSTL(tukk relativeFileName, struct CommonFileIOInterface* fileIO)
{
	GLInstanceGraphicsShape* shape = 0;

	i32 fileHandle = fileIO->fileOpen(relativeFileName, "rb");
	if (fileHandle>=0)
	{
		i32 size = 0;
		size = fileIO->getFileSize(fileHandle);
		{
			if (size>=0)
			{
				//drx3DWarning("Open STL file of %d bytes\n",size);
				tuk memoryBuffer = new char[size + 1];
				i32 actualBytesRead = fileIO->fileRead(fileHandle, memoryBuffer, size);
				if (actualBytesRead != size)
				{
					drx3DWarning("Error reading from file %s", relativeFileName);
				}
				else
				{
					i32 numTriangles = *(i32*)&memoryBuffer[80];

					if (numTriangles)
					{
						{
							//perform a sanity check instead of crashing on invalid triangles/STL files
							i32 expectedBinaryFileSize = numTriangles * 50 + 84;
							if (expectedBinaryFileSize != size)
							{
								delete[] memoryBuffer;
								fileIO->fileClose(fileHandle);
								return 0;
							}
						}
						shape = new GLInstanceGraphicsShape;
						//						b3AlignedObjectArray<GLInstanceVertex>*	m_vertices;
						//						i32				m_numvertices;
						//						b3AlignedObjectArray<i32>* 		m_indices;
						//						i32				m_numIndices;
						//						float			m_scaling[4];
						shape->m_scaling[0] = 1;
						shape->m_scaling[1] = 1;
						shape->m_scaling[2] = 1;
						shape->m_scaling[3] = 1;
						i32 index = 0;
						shape->m_indices = new b3AlignedObjectArray<i32>();
						shape->m_vertices = new b3AlignedObjectArray<GLInstanceVertex>();
						for (i32 i = 0; i < numTriangles; i++)
						{
							tuk curPtr = &memoryBuffer[84 + i * 50];
							MySTLTriangle tmp;
							memcpy(&tmp, curPtr, sizeof(MySTLTriangle));

							GLInstanceVertex v0, v1, v2;
							v0.uv[0] = v1.uv[0] = v2.uv[0] = 0.5;
							v0.uv[1] = v1.uv[1] = v2.uv[1] = 0.5;
							for (i32 v = 0; v < 3; v++)
							{
								v0.xyzw[v] = tmp.vertex0[v];
								v1.xyzw[v] = tmp.vertex1[v];
								v2.xyzw[v] = tmp.vertex2[v];
								v0.normal[v] = v1.normal[v] = v2.normal[v] = tmp.normal[v];
							}
							v0.xyzw[3] = v1.xyzw[3] = v2.xyzw[3] = 0.f;

							shape->m_vertices->push_back(v0);
							shape->m_vertices->push_back(v1);
							shape->m_vertices->push_back(v2);

							shape->m_indices->push_back(index++);
							shape->m_indices->push_back(index++);
							shape->m_indices->push_back(index++);
						}
					}
				}

				delete[] memoryBuffer;
			}
		}
		fileIO->fileClose(fileHandle);
	}
	if (shape)
	{
		shape->m_numIndices = shape->m_indices->size();
		shape->m_numvertices = shape->m_vertices->size();
	}
	return shape;
}

#endif  //LOAD_MESH_FROM_STL_H
