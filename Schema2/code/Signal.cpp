// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/Signal.h>

#include <drx3D/Schema2/StringUtils.h>

namespace sxema2
{
	//////////////////////////////////////////////////////////////////////////
	CSignal::CSignal(const SGUID& guid, const SGUID& senderGUID, tukk szName)
		: m_guid(guid)
		, m_senderGUID(senderGUID)
		, m_name(szName)
	{}

	//////////////////////////////////////////////////////////////////////////
	SGUID CSignal::GetGUID() const
	{
		return m_guid;
	}

	//////////////////////////////////////////////////////////////////////////
	void CSignal::SetSenderGUID(const SGUID& senderGUID)
	{
		m_senderGUID = senderGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	SGUID CSignal::GetSenderGUID() const
	{
		return m_senderGUID;
	}

	//////////////////////////////////////////////////////////////////////////
	void CSignal::SetName(tukk szName)
	{
		m_name = szName;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CSignal::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CSignal::SetNamespace(tukk szNamespace)
	{
		m_namespace = szNamespace;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CSignal::GetNamespace() const
	{
		return m_namespace.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CSignal::SetFileName(tukk szFileName, tukk szProjectDir)
	{
		StringUtils::MakeProjectRelativeFileName(szFileName, szProjectDir, m_fileName);
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CSignal::GetFileName() const
	{
		return m_fileName.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CSignal::SetAuthor(tukk szAuthor)
	{
		m_author = szAuthor;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CSignal::GetAuthor() const
	{
		return m_author.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	void CSignal::SetDescription(tukk szDescription)
	{
		m_description = szDescription;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CSignal::GetDescription() const
	{
		return m_description.c_str();
	}

	void CSignal::SetWikiLink(tukk szWikiLink)
	{
		m_wikiLink = szWikiLink;
	}

	 tukk CSignal::GetWikiLink() const
	 {
		return m_wikiLink.c_str();
	 }


	//////////////////////////////////////////////////////////////////////////
	size_t CSignal::GetInputCount() const
	{
		return m_inputs.size();
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CSignal::GetInputName(size_t inputIdx) const
	{
		return inputIdx < m_inputs.size() ? m_inputs[inputIdx].name.c_str() : "";
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CSignal::GetInputDescription(size_t inputIdx) const
	{
		return inputIdx < m_inputs.size() ? m_inputs[inputIdx].description.c_str() : "";
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyConstPtr CSignal::GetInputValue(size_t inputIdx) const
	{
		return inputIdx < m_inputs.size() ? m_inputs[inputIdx].pValue : IAnyConstPtr();
	}

	//////////////////////////////////////////////////////////////////////////
	TVariantConstArray CSignal::GetVariantInputs() const
	{
		return m_variantInputs;
	}

	//////////////////////////////////////////////////////////////////////////
	size_t CSignal::AddInput_Protected(tukk szName, tukk szDescription, const IAny& value)
	{
		m_inputs.push_back(SInput(szName, szDescription, value.Clone()));
		CVariantVectorOutputArchive	archive(m_variantInputs);
		archive(const_cast<IAny&>(value), "", "");
		return m_inputs.size() - 1;
	}

	//////////////////////////////////////////////////////////////////////////
	CSignal::SInput::SInput(tukk _szName, tukk _szDescription, const IAnyPtr& _pValue)
		: name(_szName)
		, description(_szDescription)
		, pValue(_pValue)
	{}
}
