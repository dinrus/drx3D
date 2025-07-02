// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: 
		Interface for a class that receives events when pc prematch state changes, etc.
	-------------------------------------------------------------------------
	История:
	- 14:08:2012  : Created by Jonathan Bunner

*************************************************************************/

#ifndef _IGAME_RULES_PREMATCH_LISTENER_H_
#define _IGAME_RULES_PREMATCH_LISTENER_H_

#if _MSC_VER > 1000
# pragma once
#endif

#if USE_PC_PREMATCH
class IGameRulesPrematchListener
{
public:
	virtual ~IGameRulesPrematchListener() {}
	virtual void OnPrematchEnd() = 0;
};
#endif //#USE_PC_PREMATCH

#endif //_IGAME_RULES_PREMATCH_LISTENER_H_
