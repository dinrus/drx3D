// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PhysTool_h__
#define __PhysTool_h__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "EditTool.h"

class CPhysPullTool : public CEditTool
{
public:
	DECLARE_DYNAMIC(CPhysPullTool)

	CPhysPullTool();
	static void RegisterCVars();

	// Overrides from CEditTool
	virtual string GetDisplayName() const override { return "Pull Physics"; }
	virtual void   Display(DisplayContext& dc) override;
	virtual bool   MouseCallback(CViewport* view, EMouseEvent event, CPoint& point, i32 flags) override;

	static float cv_HitVel0;
	static float cv_HitVel1;
	static float cv_HitProjMass;
	static float cv_HitProjVel0;
	static float cv_HitProjVel1;
	static float cv_HitExplR;
	static float cv_HitExplPress0;
	static float cv_HitExplPress1;

protected:
	virtual ~CPhysPullTool();
	void DeleteThis() { delete this; };
	void UpdateAttachPos(const struct SMiniCamera& cam, const CPoint& point);
	static i32k m_idConstr = 2015;

	IPhysicalEntity* m_pEntPull, * m_pEntAttach, * m_pBullet, * m_pRope;
	CPoint           m_lastMousePos;
	Vec3             m_lastAttachPos, m_locAttachPos;
	i32              m_partid;
	float            m_timeMove, m_timeHit, m_timeBullet;
	float            m_attachDist;
	i32 						 m_nAttachPoints;
	HCURSOR          m_hcur[4];
};

#endif // __PhysTool_h__

