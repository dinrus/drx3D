// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   История:
   - 17:12:2007   11:29 : Created by Mieszko Zielinski

*************************************************************************/
#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/BlackBoard.h>

CBlackBoard::CBlackBoard()
{
	m_BB.Create(gEnv->pSystem->GetIScriptSystem());
}

void CBlackBoard::SetFromScript(SmartScriptTable& sourceBB)
{
	m_BB->Clone(sourceBB, true);
}
