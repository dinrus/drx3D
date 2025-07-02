// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ITransformManipulator_h__
#define __ITransformManipulator_h__
#pragma once

#include "DrxSandbox/DrxSignal.h"

//////////////////////////////////////////////////////////////////////////
// ITransformManipulator implementation.
//////////////////////////////////////////////////////////////////////////
struct EDITOR_COMMON_API ITransformManipulator
{
	virtual Matrix34 GetTransform() const = 0;
	virtual void     SetCustomTransform(bool on, const Matrix34& m) = 0;
	// invalidate. Manipulator will request transform from owner during next draw
	virtual void     Invalidate() = 0;

	CDrxSignal <void (IDisplayViewport*, ITransformManipulator*, const Vec2i&, i32)> signalBeginDrag;
	CDrxSignal <void (IDisplayViewport*, ITransformManipulator*, const Vec2i&, const Vec3&, i32)> signalDragging;
	CDrxSignal <void (IDisplayViewport*, ITransformManipulator*)> signalEndDrag;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ITransform is an interface for classes that own a transform manipulator. Basically it includes callbacks so that transform manipulators
// can get the position and orientation of its components when it is tagged for an update
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct EDITOR_COMMON_API ITransformManipulatorOwner
{
	// called at update time if reference coordinate system is  local or parent. Return true if there's a valid matrix else
	// world orientation (identity matrix) is used instead
	virtual bool GetManipulatorMatrix(RefCoordSys coordSys, Matrix34& tm) { return false; }
	// called at update time when the transform manipulator is tagged as invalid
	virtual void GetManipulatorPosition(Vec3& position) = 0;
	// called during display and hit test of manipulator. This way owners can deactivate their manipulator under some circumstances
	virtual bool IsManipulatorVisible() { return true; };
};
#endif // __ITransformManipulator_h__

