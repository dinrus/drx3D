// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:	Blood splat effect

   -------------------------------------------------------------------------
   История:
   - 17:01:2006:		Created by Marco Koegler

*************************************************************************/
#ifndef __BLOODSPLATS_H__
#define __BLOODSPLATS_H__

#include "Effect.h"

class CBloodSplats : public CEffect
{
public:

	void         Init(i32 type, float maxTime);
	// IEffect overrides
	virtual bool Update(float delta);
	virtual bool OnActivate();
	virtual bool OnDeactivate();
	virtual void GetMemoryUsage(IDrxSizer* s) const;
	// ~IEffect overrides

private:
	i32   m_type;           // 0 human, 1 alien
	float m_maxTime;        // maximum time until effect expires
	float m_currentTime;    // current time
};

#endif //__BLOODSPLATS_H__
