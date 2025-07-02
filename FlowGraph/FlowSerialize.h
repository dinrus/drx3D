// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FLOWSERIALIZE_H__
#define __FLOWSERIALIZE_H__

#pragma once

#include <drx3D/FlowGraph/IFlowSystem.h>

bool   SetFromString(TFlowInputData& value, tukk str);
string ConvertToString(const TFlowInputData& value);
bool   SetAttr(XmlNodeRef node, tukk attr, const TFlowInputData& value);

#endif
