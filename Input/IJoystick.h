// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IJoystick.h
//  Version:     v1.00
//  Created:     7/8/2006 by MichaelS.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2005
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __IJOYSTICK_H__
#define __IJOYSTICK_H__

struct ISplineInterpolator;

class IJoystickChannel
{
public:
	// <interfuscator:shuffle>
	virtual ~IJoystickChannel(){}
	virtual void        AddRef() = 0;
	virtual void        Release() = 0;

	virtual tukk GetName() const = 0;

	virtual uk       GetTarget() = 0;

	virtual void        SetFlipped(bool flipped) = 0;
	virtual bool        GetFlipped() const = 0;

	virtual void        SetVideoMarkerOffset(float offset) = 0;
	virtual float       GetVideoMarkerOffset() const = 0;
	virtual void        SetVideoMarkerScale(float scale) = 0;
	virtual float       GetVideoMarkerScale() const = 0;

	virtual void        CleanupKeys(float fErrorMax) = 0;

	// TODO: Many of the above functions can be removed due to this function.
	virtual i32                  GetSplineCount() = 0;
	virtual ISplineInterpolator* GetSpline(i32 splineIndex) = 0;
	// </interfuscator:shuffle>
};

class IJoystick
{
public:
	typedef Vec3_tpl<u8> Color;

	enum ChannelType
	{
		ChannelTypeHorizontal = 0,
		ChannelTypeVertical,

		NumChannelTypes
	};

	// <interfuscator:shuffle>
	virtual ~IJoystick(){}
	virtual void              AddRef() = 0;
	virtual void              Release() = 0;

	virtual uint64            GetID() const = 0;

	virtual void              SetName(tukk szName) = 0;
	virtual tukk       GetName() = 0;

	virtual IJoystickChannel* GetChannel(ChannelType type) = 0;
	virtual void              SetChannel(ChannelType type, IJoystickChannel* pChannel) = 0;

	virtual const Vec2&      GetCentre() const = 0;
	virtual void             SetCentre(const Vec2& vCentre) = 0;
	virtual const Vec2&      GetDimensions() const = 0;
	virtual void             SetDimensions(const Vec2& vDimensions) = 0;

	virtual void             SetColor(const Color& colour) = 0;
	virtual IJoystick::Color GetColor() const = 0;
	// </interfuscator:shuffle>
};

class IJoystickSet
{
public:
	// <interfuscator:shuffle>
	virtual ~IJoystickSet(){}
	virtual void        AddRef() = 0;
	virtual void        Release() = 0;

	virtual void        SetName(tukk name) = 0;
	virtual tukk GetName() const = 0;
	virtual void        AddJoystick(IJoystick* pJoystick) = 0;
	virtual void        RemoveJoystick(IJoystick* pJoystick) = 0;
	virtual i32         GetJoystickCount() const = 0;
	virtual IJoystick*  GetJoystick(i32 index) = 0;
	virtual IJoystick*  GetJoystickAtPoint(const Vec2& vPosition) = 0;
	virtual IJoystick*  GetJoystickByID(uint64 id) = 0;
	virtual void        Serialize(XmlNodeRef& nodeJoysticks, bool bLoading) = 0;
	// </interfuscator:shuffle>
};

class IJoystickContext
{
public:
	// <interfuscator:shuffle>
	virtual ~IJoystickContext(){}

	//! Create a new joystick channel for an effector.
	virtual IJoystick*    CreateJoystick(uint64 id) = 0;

	virtual IJoystickSet* CreateJoystickSet() = 0;
	virtual IJoystickSet* LoadJoystickSet(tukk filename, bool bNoWarnings = false) = 0;
	// </interfuscator:shuffle>
};

#endif //__IJOYSTICK_H__
