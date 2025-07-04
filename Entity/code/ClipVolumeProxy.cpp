// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Entity/stdafx.h>

#include <drx3D/Entity/ClipVolumeProxy.h>
#include <drx3D/Eng3D/CGFContent.h>

DRXREGISTER_CLASS(CEntityComponentClipVolume);

//////////////////////////////////////////////////////////////////////////
CEntityComponentClipVolume::CEntityComponentClipVolume()
	: m_pClipVolume(NULL)
	, m_pBspTree(NULL)
	, m_nFlags(IClipVolume::eClipVolumeAffectedBySun)
	, m_viewDistRatio(100)
{
	m_componentFlags.Add(EEntityComponentFlags::Legacy);
}

CEntityComponentClipVolume::~CEntityComponentClipVolume()
{
	if (m_pClipVolume)
	{
		gEnv->p3DEngine->DeleteClipVolume(m_pClipVolume);
		m_pClipVolume = NULL;
	}

	m_pRenderMesh = NULL;
	if (m_pBspTree)
	{
		g_pIEntitySystem->ReleaseBSPTree3D(m_pBspTree);
		m_pBspTree = nullptr;
	}

	m_GeometryFileName = "";
}

void CEntityComponentClipVolume::Initialize()
{
	m_pClipVolume = gEnv->p3DEngine->CreateClipVolume();
}

void CEntityComponentClipVolume::ProcessEvent(const SEntityEvent& event)
{
	if (event.event == ENTITY_EVENT_XFORM ||
	    event.event == ENTITY_EVENT_HIDE ||
	    event.event == ENTITY_EVENT_UNHIDE)
	{
		gEnv->p3DEngine->UpdateClipVolume(m_pClipVolume, m_pRenderMesh, m_pBspTree, m_pEntity->GetWorldTM(), m_viewDistRatio, !m_pEntity->IsHidden(), m_nFlags, m_pEntity->GetName());
	}
}

//////////////////////////////////////////////////////////////////////////
uint64 CEntityComponentClipVolume::GetEventMask() const
{
	return ENTITY_EVENT_BIT(ENTITY_EVENT_XFORM) | ENTITY_EVENT_BIT(ENTITY_EVENT_HIDE) | ENTITY_EVENT_BIT(ENTITY_EVENT_UNHIDE);
}

void CEntityComponentClipVolume::UpdateRenderMesh(IRenderMesh* pRenderMesh, const DynArray<Vec3>& meshFaces)
{
	m_pRenderMesh = pRenderMesh;
	g_pIEntitySystem->ReleaseBSPTree3D(m_pBspTree);

	const size_t nFaceCount = meshFaces.size() / 3;
	if (nFaceCount > 0)
	{
		IBSPTree3D::FaceList faceList;
		faceList.reserve(nFaceCount);

		for (i32 i = 0; i < meshFaces.size(); i += 3)
		{
			IBSPTree3D::CFace face;
			face.push_back(meshFaces[i + 0]);
			face.push_back(meshFaces[i + 1]);
			face.push_back(meshFaces[i + 2]);

			faceList.push_back(face);
		}

		m_pBspTree = g_pIEntitySystem->CreateBSPTree3D(faceList);
	}

	if (m_pEntity && m_pClipVolume)
	{
		gEnv->p3DEngine->UpdateClipVolume(m_pClipVolume, m_pRenderMesh, m_pBspTree, m_pEntity->GetWorldTM(), m_viewDistRatio, !m_pEntity->IsHidden(), m_nFlags, m_pEntity->GetName());
	}
}

void CEntityComponentClipVolume::SetProperties(bool bIgnoresOutdoorAO, u8 viewDistRatio)
{
	if (m_pEntity && m_pClipVolume)
	{
		m_nFlags &= ~IClipVolume::eClipVolumeIgnoreOutdoorAO;
		m_nFlags |= bIgnoresOutdoorAO ? IClipVolume::eClipVolumeIgnoreOutdoorAO : 0;
		m_viewDistRatio = viewDistRatio;

		gEnv->p3DEngine->UpdateClipVolume(m_pClipVolume, m_pRenderMesh, m_pBspTree, m_pEntity->GetWorldTM(), m_viewDistRatio, !m_pEntity->IsHidden(), m_nFlags, m_pEntity->GetName());
	}
}

