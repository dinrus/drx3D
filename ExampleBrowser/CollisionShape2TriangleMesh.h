#ifndef COLLISION_SHAPE_2_GRAPHICS_H
#define COLLISION_SHAPE_2_GRAPHICS_H

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Transform2.h>
class CollisionShape;

void CollisionShape2TriangleMesh(CollisionShape* collisionShape,
                                   const Transform2& parentTransform,
                                   AlignedObjectArray<Vec3>& vertexPositions,
                                    AlignedObjectArray<Vec3>& vertexNormals,
                                     AlignedObjectArray<i32>& indicesOut);

#endif  //COLLISION_SHAPE_2_GRAPHICS_H
