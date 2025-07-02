// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/AbstractInterface.h>

#include <drx3D/Schema2/StringUtils.h>

namespace sxema2
{
	//////////////////////////////////////////////////////////////////////////
	CAbstractInterfaceFunction::CAbstractInterfaceFunction(const SGUID& guid, const SGUID& ownerGUID)
		: m_guid(guid)
		, m_ownerGUID(ownerGUID)
	{}

	//////////////////////////////////////////////////////////////////////////
	SGUID CAbstractInterfaceFunction::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CAbstractInterfaceFunction::GetOwnerGUID() const
	{
		return m_ownerGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	void CAbstractInterfaceFunction::SetName(tukk name)
	{
		m_name = name;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CAbstractInterfaceFunction::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CAbstractInterfaceFunction::SetFileName(tukk fileName, tukk projectDir)
	{
		StringUtils::MakeProjectRelativeFileName(fileName, projectDir, m_fileName);
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CAbstractInterfaceFunction::GetFileName() const
	{
		return m_fileName.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CAbstractInterfaceFunction::SetAuthor(tukk author)
	{
		m_author = author;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CAbstractInterfaceFunction::GetAuthor() const
	{
		return m_author.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CAbstractInterfaceFunction::SetDescription(tukk szDescription)
	{
		m_description = szDescription;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CAbstractInterfaceFunction::GetDescription() const
	{
		return m_description.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CAbstractInterfaceFunction::AddInput(tukk name, tukk textDesc, const IAnyPtr& pValue)
	{
		DRX_ASSERT(pValue);
		if(pValue)
		{
			m_inputs.push_back(SParam(name, textDesc, pValue));

			CVariantVectorOutputArchive	archive(m_variantInputs);
			archive(*pValue, "", "");
		}
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CAbstractInterfaceFunction::GetInputCount() const
	{
		return m_inputs.size();
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CAbstractInterfaceFunction::GetInputName(size_t iInput) const
	{
		return iInput < m_inputs.size() ? m_inputs[iInput].name.c_str() : "";
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CAbstractInterfaceFunction::GetInputDescription(size_t iInput) const
	{
		return iInput < m_inputs.size() ? m_inputs[iInput].description.c_str() : "";
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyConstPtr CAbstractInterfaceFunction::GetInputValue(size_t iInput) const
	{
		return iInput < m_inputs.size() ? m_inputs[iInput].pValue : IAnyConstPtr();
	}

	//////////////////////////////////////////////////////////////////////////
	TVariantConstArray CAbstractInterfaceFunction::GetVariantInputs() const
	{
		return m_variantInputs;
	}

	//////////////////////////////////////////////////////////////////////////
	void CAbstractInterfaceFunction::AddOutput(tukk name, tukk textDesc, const IAnyPtr& pValue)
	{
		DRX_ASSERT(pValue);
		if(pValue)
		{
			m_outputs.push_back(SParam(name, textDesc, pValue));

			CVariantVectorOutputArchive	archive(m_variantOutputs);
			archive(*pValue, "", "");
		}
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CAbstractInterfaceFunction::GetOutputCount() const
	{
		return m_outputs.size();
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CAbstractInterfaceFunction::GetOutputName(size_t iOutput) const
	{
		return iOutput < m_outputs.size() ? m_outputs[iOutput].name.c_str() : "";
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CAbstractInterfaceFunction::GetOutputDescription(size_t iOutput) const
	{
		return iOutput < m_outputs.size() ? m_outputs[iOutput].description.c_str() : "";
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyConstPtr CAbstractInterfaceFunction::GetOutputValue(size_t iOutput) const
	{
		return iOutput < m_outputs.size() ? m_outputs[iOutput].pValue : IAnyConstPtr();
	}

	//////////////////////////////////////////////////////////////////////////
	TVariantConstArray CAbstractInterfaceFunction::GetVariantOutputs() const
	{
		return m_variantOutputs;
	}

	//////////////////////////////////////////////////////////////////////////
	CAbstractInterfaceFunction::SParam::SParam(tukk _name, tukk _szDescription, const IAnyPtr& _pValue)
		: name(_name)
		, description(_szDescription)
		, pValue(_pValue)
	{}

	//////////////////////////////////////////////////////////////////////////
	CAbstractInterface::CAbstractInterface(const SGUID& guid)
		: m_guid(guid)
	{}

	//////////////////////////////////////////////////////////////////////////
	SGUID CAbstractInterface::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	void CAbstractInterface::SetName(tukk name)
	{
		m_name = name;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CAbstractInterface::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CAbstractInterface::SetNamespace(tukk szNamespace)
	{
		m_namespace = szNamespace;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CAbstractInterface::GetNamespace() const
	{
		return m_namespace.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CAbstractInterface::SetFileName(tukk fileName, tukk projectDir)
	{
		StringUtils::MakeProjectRelativeFileName(fileName, projectDir, m_fileName);
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CAbstractInterface::GetFileName() const
	{
		return m_fileName.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CAbstractInterface::SetAuthor(tukk author)
	{
		m_author = author;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CAbstractInterface::GetAuthor() const
	{
		return m_author.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CAbstractInterface::SetDescription(tukk szDescription)
	{
		m_description = szDescription;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CAbstractInterface::GetDescription() const
	{
		return m_description.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	IAbstractInterfaceFunctionPtr CAbstractInterface::AddFunction(const SGUID& guid)
	{
		CAbstractInterfaceFunctionPtr	pFunction(new CAbstractInterfaceFunction(guid, m_guid));
		m_functions.push_back(pFunction);
		return pFunction;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CAbstractInterface::GetFunctionCount() const
	{
		return m_functions.size();
	}

	//////////////////////////////////////////////////////////////////////////
	IAbstractInterfaceFunctionConstPtr CAbstractInterface::GetFunction(size_t iFunction) const
	{
		return iFunction < m_functions.size() ? m_functions[iFunction] : IAbstractInterfaceFunctionConstPtr();
	}
}
