// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/EnvElementBase.h>
#include <drx3D/Schema/IEnvInterface.h>
#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/SharedString.h>

#define SXEMA_MAKE_ENV_INTERFACE(guid, name)          sxema::EnvInterface::MakeShared(guid, name, SXEMA_SOURCE_FILE_INFO)
#define SXEMA_MAKE_ENV_INTERFACE_FUNCTION(guid, name) sxema::EnvInterfaceFunction::MakeShared(guid, name, SXEMA_SOURCE_FILE_INFO)

namespace sxema
{

class CEnvInterface : public CEnvElementBase<IEnvInterface>
{
public:

	inline CEnvInterface(const DrxGUID& guid, tukk szName, const SSourceFileInfo& sourceFileInfo)
		: CEnvElementBase(guid, szName, sourceFileInfo)
	{}

	// IEnvElement

	virtual bool IsValidScope(IEnvElement& scope) const override
	{
		switch (scope.GetType())
		{
		case EEnvElementType::Root:
		case EEnvElementType::Module:
		case EEnvElementType::Component:
		case EEnvElementType::Action:
			{
				return true;
			}
		default:
			{
				return false;
			}
		}
	}

	// ~IEnvElement
};

class CEnvInterfaceFunction : public CEnvElementBase<IEnvInterfaceFunction>
{
private:

	struct SParam
	{
		SParam(u32 _id, tukk _szName, tukk _szDescription, const CAnyValuePtr& _pValue)
			: id(_id)
			, name(_szName)
			, description(_szDescription)
			, pValue(_pValue)
		{}

		u32       id;
		string       name;
		string       description;
		CAnyValuePtr pValue;
	};

	typedef std::vector<SParam> Params;

public:

	inline CEnvInterfaceFunction(const DrxGUID& guid, tukk szName, const SSourceFileInfo& sourceFileInfo)
		: CEnvElementBase(guid, szName, sourceFileInfo)
	{}

	// IEnvElement

	virtual bool IsValidScope(IEnvElement& scope) const override
	{
		switch (scope.GetType())
		{
		case EEnvElementType::Interface:
			{
				return true;
			}
		default:
			{
				return false;
			}
		}
	}

	// ~IEnvElement

	// IEnvInterfaceFunction

	virtual u32 GetInputCount() const override
	{
		return m_inputs.size();
	}

	virtual tukk GetInputName(u32 inputIdx) const override
	{
		return inputIdx < m_inputs.size() ? m_inputs[inputIdx].name.c_str() : "";
	}

	virtual tukk GetInputDescription(u32 inputIdx) const override
	{
		return inputIdx < m_inputs.size() ? m_inputs[inputIdx].description.c_str() : "";
	}

	virtual CAnyConstPtr GetInputValue(u32 inputIdx) const override
	{
		return inputIdx < m_inputs.size() ? m_inputs[inputIdx].pValue.get() : nullptr;
	}

	virtual u32 GetOutputCount() const override
	{
		return m_outputs.size();
	}

	virtual tukk GetOutputName(u32 outputIdx) const override
	{
		return outputIdx < m_outputs.size() ? m_outputs[outputIdx].name.c_str() : "";
	}

	virtual tukk GetOutputDescription(u32 outputIdx) const override
	{
		return outputIdx < m_outputs.size() ? m_outputs[outputIdx].description.c_str() : "";
	}

	virtual CAnyConstPtr GetOutputValue(u32 outputIdx) const override
	{
		return outputIdx < m_outputs.size() ? m_outputs[outputIdx].pValue.get() : nullptr;
	}

	// ~IEnvInterfaceFunction

	inline void AddInput(u32 id, tukk szName, tukk szDescription, const CAnyConstRef& value)
	{
		// #SchematycTODO : Validate id and name!
		m_inputs.push_back(SParam(id, szName, szDescription, CAnyValue::CloneShared(value)));
	}

	inline void AddOutput(u32 id, tukk szName, tukk szDescription, const CAnyConstRef& value)
	{
		// #SchematycTODO : Validate id and name!
		m_outputs.push_back(SParam(id, szName, szDescription, CAnyValue::CloneShared(value)));
	}

private:

	Params m_inputs;
	Params m_outputs;
};

namespace EnvInterface
{

inline std::shared_ptr<CEnvInterface> MakeShared(const DrxGUID& guid, tukk szName, const SSourceFileInfo& sourceFileInfo)
{
	return std::make_shared<CEnvInterface>(guid, szName, sourceFileInfo);
}

} // EnvInterface

namespace EnvInterfaceFunction
{

inline std::shared_ptr<CEnvInterfaceFunction> MakeShared(const DrxGUID& guid, tukk szName, const SSourceFileInfo& sourceFileInfo)
{
	return std::make_shared<CEnvInterfaceFunction>(guid, szName, sourceFileInfo);
}

} // EnvInterfaceFunction
} // sxema
