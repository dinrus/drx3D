#ifndef DRX3D_COLLISION_COMMON_H
#define DRX3D_COLLISION_COMMON_H

///Общий файл-заголовочник, включающий Детекцию Столкновений drx3D

///drx3D-определения CollisionWorld и CollisionObject
#include <drx3D/Physics/Collision/Dispatch/CollisionWorld.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>

///Фигуры Столкновений
#include <drx3D/Physics/Collision/Shapes/BoxShape.h>
#include <drx3D/Physics/Collision/Shapes/SphereShape.h>
#include <drx3D/Physics/Collision/Shapes/CapsuleShape.h>
#include <drx3D/Physics/Collision/Shapes/CylinderShape.h>
#include <drx3D/Physics/Collision/Shapes/ConeShape.h>
#include <drx3D/Physics/Collision/Shapes/StaticPlaneShape.h>
#include <drx3D/Physics/Collision/Shapes/ConvexHullShape.h>
#include <drx3D/Physics/Collision/Shapes/TriangleMesh.h>
#include <drx3D/Physics/Collision/Shapes/ConvexTriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/BvhTriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/ScaledBvhTriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/TriangleMeshShape.h>
#include <drx3D/Physics/Collision/Shapes/TriangleIndexVertexArray.h>
#include <drx3D/Physics/Collision/Shapes/CompoundShape.h>
#include <drx3D/Physics/Collision/Shapes/TetrahedronShape.h>
#include <drx3D/Physics/Collision/Shapes/EmptyShape.h>
#include <drx3D/Physics/Collision/Shapes/MultiSphereShape.h>
#include <drx3D/Physics/Collision/Shapes/UniformScalingShape.h>

///Детектор Столкновений Узкой Фазы
#include <drx3D/Physics/Collision/Dispatch/SphereSphereCollisionAlgorithm.h>

//#include <drx3D/Physics/Collision/Dispatch/SphereBoxCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/Dispatch/DefaultCollisionConfiguration.h>

///Диспетчирование и генерация сталкивающихся пар (Широкая Фаза)
#include <drx3D/Physics/Collision/Dispatch/CollisionDispatcher.h>
#include <drx3D/Physics/Collision/BroadPhase/SimpleBroadphase.h>
#include <drx3D/Physics/Collision/BroadPhase/AxisSweep3.h>
#include <drx3D/Physics/Collision/BroadPhase/DbvtBroadphase.h>

///Математическая библиотека и Утилиты
#include <drx3D/Maths/Linear/Quat.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/DefaultMotionState.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <drx3D/Maths/Linear/Serializer.h>

#endif  //DRX3D_COLLISION_COMMON_H
