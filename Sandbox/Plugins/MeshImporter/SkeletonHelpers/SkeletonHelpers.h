// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/BaseTypes.h>  // Fixed-size integer types.
#include <drx3D/CoreX/Math/Drx_Math.h>

struct ICharacterInstance;

class QViewport;

extern i32k kInvalidJointId;

//! PickJoint picks joint under mouse cursor.
//! \param x, y Screen coordinates of mouse cursor.
//! \return Returns ID of joint under mouse cursor, or -1, if there is no such joint.
i32 PickJoint(const ICharacterInstance& charIns, QViewport& viewPort, float x, float y);

//! For each joint, draw a wireframe box and axes system.
void DrawJoints(const ICharacterInstance& charIns);

void DrawHighlightedJoints(const ICharacterInstance& charIns, std::vector<float>& heat, const Vec3& eye);
