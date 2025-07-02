// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// DrxEngine Header File.
// Copyright (C), DinrusPro 3D, 1999-2014.

#pragma once

#include "Gizmos/Gizmo.h"

struct DisplayContext;
class CTrackViewAnimNode;
class CTrackViewTrack;

// Gizmo of Objects animation track.
class CTrackGizmo : public CGizmo
{
public:
	CTrackGizmo();
	~CTrackGizmo();

	// Overrides from CGizmo
	virtual void GetWorldBounds(AABB& bbox);
	virtual void Display(DisplayContext& dc);
	virtual bool HitTest(HitContext& hc);
	// TODO implement this
	virtual bool MouseCallback(IDisplayViewport* view, EMouseEvent event, CPoint& point, i32 nFlags) { return false; }

	void         SetMatrix(const Matrix34& tm);
	void         SetAnimNode(CTrackViewAnimNode* pNode);
	void         DrawAxis(DisplayContext& dc, const Vec3& pos);
	void         DrawKeys(DisplayContext& dc, CTrackViewTrack* pTrack, CTrackViewTrack* pKeysTrack);

private:
	mutable Matrix34 m_matrix;
	CTrackViewAnimNode* m_pAnimNode;
	AABB                m_worldBbox;
};

