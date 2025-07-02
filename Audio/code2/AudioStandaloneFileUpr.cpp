// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Audio/StdAfx.h>
#include <drx3D/Audio/AudioStandaloneFileUpr.h>
#include <drx3D/Audio/ATLAudioObject.h>
#include <drx3D/Audio/AudioCVars.h>
#include <drx3D/Audio/IAudioImpl.h>
#include <drx3D/CoreX/String/HashedString.h>

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

namespace DrxAudio
{
//////////////////////////////////////////////////////////////////////////
CAudioStandaloneFileUpr::~CAudioStandaloneFileUpr()
{
	if (m_pIImpl != nullptr)
	{
		Release();
	}
}

//////////////////////////////////////////////////////////////////////////
void CAudioStandaloneFileUpr::Init(Impl::IImpl* const pIImpl)
{
	m_pIImpl = pIImpl;
}

//////////////////////////////////////////////////////////////////////////
void CAudioStandaloneFileUpr::Release()
{
	if (!m_constructedStandaloneFiles.empty())
	{
		for (auto const pStandaloneFile : m_constructedStandaloneFiles)
		{
			m_pIImpl->DestructStandaloneFile(pStandaloneFile->m_pImplData);
			delete pStandaloneFile;
		}
		m_constructedStandaloneFiles.clear();
	}

	m_pIImpl = nullptr;
}

//////////////////////////////////////////////////////////////////////////
CATLStandaloneFile* CAudioStandaloneFileUpr::ConstructStandaloneFile(char const* const szFile, bool const bLocalized, Impl::ITrigger const* const pITrigger)
{
	CATLStandaloneFile* pStandaloneFile = new CATLStandaloneFile();

	pStandaloneFile->m_pImplData = m_pIImpl->ConstructStandaloneFile(*pStandaloneFile, szFile, bLocalized, pITrigger);
	pStandaloneFile->m_hashedFilename = CHashedString(szFile);

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	pStandaloneFile->m_bLocalized = bLocalized;
	pStandaloneFile->m_pITrigger = pITrigger;
#endif // INCLUDE_AUDIO_PRODUCTION_CODE

	m_constructedStandaloneFiles.push_back(pStandaloneFile);
	return pStandaloneFile;
}

//////////////////////////////////////////////////////////////////////////
void CAudioStandaloneFileUpr::ReleaseStandaloneFile(CATLStandaloneFile* const pStandaloneFile)
{
	if (pStandaloneFile != nullptr)
	{
		m_constructedStandaloneFiles.remove(pStandaloneFile);
		m_pIImpl->DestructStandaloneFile(pStandaloneFile->m_pImplData);
		delete pStandaloneFile;
	}
}

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
//////////////////////////////////////////////////////////////////////////
void CAudioStandaloneFileUpr::DrawDebugInfo(IRenderAuxGeom& auxGeom, Vec3 const& listenerPosition, float posX, float posY) const
{
	static float const headerColor[4] = { 1.0f, 0.5f, 0.0f, 0.7f };
	static float const itemPlayingColor[4] = { 0.1f, 0.7f, 0.1f, 0.9f };
	static float const itemStoppingColor[4] = { 0.8f, 0.7f, 0.1f, 0.9f };
	static float const itemLoadingColor[4] = { 0.9f, 0.2f, 0.2f, 0.9f };
	static float const itemOtherColor[4] = { 0.8f, 0.8f, 0.8f, 0.9f };

	auxGeom.Draw2dLabel(posX, posY, 1.5f, headerColor, false, "Standalone Files [%" PRISIZE_T "]", m_constructedStandaloneFiles.size());
	posY += 16.0f;

	DrxFixedStringT<MaxControlNameLength> lowerCaseSearchString(g_cvars.m_pDebugFilter->GetString());
	lowerCaseSearchString.MakeLower();

	bool const bIsFilterNotSet = (lowerCaseSearchString.empty() || (lowerCaseSearchString == "0"));

	for (auto const pStandaloneFile : m_constructedStandaloneFiles)
	{
		Vec3 const& position = pStandaloneFile->m_pAudioObject->GetTransformation().GetPosition();
		float const distance = position.GetDistance(listenerPosition);

		if (g_cvars.m_debugDistance <= 0.0f || (g_cvars.m_debugDistance > 0.0f && distance < g_cvars.m_debugDistance))
		{
			char const* const szStandaloneFileName = pStandaloneFile->m_hashedFilename.GetText().c_str();
			DrxFixedStringT<MaxControlNameLength> lowerCaseStandaloneFileName(szStandaloneFileName);
			lowerCaseStandaloneFileName.MakeLower();
			char const* const szObjectName = pStandaloneFile->m_pAudioObject->m_name.c_str();
			DrxFixedStringT<MaxControlNameLength> lowerCaseObjectName(szObjectName);
			lowerCaseObjectName.MakeLower();

			bool const bDraw = bIsFilterNotSet ||
				(lowerCaseStandaloneFileName.find(lowerCaseSearchString) != DrxFixedStringT<MaxControlNameLength>::npos) ||
				(lowerCaseObjectName.find(lowerCaseSearchString) != DrxFixedStringT<MaxControlNameLength>::npos);

			if (bDraw)
			{
				float const* pColor = itemOtherColor;

				switch (pStandaloneFile->m_state)
				{
				case EAudioStandaloneFileState::Playing:
				{
					pColor = itemPlayingColor;

					break;
				}
				case EAudioStandaloneFileState::Loading:
				{
					pColor = itemLoadingColor;

					break;
				}
				case EAudioStandaloneFileState::Stopping:
				{
					pColor = itemStoppingColor;

					break;
				}
				}

				auxGeom.Draw2dLabel(posX, posY, 1.2f,
					pColor,
					false,
					"%s on %s",
					szStandaloneFileName,
					szObjectName);

				posY += 11.0f;
			}
		}
	}
}

#endif // INCLUDE_AUDIO_PRODUCTION_CODE
}      // namespace DrxAudio
