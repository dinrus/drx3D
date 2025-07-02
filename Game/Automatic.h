// Разработка 2018-2025 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: C++ Item Implementation

-------------------------------------------------------------------------
История:
- 11:9:2004   15:00 : Created by M�rcio Martins

*************************************************************************/
#ifndef __AUTOMATIC_H__
#define __AUTOMATIC_H__

#if _MSC_VER > 1000
# pragma once
#endif


#include <drx3D/Game/Single.h>

class CAutomatic : public CSingle
{
public:
	DRX_DECLARE_GTI(CAutomatic);
	
	CAutomatic();
	virtual ~CAutomatic();

	// CSingle
	virtual void StartReload(i32 zoomed) override;

	virtual void GetMemoryUsage(IDrxSizer * s) const override;
	void GetInternalMemoryUsage(IDrxSizer * s) const;
	virtual void Update(float frameTime, u32 frameId) override;
	virtual void StartFire() override;
	virtual void StopFire() override;
	// ~CSingle

private:
	
	typedef CSingle BaseClass;

};


#endif //__AUTOMATIC_H__