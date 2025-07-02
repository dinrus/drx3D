// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Script Binding for Item

   -------------------------------------------------------------------------
   История:
   - 17:12:2007   11:49 : Created by Mieszko Zielinski

*************************************************************************/
#ifndef __SCRIPTBLACKCBOARD_H__
#define __SCRIPTBLACKCBOARD_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>
#include <drx3D/AI/IBlackBoard.h>

class CBlackBoard : public IBlackBoard
{
public:
	virtual ~CBlackBoard(){}

	CBlackBoard();

	virtual SmartScriptTable& GetForScript() { return m_BB; }
	virtual void              SetFromScript(SmartScriptTable& sourceBB);
	virtual void              Clear()        { m_BB->Clear(); }

private:
	SmartScriptTable m_BB;
};

#endif // __SCRIPTBLACKCBOARD_H__
