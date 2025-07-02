// Разработка 2018-2025 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Tracer Upr

-------------------------------------------------------------------------
История:
- 17:1:2006   11:12 : Created by M�rcio Martins

*************************************************************************/
#ifndef __TRACERMANAGER_H__
#define __TRACERMANAGER_H__

#if _MSC_VER > 1000
# pragma once
#endif

enum
{
	kTracerFlag_scaleToDistance				= BIT(0),
	kTracerFlag_useGeometry						=	BIT(1),
	kTracerFlag_active								= BIT(2),
	kTracerFlag_updateDestFromBullet	= BIT(3),
  kTracerFlag_dontTranslate         = BIT(4)
};


class CTracer
{
	friend class CTracerUpr;
public:
	CTracer();
	CTracer(const Vec3 &pos);
	virtual ~CTracer();

	void Reset(const Vec3 &pos, const Vec3 &dest);
	void CreateEntity();
	void DeleteEntity();
	void SetGeometry(tukk name, float scale);
	void SetEffect(tukk name, float scale);
	void SetLifeTime(float lifeTime);
	bool Update(float frameTime, const Vec3 &campos, const float fovScale);
	float GetAge() const;
	void GetMemoryStatistics(IDrxSizer * s) const;

private:
	float				m_speed;
	Vec3				m_pos;
	Vec3				m_dest;
	Vec3				m_startingPos;
	float				m_age;
	float				m_lifeTime;
	float				m_fadeOutTime;
	float				m_startFadeOutTime;	
	float				m_scale;
	float				m_geometryOpacity;
	float				m_slideFrac;
	EntityId		m_entityId;
	EntityId		m_boundToBulletId;
	u16			m_tracerFlags;
	int8				m_effectSlot;
	int8        m_geometrySlot;
};


class CTracerUpr
{
	const static i32 kMaxNumTracers = 96;
	
public:
	CTracerUpr();
	virtual ~CTracerUpr();

	struct STracerParams
	{
		STracerParams()
		{
			geometry = NULL;
			effect = NULL;
			speed = 0.0f;
			lifetime = 0.0f;
			delayBeforeDestroy = 0.0f;
			scaleToDistance = false;
			position.Set(0.0f,0.0f,0.0f);
			destination.Set(0.0f,0.0f,0.0f);
			startFadeOutTime = 0.0f;
			scale = 1.0f;
			geometryOpacity = 0.99f;
			slideFraction = 0.f;
			updateDestPosFromBullet = false;
      dontTranslate = false;
		}
		tukk geometry;
		tukk effect;
		Vec3				position;
		Vec3				destination;
		float				speed;
		float				lifetime;
		float				delayBeforeDestroy;
		float				startFadeOutTime;
		float				scale;
		float				geometryOpacity;
		float				slideFraction;
		bool				scaleToDistance;
		bool				updateDestPosFromBullet;
    bool        dontTranslate;
	};

	i32 EmitTracer(const STracerParams &params, const EntityId projectileId = 0);
	void OnBoundProjectileDestroyed(i32k tracerIdx, const EntityId projectileId, const Vec3& newEndTracerPosition);

	void Update(float frameTime);
	void Reset();
	void ClearCurrentActiveTracers();
	void GetMemoryStatistics(IDrxSizer *);

private:

	CTracer					m_tracerPool[kMaxNumTracers + 4];

	i16						m_numActiveTracers;
};


#endif //__TRACERMANAGER_H__