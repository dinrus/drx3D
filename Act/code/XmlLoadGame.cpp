// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/XmlLoadGame.h>
#include <drx3D/Act/XmlSerializeHelper.h>

#include <drx3D/CoreX/Platform/IPlatformOS.h>

struct CXmlLoadGame::Impl
{
	XmlNodeRef root;
	XmlNodeRef metadata;
	string     inputFile;

	std::vector<_smart_ptr<CXmlSerializeHelper>> sections;
};

CXmlLoadGame::CXmlLoadGame() : m_pImpl(new Impl)
{
}

CXmlLoadGame::~CXmlLoadGame()
{
}

bool CXmlLoadGame::Init(tukk name)
{
	if (GetISystem()->GetPlatformOS()->UsePlatformSavingAPI())
	{
		IPlatformOS::ISaveReaderPtr pSaveReader = GetISystem()->GetPlatformOS()->SaveGetReader(name);
		if (!pSaveReader)
		{
			return false;
		}

		size_t nFileSize;

		if ((pSaveReader->GetNumBytes(nFileSize) == IPlatformOS::eFOC_Failure) || (nFileSize <= 0))
		{
			return false;
		}

		std::vector<char> xmlData;
		xmlData.resize(nFileSize);

		if (pSaveReader->ReadBytes(&xmlData[0], nFileSize) == IPlatformOS::eFOC_Failure)
		{
			return false;
		}

		m_pImpl->root = GetISystem()->LoadXmlFromBuffer(&xmlData[0], nFileSize);
	}
	else
	{
		m_pImpl->root = GetISystem()->LoadXmlFromFile(name);
	}

	if (!m_pImpl->root)
		return false;

	m_pImpl->inputFile = name;

	m_pImpl->metadata = m_pImpl->root->findChild("Metadata");
	if (!m_pImpl->metadata)
		return false;

	return true;
}

bool CXmlLoadGame::Init(const XmlNodeRef& root, tukk fileName)
{
	m_pImpl->root = root;
	if (!m_pImpl->root)
		return false;

	m_pImpl->inputFile = fileName;

	m_pImpl->metadata = m_pImpl->root->findChild("Metadata");
	if (!m_pImpl->metadata)
		return false;

	return true;
}

tukk CXmlLoadGame::GetMetadata(tukk tag)
{
	return m_pImpl->metadata->getAttr(tag);
}

bool CXmlLoadGame::GetMetadata(tukk tag, i32& value)
{
	return m_pImpl->metadata->getAttr(tag, value);
}

bool CXmlLoadGame::HaveMetadata(tukk tag)
{
	return m_pImpl->metadata->haveAttr(tag);
}

std::unique_ptr<TSerialize> CXmlLoadGame::GetSection(tukk section)
{
	XmlNodeRef node = m_pImpl->root->findChild(section);
	if (!node)
		return std::unique_ptr<TSerialize>();

	_smart_ptr<CXmlSerializeHelper> pSerializer = new CXmlSerializeHelper;
	m_pImpl->sections.push_back(pSerializer);
	return std::unique_ptr<TSerialize>(new TSerialize(pSerializer->GetReader(node)));
}

bool CXmlLoadGame::HaveSection(tukk section)
{
	return m_pImpl->root->findChild(section) != 0;
}

void CXmlLoadGame::Complete()
{
	delete this;
}

tukk CXmlLoadGame::GetFileName() const
{
	if (m_pImpl.get())
		return m_pImpl.get()->inputFile;
	return NULL;
}
