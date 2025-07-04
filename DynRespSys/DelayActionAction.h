// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************

   This action will delay the execution of the wrapped action.

   /************************************************************************/

#pragma once

#include <drx3D/DynRespSys/IDynamicResponseAction.h>

namespace DrxDRS
{
class CResponseInstance;

class DelayActionActionInstance final : public DRS::IResponseActionInstance
{
public:
	virtual ~DelayActionActionInstance() override;
	DelayActionActionInstance(float timeToDelay, DRS::IResponseActionSharedPtr pActionToDelay, CResponseInstance* pResponseInstance);

	//////////////////////////////////////////////////////////
	// IResponseActionInstance implementation
	virtual eCurrentState Update() override;
	virtual void          Cancel() override;
	//////////////////////////////////////////////////////////

private:
	CTimeValue                            m_delayFinishTime;
	DRS::IResponseActionSharedPtr         m_pDelayedAction;
	CResponseInstance*                    m_pResponseInstance;

	DRS::IResponseActionInstanceUniquePtr m_RunningInstance;    //this is the instance of the action that we have delayed, once started, we forward all update/cancel/Release calls
};
} //endns DrxDRS
