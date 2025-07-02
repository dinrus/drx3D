#ifndef LOAD_MESH_FROM_COLLADA_H
#define LOAD_MESH_FROM_COLLADA_H

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include "../../OpenGLWindow/GLInstanceGraphicsShape.h"
#include "ColladaGraphicsInstance.h"

void LoadMeshFromCollada(tukk relativeFileName,
						 AlignedObjectArray<GLInstanceGraphicsShape>& visualShapes,
						 AlignedObjectArray<ColladaGraphicsInstance>& visualShapeInstances,
						 Transform2& upAxisTrans,
						 float& unitMeterScaling,
						 i32 clientUpAxis,
						 struct CommonFileIOInterface* fileIO);

//#define COMPARE_WITH_ASSIMP
#ifdef COMPARE_WITH_ASSIMP
void LoadMeshFromColladaAssimp(tukk relativeFileName,
							   AlignedObjectArray<GLInstanceGraphicsShape>& visualShapes,
							   AlignedObjectArray<ColladaGraphicsInstance>& visualShapeInstances,
							   Transform2& upAxisTrans,
							   float& unitMeterScaling);
#endif  //COMPARE_WITH_ASSIMP

#endif  //LOAD_MESH_FROM_COLLADA_H
