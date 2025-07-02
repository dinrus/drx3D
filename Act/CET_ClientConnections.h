// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Act/ClassRegistryReplicator.h>

void AddOnClientConnect(IContextEstablisher* pEst, EContextViewState state, bool isReset);
void AddOnClientEnteredGame(IContextEstablisher* pEst, EContextViewState state, bool isReset);