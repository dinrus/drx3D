// Разработка 2018-2025 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Detonation Fire Mode Implementation

-------------------------------------------------------------------------
История:
- 11:9:2004   15:00 : Created by M�rcio Martins

*************************************************************************/
#ifndef __DETONATE_H__
#define __DETONATE_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include <drx3D/Game/Single.h>

class CDetonate :
	public CSingle
{
private:
	
	typedef CSingle BaseClass;
	struct ExplodeAction;
	struct DropRemoveAction;

public:
	DRX_DECLARE_GTI(CDetonate);

	CDetonate();
	virtual ~CDetonate();

	//IFireMode
	virtual void Update(float frameTime, u32 frameId) override;
	virtual void GetMemoryUsage(IDrxSizer * s) const override
	{ 
		s->AddObject(this, sizeof(*this));	
		CSingle::GetInternalMemoryUsage(s);		// collect memory of parent class
	}

	virtual void Activate(bool activate) override;

	virtual bool CanReload() const override;

	virtual bool CanFire(bool considerAmmo = true) const override;
	virtual void StartFire() override;

	virtual void NetShoot(const Vec3 &hit, i32 ph) override;
	//~IFireMode

	virtual void SetCanDetonate(bool canDet);
	virtual tukk GetCrosshair() const override;

	bool ClientCanDetonate() { return m_canDetonate; }

	void OutOfAmmoExplode();
	void DropRemoveItem();

protected:
	bool Detonate(bool net=false);
	void SelectLast();

	float	m_detonationTimer;
	bool	m_canDetonate;
};


#endif //__DETONATE_H__