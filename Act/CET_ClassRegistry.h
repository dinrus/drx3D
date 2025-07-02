// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CET_CLASSREGISTRY_H__
#define __CET_CLASSREGISTRY_H__

#pragma once

#include <drx3D/Act/ClassRegistryReplicator.h>

void AddRegisterAllClasses(IContextEstablisher* pEst, EContextViewState state, CClassRegistryReplicator* pRep);
void AddSendClassRegistration(IContextEstablisher* pEst, EContextViewState state, CClassRegistryReplicator* pRep, std::vector<SSendableHandle>** ppWaitFor);
void AddSendClassHashRegistration(IContextEstablisher* pEst, EContextViewState state, CClassRegistryReplicator* pRep, std::vector<SSendableHandle>** ppWaitFor);

#endif
