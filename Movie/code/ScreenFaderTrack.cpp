// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Movie/StdAfx.h>

#include <drx3D/Movie/ScreenFaderTrack.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>

//-----------------------------------------------------------------------------
CScreenFaderTrack::CScreenFaderTrack()
{
	m_lastTextureID = -1;
	SetScreenFaderTrackDefaults();
}

//-----------------------------------------------------------------------------
CScreenFaderTrack::~CScreenFaderTrack()
{
	ReleasePreloadedTextures();
}

//-----------------------------------------------------------------------------
void CScreenFaderTrack::SerializeKey(SScreenFaderKey& key, XmlNodeRef& keyNode, bool bLoading)
{
	if (bLoading)
	{
		keyNode->getAttr("fadeTime", key.m_fadeTime);
		Vec3 color(0, 0, 0);
		keyNode->getAttr("fadeColor", color);
		key.m_fadeColor = Vec4(color, 1.f);
		i32 fadeType = 0;
		keyNode->getAttr("fadeType", fadeType);
		key.m_fadeType = SScreenFaderKey::EFadeType(fadeType);
		i32 fadeChangeType(0);

		if (keyNode->getAttr("fadeChangeType", fadeChangeType))
		{
			key.m_fadeChangeType = SScreenFaderKey::EFadeChangeType(fadeChangeType);
		}
		else
		{
			key.m_fadeChangeType = SScreenFaderKey::eFCT_Linear;
		}

		tukk str;
		str = keyNode->getAttr("texture");
		drx_strcpy(key.m_texture, str, sizeof(key.m_texture));
		keyNode->getAttr("useCurColor", key.m_bUseCurColor);
	}
	else
	{
		keyNode->setAttr("fadeTime", key.m_fadeTime);
		Vec3 color(key.m_fadeColor.x, key.m_fadeColor.y, key.m_fadeColor.z);
		keyNode->setAttr("fadeColor", color);
		keyNode->setAttr("fadeType", (i32)key.m_fadeType);
		keyNode->setAttr("fadeChangeType", (i32)key.m_fadeChangeType);
		keyNode->setAttr("texture", key.m_texture);
		keyNode->setAttr("useCurColor", key.m_bUseCurColor);
	}
}

//-----------------------------------------------------------------------------
void CScreenFaderTrack::PreloadTextures()
{
	if (!m_preloadedTextures.empty())
	{
		ReleasePreloadedTextures();
	}

	i32k nKeysCount = GetNumKeys();

	if (nKeysCount > 0)
	{
		m_preloadedTextures.reserve(nKeysCount);

		for (i32 nKeyIndex = 0; nKeyIndex < nKeysCount; ++nKeyIndex)
		{
			SScreenFaderKey key;
			GetKey(nKeyIndex, &key);

			if (key.m_texture[0])
			{
				ITexture* pTexture = gEnv->pRenderer->EF_LoadTexture(key.m_texture, FT_DONT_STREAM | FT_STATE_CLAMP);

				if (pTexture)
				{
					pTexture->SetClamp(true);
					m_preloadedTextures.push_back(pTexture);
				}
			}
			else
			{
				m_preloadedTextures.push_back(NULL);
			}
		}
	}
}

//-----------------------------------------------------------------------------
ITexture* CScreenFaderTrack::GetActiveTexture() const
{
	i32 index = GetLastTextureID();
	return (index >= 0 && (size_t) index < m_preloadedTextures.size()) ? m_preloadedTextures[index] : 0;
}

void CScreenFaderTrack::SetScreenFaderTrackDefaults()
{
	m_bTextureVisible = false;
	m_drawColor = Vec4(1, 1, 1, 1);
}
//-----------------------------------------------------------------------------
void CScreenFaderTrack::ReleasePreloadedTextures()
{
	const size_t size = m_preloadedTextures.size();

	for (size_t i = 0; i < size; ++i)
	{
		SAFE_RELEASE(m_preloadedTextures[i]);
	}

	m_preloadedTextures.resize(0);
}

//-----------------------------------------------------------------------------
bool CScreenFaderTrack::SetActiveTexture(i32 index)
{
	ITexture* pTexture = GetActiveTexture();

	if (gEnv->IsEditor())
	{
		SetLastTextureID(index);

		// Check if textures should be reloaded.
		bool bNeedTexReload = pTexture == NULL; // Not yet loaded

		if (pTexture)
		{
			SScreenFaderKey key;
			GetKey(index, &key);

			if (strcmp(key.m_texture, pTexture->GetName()) != 0)
			{
				bNeedTexReload = true;  // Loaded, but a different texture
			}
		}

		if (bNeedTexReload)
		{
			// Ok, try to reload.
			PreloadTextures();
			pTexture = GetActiveTexture();
		}
	}

	return pTexture != 0;
}
