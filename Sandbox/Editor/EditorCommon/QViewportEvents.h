// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class QViewport;

struct SMouseEvent
{
	enum EType
	{
		TYPE_NONE,
		TYPE_PRESS,
		TYPE_RELEASE,
		TYPE_MOVE
	};

	enum EButton
	{
		BUTTON_NONE,
		BUTTON_LEFT,
		BUTTON_RIGHT,
		BUTTON_MIDDLE
	};

	EType      type;
	i32        x;
	i32        y;
	EButton    button;
	bool       shift;
	bool       control;
	QViewport* viewport;

	SMouseEvent()
		: type(TYPE_NONE)
		, x(INT_MIN)
		, y(INT_MIN)
		, button(BUTTON_NONE)
		, viewport(0)
		, shift(false)
		, control(false)
	{
	}
};

struct SSelectionID
{
};

struct SInteractionEvent
{
	enum EType
	{
		TYPE_NONE,
		TYPE_ENTER,
		TYPE_LEAVE,
		TYPE_DRAG
	};

	SSelectionID selection;
	Vec3         start;
	Vec3         end;
};

struct SKeyEvent
{
	enum EType
	{
		TYPE_NONE,
		TYPE_PRESS,
		TYPE_RELEASE
	};

	EType type;
	i32   key;

	SKeyEvent()
		: type(TYPE_NONE)
	{
	}
};

