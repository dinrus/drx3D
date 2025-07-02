// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Act/StdAfx.h>

#include <drx3D/Act/MannequinInterface.h>

#include <drx3D/Act/ActionController.h>
#include <drx3D/Act/AnimationDatabaseUpr.h>
#include <drx3D/Act/GameObject.h>
#include <drx3D/Act/MannequinDebug.h>
#include <drx3D/Act/ProceduralClipFactory.h>

CMannequinInterface::CMannequinInterface()
	: m_pAnimationDatabaseUpr(new CAnimationDatabaseUpr())
	, m_mannequinGameListeners(0)
	, m_pProceduralClipFactory(new CProceduralClipFactory())
	, m_bSilentPlaybackMode(false)
{
	RegisterCVars();

	mannequin::RegisterProceduralClipsForModule(*m_pProceduralClipFactory);
}

CMannequinInterface::~CMannequinInterface()
{
	delete m_pAnimationDatabaseUpr;
}

void CMannequinInterface::UnloadAll()
{
	CActionController::OnShutdown();
	GetAnimationDatabaseUpr().UnloadAll();
	GetMannequinUserParamsUpr().Clear();
}

void CMannequinInterface::ReloadAll()
{
	GetAnimationDatabaseUpr().ReloadAll();
	GetMannequinUserParamsUpr().ReloadAll(GetAnimationDatabaseUpr());
}

IAnimationDatabaseUpr& CMannequinInterface::GetAnimationDatabaseUpr()
{
	DRX_ASSERT(m_pAnimationDatabaseUpr != NULL);
	return *m_pAnimationDatabaseUpr;
}

IActionController* CMannequinInterface::CreateActionController(IEntity* pEntity, SAnimationContext& context)
{
	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Mannequin, 0, "ActionController (%s)", pEntity ? pEntity->GetName() ? pEntity->GetName() : "<unknown>" : "<no entity>");
	return new CActionController(pEntity, context);
}

IActionController* CMannequinInterface::FindActionController(const IEntity& entity)
{
	return CActionController::FindActionController(entity);
}

IMannequinEditorUpr* CMannequinInterface::GetMannequinEditorUpr()
{
	return m_pAnimationDatabaseUpr;
}

void CMannequinInterface::AddMannequinGameListener(IMannequinGameListener* pListener)
{
	m_mannequinGameListeners.push_back(pListener);
}

void CMannequinInterface::RemoveMannequinGameListener(IMannequinGameListener* pListener)
{
	m_mannequinGameListeners.erase(std::remove(m_mannequinGameListeners.begin(), m_mannequinGameListeners.end(), pListener), m_mannequinGameListeners.end());
}

u32 CMannequinInterface::GetNumMannequinGameListeners()
{
	return m_mannequinGameListeners.size();
}

IMannequinGameListener* CMannequinInterface::GetMannequinGameListener(u32 idx)
{
	DRX_ASSERT(idx < m_mannequinGameListeners.size());
	return m_mannequinGameListeners[idx];
}

CMannequinUserParamsUpr& CMannequinInterface::GetMannequinUserParamsUpr()
{
	return m_userParamsUpr;
}

IProceduralClipFactory& CMannequinInterface::GetProceduralClipFactory()
{
	return *m_pProceduralClipFactory;
}

void CMannequinInterface::SetSilentPlaybackMode(bool bSilentPlaybackMode)
{
	m_bSilentPlaybackMode = bSilentPlaybackMode;
}

bool CMannequinInterface::IsSilentPlaybackMode() const
{
	return m_bSilentPlaybackMode;
}

void CMannequinInterface::RegisterCVars()
{
	mannequin::debug::RegisterCommands();
	CAnimationDatabase::RegisterCVars();
#ifndef _RELEASE
	REGISTER_STRING("mn_sequence_path", "Animations/Mannequin/FragmentSequences/", VF_CHEAT, "Default path for DrxMannequin sequence files");
	REGISTER_STRING("mn_override_preview_file", "", VF_CHEAT, "Default DrxMannequin preview file to use. When set it overrides the corresponding sandbox setting.");
#endif
}
