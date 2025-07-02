#ifndef D3_SAP_AABB_H
#define D3_SAP_AABB_H

#include <drx3D/Common/b3Scalar.h>
#include <drx3D/Physics/Collision/BroadPhase/shared/b3Aabb.h>

///just make sure that the b3Aabb is 16-byte aligned
D3_ATTRIBUTE_ALIGNED16(struct)
b3SapAabb : public b3Aabb{

			};

#endif  //D3_SAP_AABB_H
