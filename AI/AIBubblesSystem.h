// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/IAIBubblesSystem.h>

bool AIQueueBubbleMessage(tukk messageName, const EntityId entityID,
                          tukk message, u32 flags, float duration = 0, SAIBubbleRequest::ERequestType requestType = SAIBubbleRequest::eRT_ErrorMessage);
