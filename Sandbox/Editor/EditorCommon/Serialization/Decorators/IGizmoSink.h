// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace Serialization {

struct LocalPosition;
struct LocalFrame;
struct LocalOrientation;

struct GizmoFlags
{
	bool visible;
	bool selected;

	GizmoFlags() : visible(true), selected(false) {}
};

struct IGizmoSink
{
	virtual i32  CurrentGizmoIndex() const = 0;
	virtual i32  Write(const LocalPosition&, const GizmoFlags& flags, ukk handle) = 0;
	virtual i32  Write(const LocalOrientation&, const GizmoFlags& flags, ukk handle) = 0;
	virtual i32  Write(const LocalFrame&, const GizmoFlags& flags, ukk handle) = 0;
	virtual void SkipRead() = 0;
	virtual bool Read(LocalPosition* position, GizmoFlags* flags, ukk handle) = 0;
	virtual bool Read(LocalOrientation* position, GizmoFlags* flags, ukk handle) = 0;
	virtual bool Read(LocalFrame* position, GizmoFlags* flags, ukk handle) = 0;
	virtual void Reset(ukk handle) = 0;
};

}