void CEntityComponentClipVolume::LegacySerializeXML(XmlNodeRef& entityNode, XmlNodeRef& componentNode, bool bLoading)
{
	if (bLoading)
	{
		LOADING_TIME_PROFILE_SECTION;

		XmlNodeRef volumeNode = componentNode->findChild("ClipVolume");
		if (!volumeNode)
		{
			// legacy behavior, look for properties on the entity node level
			volumeNode = entityNode->findChild("ClipVolume");
		}

		if (volumeNode)
		{
			tukk szFileName = NULL;
			if (volumeNode->getAttr("GeometryFileName", &szFileName))
			{
				// replace %level% by level path
				char szFilePath[_MAX_PATH];
				i32k nAliasNameLen = sizeof("%level%") - 1;

				drx_strcpy(szFilePath, gEnv->p3DEngine->GetLevelFilePath(szFileName + nAliasNameLen));

				if (m_pEntity && LoadFromFile(szFilePath))
				{
					gEnv->p3DEngine->UpdateClipVolume(m_pClipVolume, m_pRenderMesh, m_pBspTree, m_pEntity->GetWorldTM(), m_viewDistRatio, !m_pEntity->IsHidden(), m_nFlags, m_pEntity->GetName());
				}
			}
		}
	}
	else
	{
		XmlNodeRef volumeNode = componentNode->newChild("ClipVolume");
		volumeNode->setAttr("GeometryFileName", m_GeometryFileName);
	}
}

bool CEntityComponentClipVolume::LoadFromFile(tukk szFilePath)
{
	assert(!m_pRenderMesh && !m_pBspTree);

	if (FILE* pCgfFile = gEnv->pDrxPak->FOpen(szFilePath, "rb"))
	{
		const size_t nFileSize = gEnv->pDrxPak->FGetSize(pCgfFile);
		u8* pBuffer = new u8[nFileSize];

		gEnv->pDrxPak->FReadRawAll(pBuffer, nFileSize, pCgfFile);
		gEnv->pDrxPak->FClose(pCgfFile);

		_smart_ptr<IChunkFile> pChunkFile = gEnv->p3DEngine->CreateChunkFile(true);
		if (pChunkFile->ReadFromMemory(pBuffer, nFileSize))
		{
			if (IChunkFile::ChunkDesc* pBspTreeDataChunk = pChunkFile->FindChunkByType(ChunkType_BspTreeData))
			{
				m_pBspTree = g_pIEntitySystem->CreateBSPTree3D(IBSPTree3D::FaceList());
				m_pBspTree->ReadFromBuffer(static_cast<u8*>(pBspTreeDataChunk->data));
			}
			else
			{
				DrxLog("ClipVolume '%s' runtime collision data not found. Please reexport the level.", m_pEntity->GetName());
			}
		}

		if (gEnv->pRenderer)
		{
			CContentCGF* pCgfContent = gEnv->p3DEngine->CreateChunkfileContent(szFilePath);
			if (gEnv->p3DEngine->LoadChunkFileContentFromMem(pCgfContent, pBuffer, nFileSize, 0, false))
			{
				for (i32 i = 0; i < pCgfContent->GetNodeCount(); ++i)
				{
					CNodeCGF* pNode = pCgfContent->GetNode(i);
					if (pNode->type == CNodeCGF::NODE_MESH && pNode->pMesh)
					{
						m_pRenderMesh = gEnv->pRenderer->CreateRenderMesh("ClipVolume", szFilePath, NULL, eRMT_Static);
						m_pRenderMesh->SetMesh(*pNode->pMesh, 0, FSM_CREATE_DEVICE_MESH, NULL, false);

						break;
					}
				}
			}
			gEnv->p3DEngine->ReleaseChunkfileContent(pCgfContent);
		}

		delete[] pBuffer;
	}

	return m_pRenderMesh && m_pBspTree;
}

void CEntityComponentClipVolume::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->Add(m_GeometryFileName);
	pSizer->AddObject(m_pRenderMesh);
	pSizer->AddObject(m_pBspTree);
}

void CEntityComponentClipVolume::SetGeometryFilename(tukk sFilename)
{
	m_GeometryFileName = sFilename;
}
