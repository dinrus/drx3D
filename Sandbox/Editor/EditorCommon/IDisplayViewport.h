// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct DisplayContext;
struct AABB;
class CBaseObjectsCache;
class CPoint;

// Viewport functionality required for DisplayContext
struct IDisplayViewport
{
	virtual void               Update() = 0;
	virtual float              GetScreenScaleFactor(const Vec3& position) const = 0;
	virtual bool               HitTestLine(const Vec3& lineP1, const Vec3& lineP2, const CPoint& hitpoint, i32 pixelRadius, float* pToCameraDistance = 0) const = 0;
	virtual CBaseObjectsCache* GetVisibleObjectsCache() = 0;

	enum EAxis
	{
		AXIS_NONE,
		AXIS_X,
		AXIS_Y,
		AXIS_Z
	};
	virtual void            GetPerpendicularAxis(EAxis* axis, bool* is2D) const = 0;

	virtual const Matrix34& GetViewTM() const = 0;
	virtual POINT           WorldToView(const Vec3& worldPoint) const = 0;
	virtual Vec3            WorldToView3D(const Vec3& worldPoint, i32 flags = 0) const = 0;
	virtual Vec3            ViewToWorld(POINT vp, bool* collideWithTerrain = 0, bool onlyTerrain = false, bool bSkipVegetation = false, bool bTestRenderMesh = false) const = 0;
	virtual void            ViewToWorldRay(POINT vp, Vec3& raySrc, Vec3& rayDir) const = 0;
	virtual Vec3            ViewToAxisConstraint(POINT& point, Vec3 axis, Vec3 origin) const = 0;
	virtual Vec3            ViewDirection() const = 0;
	virtual Vec3            UpViewDirection() const = 0;
	virtual Vec3            CameraToWorld(Vec3 worldPoint) const = 0;
	virtual float           GetGridStep() const = 0;

	virtual float           GetAspectRatio() const = 0;

	virtual bool            IsBoundsVisible(const AABB& box) const = 0;

	virtual void            ScreenToClient(POINT* pt) const = 0;
	virtual void            GetDimensions(i32* width, i32* height) const = 0;
	virtual Vec3			MapViewToCP(CPoint point, i32 axis, bool aSnapToTerrain = false, float aTerrainOffset = 0.f) = 0;
	virtual void            SetConstructionMatrix(const Matrix34& xform) = 0;

	virtual Vec3            SnapToGrid(Vec3 vec) = 0;
	virtual void            SetCursor(QCursor* hCursor) = 0;
};

