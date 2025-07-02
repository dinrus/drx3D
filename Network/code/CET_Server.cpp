// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include <drx3D/Network/NetHelpers.h>
#include  <drx3D/Network/CET_Server.h>
#include  <drx3D/Network/NetChannel.h>
#include  <drx3D/Network/ContextView.h>
#include  <drx3D/Network/ClientContextView.h>

class CCET_PostSpawnObjects : public CCET_Base
{
public:
	CCET_PostSpawnObjects() { ++g_objcnt.postSpawnObjects; }
	~CCET_PostSpawnObjects() { --g_objcnt.postSpawnObjects; }

	tukk GetName()
	{
		return "PostInitObjects";
	}

	EContextEstablishTaskResult OnStep(SContextEstablishState& state)
	{
		CNetChannel* pChannel = (CNetChannel*) state.pSender;
		if (pChannel)
		{
			// TODO: tidy up
			SCOPED_GLOBAL_LOCK;
			pChannel->GetContextView()->ContextState()->GC_SendPostSpawnEntities(pChannel->GetContextView());
			return eCETR_Ok;
		}
		return eCETR_Failed;
	}
};

void AddPostSpawnObjects(IContextEstablisher* pEst, EContextViewState state)
{
	pEst->AddTask(state, new CCET_PostSpawnObjects);
}
