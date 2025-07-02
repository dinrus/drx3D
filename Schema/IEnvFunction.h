// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IEnvElement.h>
#include <drx3D/Schema/EnumFlags.h>
#include <drx3D/Schema/GUID.h>

namespace sxema
{

// Forward declare classes.
class CAnyConstPtr;
class CCommonTypeDesc;
class CRuntimeParamMap;

enum class EEnvFunctionFlags
{
	None         = 0,
	Member       = BIT(0), // Function is member of object.
	Pure         = BIT(1), // Function does not affect global state.
	Const        = BIT(2), // Function is constant.
	Construction = BIT(3)  // Function can be called from construction graph.
};

typedef CEnumFlags<EEnvFunctionFlags> EnvFunctionFlags;

struct IEnvFunction : public IEnvElementBase<EEnvElementType::Function>
{
	virtual ~IEnvFunction() {}

	virtual bool                   Validate() const = 0;
	virtual EnvFunctionFlags       GetFunctionFlags() const = 0;
	virtual const CCommonTypeDesc* GetObjectTypeDesc() const = 0;
	virtual u32                 GetInputCount() const = 0;
	virtual u32                 GetInputId(u32 inputIdx) const = 0;
	virtual tukk            GetInputName(u32 inputIdx) const = 0;
	virtual tukk            GetInputDescription(u32 inputIdx) const = 0;
	virtual CAnyConstPtr           GetInputData(u32 inputIdx) const = 0;
	virtual u32                 GetOutputCount() const = 0;
	virtual u32                 GetOutputId(u32 outputIdx) const = 0;
	virtual tukk            GetOutputName(u32 outputIdx) const = 0;
	virtual tukk            GetOutputDescription(u32 outputIdx) const = 0;
	virtual CAnyConstPtr           GetOutputData(u32 outputIdx) const = 0;
	virtual void                   Execute(CRuntimeParamMap& params, uk pObject) const = 0;
};

DECLARE_SHARED_POINTERS(IEnvFunction)

} // sxema
