// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/IAbstractInterface.h>

namespace sxema2
{
	// Abstract interface function.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CAbstractInterfaceFunction : public IAbstractInterfaceFunction
	{
	public:

		CAbstractInterfaceFunction(const SGUID& guid, const SGUID& ownerGUID);

		// IAbstractInterfaceFunction
		virtual SGUID GetGUID() const override;
		virtual SGUID GetOwnerGUID() const override;
		virtual void SetName(tukk szName) override;
		virtual tukk GetName() const override;
		virtual void SetFileName(tukk szFileName, tukk szProjectDir) override;
		virtual tukk GetFileName() const override;
		virtual void SetAuthor(tukk szAuthor) override;
		virtual tukk GetAuthor() const override;
		virtual void SetDescription(tukk szDescription) override;
		virtual tukk GetDescription() const override;
		virtual void AddInput(tukk name, tukk szDescription, const IAnyPtr& pValue) override;
		virtual size_t GetInputCount() const override;
		virtual tukk GetInputName(size_t iInput) const override;
		virtual tukk GetInputDescription(size_t iInput) const override;
		virtual IAnyConstPtr GetInputValue(size_t iInput) const override;
		virtual TVariantConstArray GetVariantInputs() const override;
		virtual void AddOutput(tukk name, tukk szDescription, const IAnyPtr& pValue) override;
		virtual size_t GetOutputCount() const override;
		virtual tukk GetOutputName(size_t iOutput) const override;
		virtual tukk GetOutputDescription(size_t iOutput) const override;
		virtual IAnyConstPtr GetOutputValue(size_t iOutput) const override;
		virtual TVariantConstArray GetVariantOutputs() const override;
		// ~IAbstractInterfaceFunction

	private:

		struct SParam
		{
			SParam(tukk _name, tukk _szDescription, const IAnyPtr& _pValue);

			string	name;
			string	description;
			IAnyPtr	pValue;
		};

		typedef std::vector<SParam> TParamVector;

		SGUID						m_guid;
		SGUID						m_ownerGUID;
		string					m_name;
		string					m_fileName;
		string					m_author;
		string					m_description;
		TParamVector		m_inputs;
		TVariantVector	m_variantInputs;
		TParamVector		m_outputs;
		TVariantVector	m_variantOutputs;
	};

	DECLARE_SHARED_POINTERS(CAbstractInterfaceFunction)

	// Abstract interface.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CAbstractInterface : public IAbstractInterface
	{
	public:

		CAbstractInterface(const SGUID& guid);

		// IAbstractInterface
		virtual SGUID GetGUID() const override;
		virtual void SetName(tukk name) override;
		virtual tukk GetName() const override;
		virtual void SetNamespace(tukk szNamespace) override;
		virtual tukk GetNamespace() const override;
		virtual void SetFileName(tukk fileName, tukk projectDir) override;
		virtual tukk GetFileName() const override;
		virtual void SetAuthor(tukk author) override;
		virtual tukk GetAuthor() const override;
		virtual void SetDescription(tukk szDescription) override;
		virtual tukk GetDescription() const override;
		virtual IAbstractInterfaceFunctionPtr AddFunction(const SGUID& guid) override;
		virtual size_t GetFunctionCount() const override;
		virtual IAbstractInterfaceFunctionConstPtr GetFunction(size_t iFunction) const override;
		// ~IAbstractInterface

	private:

		typedef std::vector<CAbstractInterfaceFunctionPtr> TFunctionVector;

		SGUID						m_guid;
		string					m_name;
		string					m_namespace;
		string					m_fileName;
		string					m_author;
		string					m_description;
		TFunctionVector	m_functions;
	};
}
