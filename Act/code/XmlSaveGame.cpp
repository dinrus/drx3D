// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/XmlSaveGame.h>
#include <drx3D/Act/XmlSerializeHelper.h>

#include <drx3D/CoreX/Platform/IPlatformOS.h>

static i32k XML_SAVEGAME_MAX_CHUNK_SIZE = 32767 / 2;

struct CXmlSaveGame::Impl
{
	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->Add(*this);
		pSizer->AddObject(sections);
		pSizer->AddObject(root);
		pSizer->AddObject(metadata);
		pSizer->AddObject(outputFile);
	}

	XmlNodeRef root;
	XmlNodeRef metadata;
	string     outputFile;

	typedef std::vector<_smart_ptr<CXmlSerializeHelper>> TContexts;
	TContexts sections;
};

CXmlSaveGame::CXmlSaveGame() : m_pImpl(new Impl), m_eReason(eSGR_QuickSave)
{
}

CXmlSaveGame::~CXmlSaveGame()
{
}

bool CXmlSaveGame::Init(tukk name)
{
	m_pImpl->outputFile = name;
	m_pImpl->root = GetISystem()->CreateXmlNode("SaveGame", true);
	m_pImpl->metadata = m_pImpl->root->createNode("Metadata");
	m_pImpl->root->addChild(m_pImpl->metadata);
	return true;
}

XmlNodeRef CXmlSaveGame::GetMetadataXmlNode() const
{
	return m_pImpl->metadata;
}

void CXmlSaveGame::AddMetadata(tukk tag, tukk value)
{
	m_pImpl->metadata->setAttr(tag, value);
}

void CXmlSaveGame::AddMetadata(tukk tag, i32 value)
{
	m_pImpl->metadata->setAttr(tag, value);
}

TSerialize CXmlSaveGame::AddSection(tukk section)
{
	XmlNodeRef node = m_pImpl->root->createNode(section);
	m_pImpl->root->addChild(node);

	_smart_ptr<CXmlSerializeHelper> pSerializer = new CXmlSerializeHelper;
	m_pImpl->sections.push_back(pSerializer);

	return TSerialize(pSerializer->GetWriter(node));
}

u8* CXmlSaveGame::SetThumbnail(u8k* imageData, i32 width, i32 height, i32 depth)
{
	return 0;
}

bool CXmlSaveGame::SetThumbnailFromBMP(tukk filename)
{
	return false;
}

bool CXmlSaveGame::Complete(bool successfulSoFar)
{
	if (successfulSoFar)
	{
		successfulSoFar &= Write(m_pImpl->outputFile.c_str(), m_pImpl->root);
	}
	delete this;
	return successfulSoFar;
}

tukk CXmlSaveGame::GetFileName() const
{
	if (m_pImpl.get())
		return m_pImpl.get()->outputFile;
	return NULL;
}

void CXmlSaveGame::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(m_pImpl);
}

bool CXmlSaveGame::Write(tukk filename, XmlNodeRef data)
{
#if DRX_PLATFORM_WINDOWS
	DrxSetFileAttributes(filename, 0x00000080);    // FILE_ATTRIBUTE_NORMAL
#endif

	// Try opening file for creation first, will also create directory.
	FILE* pFile = NULL;

	if (!GetISystem()->GetPlatformOS()->UsePlatformSavingAPI())
	{
		pFile = gEnv->pDrxPak->FOpen(filename, "wb");
		if (!pFile)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Failed to create file %s", filename);
			return false;
		}
	}

	const bool bSavedToFile = data->saveToFile(filename, XML_SAVEGAME_MAX_CHUNK_SIZE, pFile);
	if (pFile)
	{
		gEnv->pDrxPak->FClose(pFile);
	}
	return bSavedToFile;
}
