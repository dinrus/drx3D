// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

struct IPhyicalWorld;
class CCamera;

//! Этот объект реализует отображение отладочной информации через IPhysRenderer (DinrusXPhys) и IRenderer (XRender).
struct IPhysicsDebugRenderer
{
	// <interfuscator:shuffle>
	virtual ~IPhysicsDebugRenderer(){}
	virtual void UpdateCamera(const CCamera& camera) = 0;
	virtual void DrawAllHelpers(IPhysicalWorld* world) = 0;
	virtual void DrawEntityHelpers(IPhysicalEntity* entity, i32 helperFlags) = 0;
	virtual void Flush(float dt) = 0;
	// </interfuscator:shuffle>
};

//! \endcond