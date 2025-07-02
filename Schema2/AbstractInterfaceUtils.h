// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Merge with EnvUtils/EnvRegistryUtils?

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/IAbstractInterface.h>
#include <drx3D/Schema2/IEnvRegistry.h>

namespace sxema2
{
	namespace AbstractInterfaceUtils
	{
		inline size_t FindFunction(const IAbstractInterface& abstractInterface, const SGUID& guid)
		{
			for(size_t functionIdx = 0, functionCount = abstractInterface.GetFunctionCount(); functionIdx < functionCount; ++ functionIdx)
			{
				IAbstractInterfaceFunctionConstPtr	pFunction = abstractInterface.GetFunction(functionIdx);
				if(pFunction->GetGUID() == guid)
				{
					return functionIdx;
				}
			}
			return INVALID_INDEX;
		}

		inline size_t FindFunctionInputByName(const IAbstractInterfaceFunction& abstractInterfaceFunction, tukk szInputName)
		{
			SXEMA2_SYSTEM_ASSERT(szInputName);
			if(szInputName)
			{
				for(size_t inputIdx = 0, inputCount = abstractInterfaceFunction.GetInputCount() ; inputIdx < inputCount ; ++ inputIdx)
				{
					if(strcmp(abstractInterfaceFunction.GetInputName(inputIdx), szInputName) == 0)
					{
						return inputIdx;
					}
				}
			}
			return INVALID_INDEX;
		}

		inline size_t FindFunctionOutputByName(const IAbstractInterfaceFunction& abstractInterfaceFunction, tukk szOutputName)
		{
			SXEMA2_SYSTEM_ASSERT(szOutputName);
			if(szOutputName)
			{
				for(size_t outputIdx = 0, outputCount = abstractInterfaceFunction.GetOutputCount() ; outputIdx < outputCount ; ++ outputIdx)
				{
					if(strcmp(abstractInterfaceFunction.GetOutputName(outputIdx), szOutputName) == 0)
					{
						return outputIdx;
					}
				}
			}
			return INVALID_INDEX;
		}
	}
}
