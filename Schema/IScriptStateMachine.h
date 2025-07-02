// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptElement.h>

namespace sxema
{
// #SchematycTODO : Consider making tasks and state machines different element types! Alternatively can we just use a node to stop/start state machines?
enum class EScriptStateMachineLifetime
{
	Persistent,
	Task
};

struct IScriptStateMachine : public IScriptElementBase<EScriptElementType::StateMachine>
{
	virtual ~IScriptStateMachine() {}

	virtual EScriptStateMachineLifetime GetLifetime() const = 0;
	virtual DrxGUID                       GetContextGUID() const = 0;
	virtual DrxGUID                       GetPartnerGUID() const = 0;
};
} // sxema
