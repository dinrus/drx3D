// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RULERPOINT_H__
#define __RULERPOINT_H__

#include <drx3D/CoreX/Extension/DrxGUID.h>

class CRuler;

//! Ruler point helper - Defines a point for the ruler
class CRulerPoint
{
public:
	CRulerPoint();
	CRulerPoint& operator=(CRulerPoint const& other);

	void         Reset();
	void         Render(IRenderer* pRenderer);

	//! Set helpers
	void Set(const Vec3& vPos);
	void Set(CBaseObject* pObject);
	void SetHelperSettings(float scale, float trans);

	//! Returns is point has valid data in it (in use)
	bool IsEmpty() const;

	//! Helpers to get correct data out
	Vec3         GetPos() const;
	Vec3         GetMidPoint(const CRulerPoint& otherPoint) const;
	float        GetDistance(const CRulerPoint& otherPoint) const;
	CBaseObject* GetObject() const;

private:
	enum EType
	{
		eType_Invalid,
		eType_Point,
		eType_Object,

		eType_COUNT,
	};
	EType m_type;

	Vec3     m_vPoint;
	DrxGUID  m_ObjectGUID;
	float    m_sphereScale;
	float    m_sphereTrans;
};

#endif //__RULERPOINT_H__

