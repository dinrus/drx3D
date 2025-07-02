// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/RuntimeGraph.h>
#include <drx3D/Schema/Any.h>

namespace sxema
{
typedef SRuntimeResult (* RuntimeGraphNodeCallbackPtr)(SRuntimeContext& context, const SRuntimeActivationParams& activationParams);

struct IGraphNodeCompiler
{
	virtual ~IGraphNodeCompiler() {}

	virtual u32 GetGraphIdx() const = 0;
	virtual u32 GetGraphNodeIdx() const = 0;

	virtual void   BindCallback(RuntimeGraphNodeCallbackPtr pCallback) = 0;
	virtual void   BindData(const CAnyConstRef& value) = 0;
};
} // sxema
