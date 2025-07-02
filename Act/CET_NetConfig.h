// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CET_NETCONFIG_H__
#define __CET_NETCONFIG_H__

#pragma once

void AddEstablishedContext(IContextEstablisher* pEst, EContextViewState state, i32 token);
void AddDeclareWitness(IContextEstablisher* pEst, EContextViewState state);
void AddDelegateAuthorityToClientActor(IContextEstablisher* pEst, EContextViewState state);
void AddClearPlayerIds(IContextEstablisher* pEst, EContextViewState state);

#endif
