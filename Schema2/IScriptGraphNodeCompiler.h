// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/Any.h>
#include <drx3D/Schema2/BasicTypes.h>
#include <drx3D/Schema2/IRuntime.h>

namespace sxema2
{
	struct ILibClass;

	struct IScriptGraphNodeCompiler
	{
		virtual ~IScriptGraphNodeCompiler() {}

		virtual const ILibClass& GetLibClass() const = 0;
		virtual bool BindCallback(RuntimeNodeCallbackPtr pCallback) = 0;
		virtual bool BindAnyAttribute(u32 attributeId, const IAny& value) = 0;
		virtual bool BindAnyInput(u32 inputIdx, const IAny& value) = 0;
		virtual bool BindAnyOutput(u32 ouputIdx, const IAny& value) = 0;
		virtual bool BindAnyFunctionInput(u32 inputIdx, const IAny& value) = 0;
		virtual bool BindAnyFunctionOutput(u32 ouputIdx, const IAny& value) = 0;

		template <typename TYPE> bool BindAttribute(u32 attributeId, const TYPE& value)
		{
			return BindAnyAttribute(attributeId, MakeAny(value));
		}

		template <typename TYPE> bool BindInput(u32 inputIdx, const TYPE& value)
		{
			return BindAnyInput(inputIdx, MakeAny(value));
		}

		template <typename TYPE> bool BindOutput(u32 ouputIdx, const TYPE& value)
		{
			return BindAnyOutput(ouputIdx, MakeAny(value));
		}

		template <typename TYPE> bool BindFunctionInput(u32 inputIdx, const TYPE& value)
		{
			return BindAnyFunctionInput(inputIdx, MakeAny(value));
		}

		template <typename TYPE> bool BindFunctionOutput(u32 ouputIdx, const TYPE& value)
		{
			return BindAnyFunctionOutput(ouputIdx, MakeAny(value));
		}
	};
}
