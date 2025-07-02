// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Input/IJoystick.h>
#include <drx3D/CoreX/smartptr.h>

struct IFacialAnimChannel;

class IFacialJoystickSerializeContext
{
public:
	virtual ~IFacialJoystickSerializeContext(){}
	virtual IFacialAnimChannel* FindChannel(tukk szName) = 0;
};

class CFacialJoystickChannel : public IJoystickChannel
{
public:
	CFacialJoystickChannel(const string& channelName);
	CFacialJoystickChannel(IFacialAnimChannel* pChannel);

	// IJoystickChannel
	virtual void                 AddRef();
	virtual void                 Release();

	virtual tukk          GetName() const;

	virtual uk                GetTarget();
	virtual i32                  GetSplineCount();
	virtual ISplineInterpolator* GetSpline(i32 splineIndex);

	virtual void                 SetFlipped(bool flipped);
	virtual bool                 GetFlipped() const;

	virtual void                 SetVideoMarkerOffset(float offset);
	virtual float                GetVideoMarkerOffset() const;
	virtual void                 SetVideoMarkerScale(float scale);
	virtual float                GetVideoMarkerScale() const;

	virtual void                 CleanupKeys(float fErrorMax);

	tukk                  GetPath() const;

	IFacialAnimChannel*          GetFacialChannel();

	void                         Bind(IFacialJoystickSerializeContext* pContext);

private:
	IFacialAnimChannel* m_pChannel;
	string              m_channelName;
	i32                 m_refCount;
	bool                m_bFlipped;
	float               m_videoMarkerOffset;
	float               m_videoMarkerScale;
};

class CFacialJoystick : public IJoystick
{
public:
	CFacialJoystick(uint64 id);

	// IJoystick
	virtual void              AddRef();
	virtual void              Release();

	virtual uint64            GetID() const;

	virtual void              SetName(tukk szName);
	virtual tukk       GetName();

	virtual IJoystickChannel* GetChannel(ChannelType type);
	virtual void              SetChannel(ChannelType type, IJoystickChannel* pChannel);

	virtual const Vec2&      GetCentre() const;
	virtual void             SetCentre(const Vec2& vCentre);
	virtual const Vec2&      GetDimensions() const;
	virtual void             SetDimensions(const Vec2& vDimensions);

	CFacialJoystickChannel*  GetDerivedChannel(ChannelType type);

	void                     Bind(IFacialJoystickSerializeContext* pContext);

	virtual void             SetColor(const Color& colour);
	virtual IJoystick::Color GetColor() const;

private:
	_smart_ptr<CFacialJoystickChannel> m_channels[NumChannelTypes];
	uint64                             m_id;
	Vec2                               m_vCentre;
	Vec2                               m_vDimensions;
	i32                                m_refCount;
	string                             m_name;
	Color                              m_colour;
};

class CFacialJoystickSet : public IJoystickSet
{
public:
	CFacialJoystickSet();

	void HandleRemovedChannel(IFacialAnimChannel* pChannel);

	// IJoystickSet
	virtual void        AddRef();
	virtual void        Release();

	virtual void        SetName(tukk name);
	virtual tukk GetName() const;
	virtual void        AddJoystick(IJoystick* pJoystick);
	virtual void        RemoveJoystick(IJoystick* pJoystick);
	virtual i32         GetJoystickCount() const;
	virtual IJoystick*  GetJoystick(i32 index);
	virtual IJoystick*  GetJoystickAtPoint(const Vec2& vPosition);
	virtual IJoystick*  GetJoystickByID(uint64 id);

	virtual void        Serialize(XmlNodeRef& node, bool bLoading);

	void                Bind(IFacialJoystickSerializeContext* pContext);

private:
	typedef std::vector<_smart_ptr<CFacialJoystick>> JoystickContainer;
	JoystickContainer m_joysticks;
	i32               m_refCount;
	string            m_name;
};
