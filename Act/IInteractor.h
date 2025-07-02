// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Interactor interface.

   -------------------------------------------------------------------------
   История:
   - 26:6:2006   17:06 : Created by Márcio Martins

*************************************************************************/
#ifndef __IINTERACTOR_H__
#define __IINTERACTOR_H__

#pragma once

struct IInteractor : public IGameObjectExtension
{
	virtual bool IsUsable(EntityId entityId) const = 0;
	virtual bool IsLocked() const = 0;
	virtual i32  GetLockIdx() const = 0;
	virtual i32  GetLockedEntityId() const = 0;
	virtual void SetQueryMethods(tuk pMethods) = 0;
	virtual i32  GetOverEntityId() const = 0;
	virtual i32  GetOverSlotIdx() const = 0;
};

#endif // __IINTERACTOR_H__
