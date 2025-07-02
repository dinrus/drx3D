// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/ISignal.h>

namespace sxema2
{
	class CSignal : public ISignal
	{
	public:

		CSignal(const SGUID& guid, const SGUID& senderGUID = SGUID(), tukk szName = nullptr);

		// ISignal
		virtual SGUID GetGUID() const override;
		virtual void SetSenderGUID(const SGUID& senderGUID) override;
		virtual SGUID GetSenderGUID() const override;
		virtual void SetName(tukk szName) override;
		virtual tukk GetName() const override;
		virtual void SetNamespace(tukk szNamespace) override;
		virtual tukk GetNamespace() const override;
		virtual void SetFileName(tukk szFileName, tukk szProjectDir) override;
		virtual tukk GetFileName() const override;
		virtual void SetAuthor(tukk szAuthor) override;
		virtual tukk GetAuthor() const override;
		virtual void SetDescription(tukk szDescription) override;
		virtual tukk GetDescription() const override;
		virtual void SetWikiLink(tukk szWikiLink) override;
		virtual tukk GetWikiLink() const override;
		virtual size_t GetInputCount() const override;
		virtual tukk GetInputName(size_t inputIdx) const override;
		virtual tukk GetInputDescription(size_t inputIdx) const override;
		virtual TVariantConstArray GetVariantInputs() const override;
		virtual IAnyConstPtr GetInputValue(size_t inputIdx) const override;
		// ~ISignal

	protected:

		// ~ISignal
		virtual size_t AddInput_Protected(tukk szName, tukk szDescription, const IAny& value) override;
		// ~ISignal

	private:

		struct SInput
		{
			SInput(tukk _szName, tukk _szDescription, const IAnyPtr& _pValue);

			string  name;
			string  description;
			IAnyPtr pValue;
		};

		typedef std::vector<SInput> Inputs;

		SGUID          m_guid;
		SGUID          m_senderGUID;
		string         m_name;
		string         m_namespace;
		string         m_fileName;
		string         m_author;
		string         m_description;
		string         m_wikiLink;
		Inputs         m_inputs;
		TVariantVector m_variantInputs;
	};

	DECLARE_SHARED_POINTERS(CSignal)
}
