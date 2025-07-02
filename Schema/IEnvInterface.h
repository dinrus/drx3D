// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Rename IEnvInterfaceFunction to IEnvFunctionSignature/IEnvFunctionPrototype and move to separate header?

#pragma once

#include <drx3D/Schema/IEnvElement.h>
#include <drx3D/Schema/Assert.h>

namespace sxema
{

// Forward declare classes.
class CAnyConstPtr;

struct IEnvInterfaceFunction : public IEnvElementBase<EEnvElementType::InterfaceFunction>
{
	virtual ~IEnvInterfaceFunction() {}

	virtual u32       GetInputCount() const = 0;
	virtual tukk  GetInputName(u32 inputIdx) const = 0;
	virtual tukk  GetInputDescription(u32 inputIdx) const = 0;
	virtual CAnyConstPtr GetInputValue(u32 inputIdx) const = 0;
	virtual u32       GetOutputCount() const = 0;
	virtual tukk  GetOutputName(u32 outputIdx) const = 0;
	virtual tukk  GetOutputDescription(u32 outputIdx) const = 0;
	virtual CAnyConstPtr GetOutputValue(u32 outputIdx) const = 0;
};

struct IEnvInterface : public IEnvElementBase<EEnvElementType::Interface>
{
	virtual ~IEnvInterface() {}
};

} // sxema
