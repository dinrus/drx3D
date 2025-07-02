// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/GUID.h>
#include <drx3D/Schema2/IAny.h>
#include <drx3D/Schema2/EnvTypeId.h>

namespace sxema2
{
	enum class EEnvFunctionFlags
	{
		None   = 0,
		Member = BIT(0),
		Const  = BIT(1),
		//Pure   = BIT(2)
	};

	DECLARE_ENUM_CLASS_FLAGS(EEnvFunctionFlags)

	struct SEnvFunctionResult {};
	
	struct SEnvFunctionContext {};

	static u32k s_maxEnvFunctionParams = 10;

	typedef const IAny* EnvFunctionInputs[s_maxEnvFunctionParams];
	typedef IAny*       EnvFunctionOutputs[s_maxEnvFunctionParams];

	struct IEnvFunctionDescriptor
	{
		virtual ~IEnvFunctionDescriptor() {}

		virtual SGUID GetGUID() const = 0;
		virtual tukk GetName() const = 0;
		virtual tukk GetNamespace() const = 0;
		virtual tukk GetFileName() const = 0;
		virtual tukk GetAuthor() const = 0;
		virtual tukk GetDescription() const = 0;
		virtual EEnvFunctionFlags GetFlags() const = 0;
		virtual EnvTypeId GetContextTypeId() const = 0;
		virtual u32 GetInputCount() const = 0;
		virtual u32 GetInputId(u32 inputIdx) const = 0;
		virtual tukk GetInputName(u32 inputIdx) const = 0;
		virtual tukk GetInputDescription(u32 inputIdx) const = 0;
		virtual IAnyConstPtr GetInputValue(u32 inputIdx) const = 0;
		virtual u32 GetOutputCount() const = 0;
		virtual u32 GetOutputId(u32 outputIdx) const = 0;
		virtual tukk GetOutputName(u32 outputIdx) const = 0;
		virtual tukk GetOutputDescription(u32 outputIdx) const = 0;
		virtual IAnyConstPtr GetOutputValue(u32 outputIdx) const = 0;
		virtual SEnvFunctionResult Execute(const SEnvFunctionContext& context, uk pObject, const EnvFunctionInputs& inputs, const EnvFunctionOutputs& outputs) const = 0;
	};
}
