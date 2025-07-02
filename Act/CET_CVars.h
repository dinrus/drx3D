// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CET_CVARS_H__
#define __CET_CVARS_H__

#pragma once

#include "GameChannel.h"
#include "GameClientChannel.h"

class CGameServerChannel;
void AddCVarSync(IContextEstablisher* pEst, EContextViewState state, CGameServerChannel* pChannel);

class CBatchSyncVarsSink : public IConsoleVarSink
{
public:
	CBatchSyncVarsSink(CGameServerChannel* pChannel);

	// IConsoleVarSink
	virtual bool OnBeforeVarChange(ICVar* pVar, tukk sNewValue);
	virtual void OnAfterVarChange(ICVar* pVar);
	virtual void OnVarUnregister(ICVar* pVar) {}

private:
	CGameServerChannel*                m_pChannel;
	SSendableHandle                    m_consoleVarSendable;
	SClientBatchConsoleVariablesParams m_params;
};

#endif
