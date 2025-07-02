// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Game/ProfileOptions.h>

#include <drx3D/Game/Option.h>
#include <drx3D/Game/Game.h>
#include <drx3D/Act/IPlayerProfiles.h>
#include <drx3D/Game/ScreenResolution.h>
#include <drx3D/Game/GameXmlParamReader.h>

//////////////////////////////////////////////////////////////////////////


CProfileOptions::CProfileOptions()
	: m_bDeferredSaveMP(false)
	, m_bInitialProfileLoaded(false)
	, m_bLoadingProfile(false)
{
}

CProfileOptions::~CProfileOptions()
{
	IPlayerProfileUpr* const profileUpr = g_pGame->GetIGameFramework()->GetIPlayerProfileUpr();
	if(profileUpr)
	{
		profileUpr->RemoveListener(this);
	}

	while(m_allOptions.size()>0)
	{
		COption* pOption = m_allOptions.back(); m_allOptions.pop_back();
		SAFE_DELETE(pOption);
	}
}

void CProfileOptions::Init()
{
	XmlNodeRef root = GetISystem()->LoadXmlFromFile("libs/config/profiles/default/attributes.xml");
	if(!root)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Failed loading attributes.xml");
		return;
	}

	CGameXmlParamReader reader(root);
	i32k childCount = reader.GetUnfilteredChildCount();
	m_allOptions.reserve(childCount);


	for (i32 i = 0; i < childCount; ++i)
	{
		XmlNodeRef node = reader.GetFilteredChildAt(i);
		if(node)
		{
			AddOption(node);
		}
	}

	IPlayerProfileUpr* const profileUpr = g_pGame->GetIGameFramework()->GetIPlayerProfileUpr();
	DRX_ASSERT_MESSAGE(profileUpr != NULL, "IPlayerProfileUpr doesn't exist - profile options will not be updated");
	if(profileUpr)
		profileUpr->AddListener(this, false);
}

void CProfileOptions::SaveToProfile(IPlayerProfile* pProfile, bool online, u32 reason)
{
	if(reason & ePR_Options)
	{
	  if(online && gEnv->bMultiplayer)
	  if (pProfile == NULL)
		  return;
  
	  //Want to save all the options (mostly for online attributes)
	  std::vector<COption*>::iterator it = m_allOptions.begin();
	  std::vector<COption*>::iterator end = m_allOptions.end();
	  for(; it!=end; ++it)
	  {
			COption					*pOption = *it;

			IPlayerProfile	*pPrevPlayerProfile = pOption->GetPlayerProfile();

			pOption->SetPlayerProfile(pProfile);
		  
			pOption->SetToProfile();

			pOption->SetPlayerProfile(pPrevPlayerProfile);
	  }
	}
}


void CProfileOptions::LoadFromProfile(IPlayerProfile* pProfile, bool online, u32 reason)
{
	if(reason & ePR_Options)
	{
		DRX_ASSERT(pProfile != NULL);
		if (pProfile == NULL)
			return;

		m_bLoadingProfile = true;
		std::vector<COption*> options_to_init = m_allOptions;

		// Update the stored player profile pointer to stay in sync
		std::vector<COption*>::iterator it = options_to_init.begin();
		std::vector<COption*>::iterator end = options_to_init.end();
		for(; it!=end; ++it)
		{
			(*it)->SetPlayerProfile(pProfile);	//Resets to default
			(*it)->InitializeFromProfile();
		}
		m_bLoadingProfile = false;

		if (!IsInitialProfileLoaded())
		{
			SetInitialProfileLoaded(true);
		}

		// Need to support overriding cvars from game.cfg
		// After profile switch, need game.cfg rewritten since don't want original game.cfg values from overwriting
		// if quit game and come back
		WriteGameCfg();
	}
}

bool CProfileOptions::IsCVar(tukk cVarName)
{
	if(!cVarName || !cVarName[0])
		return false;

	ICVar *pCVar = gEnv->pConsole->GetCVar(cVarName);
	return pCVar!=NULL;
}



bool CProfileOptions::IsOption(tukk option)
{
	return GetOption(option)!=NULL;
}



COption* CProfileOptions::GetOption(tukk option) const
{
	if(!option || !option[0])
		return NULL;

	bool returnValue = false;
	std::vector<COption*>::const_iterator it = m_allOptions.begin();
	std::vector<COption*>::const_iterator end = m_allOptions.end();
	for(; it!=end; ++it)
	{
		COption* pOption = (*it);
		if (pOption->GetName().compare(option)==0)
		{
			return pOption;
		}
	}
	return NULL;
}



COption* CProfileOptions::GetOptionByCVar(tukk cvar) const
{
	if(!cvar || !cvar[0])
		return NULL;

	bool returnValue = false;
	std::vector<COption*>::const_iterator it = m_allOptions.begin();
	std::vector<COption*>::const_iterator end = m_allOptions.end();
	for(; it!=end; ++it)
	{
		COption* pOption = (*it);
		if(pOption->GetType() == ePOT_CVar || pOption->GetType() == ePOT_SysSpec)
		{
			CCVarOption* pCVarOption = static_cast<CCVarOption*>(pOption);

			if (pCVarOption->GetCVar().compareNoCase(cvar)==0)
			{
				return pOption;
			}
		}
		else if(pOption->GetType() == ePOT_ScreenResolution)
		{
			if(!stricmp("r_width", cvar) || !stricmp("r_height", cvar))
			{
				return pOption;
			}
		}
	}
	return NULL;
}



void CProfileOptions::AddOption(const XmlNodeRef node)
{
	tukk platform = node->getAttr("platform");
	if(platform != NULL && platform[0])
	{
		bool result = (strstr(platform, "pc")==0);

		if (result)
			return;
	}

	// Ignore gameOnly attributes in the editor
	bool gameOnly = false;
	node->getAttr("gameOnly",gameOnly);
	if(gameOnly && gEnv->IsEditor())
		return;

	tukk name = node->getAttr("name");
	tukk val = node->getAttr("value");
	tukk cvar = node->getAttr("cvar");

	bool preview = false;
	node->getAttr("preview", preview);

	bool confirmation = false;
	node->getAttr("confirmation", confirmation);

	bool restart = false;
	node->getAttr("requiresRestart", restart);

	bool writeToConfig = false;
	node->getAttr("writeToConfig", writeToConfig);

	AddOption(name, val, cvar, preview, confirmation, restart, restart||writeToConfig); //options that require restart also need to be written to the config
}



void CProfileOptions::AddOption(tukk name, tukk value, tukk cvar /*= NULL*/, const bool preview /*= false*/, const bool confirmation /*= false*/, const bool restart /*= false*/, const bool writeToConfig /*= false*/)
{
	if(!name || !name[0])
		return;

	if(!value)
		return;

	//ScopedSwitchToGlobalHeap globalHeap;

	COption* pOption = NULL;
	DrxFixedStringT<64> tmpName(name);
	DrxFixedStringT<64> compareName("SysSpec");
	if(tmpName.find(compareName.c_str())==0)
	{
		if(compareName.length() == tmpName.length())
		{
			pOption = new CSysSpecAllOption(name, value, cvar);
		}
		else
		{
			pOption = new CSysSpecOption(name, value, cvar);
		}
	}
	else if(IsCVar(cvar))
	{
		pOption = new CCVarOption(name, value, cvar);
	}
	else if(!strcmp(name, "Resolution"))
	{
		pOption = new CScreenResolutionOption(name, value);
	}
	else
	{
		pOption = new COption(name, value);
	}

	pOption->SetPreview(preview);
	pOption->SetConfirmation(confirmation);
	pOption->SetRequiresRestart(restart);
	pOption->SetWriteToConfig(writeToConfig);

	if(pOption)
	{
		if(IPlayerProfileUpr* profileUpr = g_pGame->GetIGameFramework()->GetIPlayerProfileUpr())
		{
			if(IPlayerProfile *profile = profileUpr->GetCurrentProfile(profileUpr->GetCurrentUser()))
			{
				pOption->SetPlayerProfile(profile);
				pOption->InitializeFromProfile();
			}

			if(!m_bLoadingProfile)
			{
				m_allOptions.push_back(pOption);
			}
			else
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING,
						   "Adding \"%s\" option while loading values from profile, option might not be initialized properly. Consider adding to attributes.xml", name);
			}
		}
	}
}



string CProfileOptions::GetOptionValueOrCreate(tukk command, bool getDefault/*=false*/)
{
	DRX_ASSERT(command != NULL);
	if(!command || !command[0])
		return "";

	if(!IsOption(command))
	{
		AddOption(command, "");
	}

	return GetOptionValue(command, getDefault);
}



string CProfileOptions::GetOptionValue(tukk command, bool getDefault/*=false*/)
{
	if(!command || !IsOption(command))
	{
		return "";
	}

	std::vector<COption*>::const_iterator it = m_allOptions.begin();
	std::vector<COption*>::const_iterator end = m_allOptions.end();
	for(; it!=end; ++it)
	{
		if((*it)->GetName().compare(command)==0)
		{
			return getDefault ? (*it)->GetDefault() : (*it)->Get();
		}
	}
	return "";
}



i32	CProfileOptions::GetOptionValueAsInt(tukk command, bool getDefault/*=false*/)
{
	if (!command)
		return -1;

	return atoi(GetOptionValue(command, getDefault));
}



float	CProfileOptions::GetOptionValueAsFloat(tukk command, bool getDefault/*=false*/)
{
	if (!command)
		return -1.0f;

	return (float) atof(GetOptionValue(command, getDefault));
}



void CProfileOptions::SetOptionValue(tukk command, tukk param, bool toPendingOptions)
{
	if (!command || !command[0])
		return;

	if (!param)
		return;
		
  if(!IsOption(command))
  {
    AddOption(command, param);
  }

  if (toPendingOptions)
  {
    AddOrReplacePendingOption(command, param);
    return;
  }

	//ScopedSwitchToGlobalHeap globalHeap;

  std::vector<COption*>::const_iterator it = m_allOptions.begin();
  std::vector<COption*>::const_iterator end = m_allOptions.end();
  for(; it!=end; ++it)
  {
    if((*it)->GetName().compare(command)==0)
    {
      (*it)->Set(param);
    }
  }
}



void CProfileOptions::SetOptionValue(tukk command, i32 value, bool toPendingOptions)
{
	if (!command || !command[0])
		return;

	string strValue;
	strValue.Format("%d", value);
	SetOptionValue(command,strValue.c_str(), toPendingOptions);	
}



void CProfileOptions::SetOptionValue(tukk command, float value, bool toPendingOptions)
{
	if (!command || !command[0])
		return;

	string strValue;
	strValue.Format("%f", value);
	SetOptionValue(command,strValue.c_str(), toPendingOptions);	
}



void CProfileOptions::InitializeFromCVar()
{
	IPlayerProfileUpr* const profileUpr = g_pGame->GetIGameFramework()->GetIPlayerProfileUpr();
	if(!profileUpr)
	{
		return;
	}

	IPlayerProfile* const playerProfile = profileUpr->GetCurrentProfile(profileUpr->GetCurrentUser());
	if (!playerProfile)
	{
		return;
	}

	//ScopedSwitchToGlobalHeap globalHeap;

	std::vector<COption*>::const_iterator it = m_allOptions.begin();
	std::vector<COption*>::const_iterator end = m_allOptions.end();
	for(; it!=end; ++it)
	{
		(*it)->SetPlayerProfile(playerProfile);
		(*it)->InitializeFromCVar();
	}
}



void CProfileOptions::SaveProfile(u32 reason /* = ePR_Options*/)
{
	IPlayerProfileUpr* profileUpr = g_pGame->GetIGameFramework()->GetIPlayerProfileUpr();
	if(!profileUpr || profileUpr->IsLoadingProfile() || profileUpr->IsSavingProfile())
		return;

	IPlayerProfileUpr::EProfileOperationResult result;
	profileUpr->SaveProfile(profileUpr->GetCurrentUser(), result, reason);
}

string CProfileOptions::GetPendingOptionValue(tukk command)
{
	std::vector<SPendingOption>::const_iterator it = m_pendingOptions.begin();
	std::vector<SPendingOption>::const_iterator end = m_pendingOptions.end();

	for(; it!=end; ++it)
	{
		const SPendingOption& pendingOption = (*it);
		if(!it->command.compareNoCase(command))
		{
			return pendingOption.param.c_str();
		}
	}

	return "";
}

bool CProfileOptions::HasPendingOptionValues()const
{
	return !m_pendingOptions.empty();
}



bool CProfileOptions::HasPendingOptionValue(tukk optionName)const
{
	std::vector<SPendingOption>::const_iterator it = m_pendingOptions.begin();
	std::vector<SPendingOption>::const_iterator end = m_pendingOptions.end();

	for(; it!=end; ++it)
	{
		const SPendingOption& option = (*it);
		tukk command = it->command.c_str();
		if(!it->command.compareNoCase(optionName))
		{
			return true;
		}
	}
	return false;
}



bool CProfileOptions::HasPendingOptionValuesWithConfirmation() const
{
	std::vector<SPendingOption>::const_iterator it = m_pendingOptions.begin();
	std::vector<SPendingOption>::const_iterator end = m_pendingOptions.end();

	for(; it!=end; ++it)
	{
		const SPendingOption& option = (*it);
		if(option.confirmation)
		{
			return true;
		}
	}
	return false;
}



bool CProfileOptions::HasPendingOptionValuesWithRequiresRestart() const
{
	std::vector<SPendingOption>::const_iterator it = m_pendingOptions.begin();
	std::vector<SPendingOption>::const_iterator end = m_pendingOptions.end();

	for(; it!=end; ++it)
	{
		const SPendingOption& option = (*it);
		if(option.restart)
		{
			return true;
		}
	}
	return false;
}



void CProfileOptions::ApplyPendingOptionsValuesForConfirmation()
{
	std::vector<SPendingOption>::const_iterator it = m_pendingOptions.begin();
	std::vector<SPendingOption>::const_iterator end = m_pendingOptions.end();

	for(; it!=end; ++it)
	{
		const SPendingOption& option = (*it);
		SetOptionValue(option.command.c_str(), option.param.c_str());
	}
}



void CProfileOptions::FlushPendingOptionValues()
{
	if(m_pendingOptions.empty())
		return;

	WriteGameCfg();

	std::vector<SPendingOption>::const_iterator it = m_pendingOptions.begin();
	std::vector<SPendingOption>::const_iterator end = m_pendingOptions.end();

	bool bResetOverscan = false;
	bool bIsMultiGPUEnabled = false;
	if(gEnv->pRenderer)
		gEnv->pRenderer->EF_Query(EFQ_MultiGPUEnabled, bIsMultiGPUEnabled);
	if (bIsMultiGPUEnabled)
	{
		bool bRequiresMGPUCfgReload = false;

		for(; it!=end; ++it)
		{
			const SPendingOption& option = (*it);
			tukk szCommandStr = option.command.c_str();
			if ( (strcmp(szCommandStr, "Resolution") != 0) &&
				 (strcmp(szCommandStr, "Fullscreen") != 0) &&
				 (strcmp(szCommandStr, "VSync") != 0) &&
				 (strcmp(szCommandStr, "SysSpec") != 0)) // mgpu.cfg is automatically reloaded when cvar is changed
			{
				bRequiresMGPUCfgReload = true;
			}

			SetOptionValue(szCommandStr, option.param.c_str());
		}

		if (bRequiresMGPUCfgReload)
		{
			GetISystem()->LoadConfiguration("mgpu.cfg",0,eLoadConfigInit);
		}
	}
	else
	{
		for(; it!=end; ++it)
		{
			const SPendingOption& option = (*it);
			tukk szCommandStr = option.command.c_str();
			if (strcmp(szCommandStr, "Stereo") == 0)
			{
				bResetOverscan = true;
			}
			SetOptionValue(option.command.c_str(), option.param.c_str());
		}
	}

	stl::free_container(m_pendingOptions);

	if (bResetOverscan)
	{
		ResetOverscanBorders ();
	}
}



void CProfileOptions::ClearPendingOptionValues()
{
	if(m_pendingOptions.empty())
		return;

	std::vector<SPendingOption>::const_iterator it = m_pendingOptions.begin();
	std::vector<SPendingOption>::const_iterator end = m_pendingOptions.end();

	for(; it!=end; ++it)
	{
		const SPendingOption& option = (*it);
		if(option.preview)
		{
			SetOptionValue(option.command.c_str(), option.original.c_str());
		}
	}

	stl::free_container(m_pendingOptions);
}



void CProfileOptions::ClearPendingOptionValuesFromConfirmation()
{
	if(m_pendingOptions.empty())
		return;

	std::vector<SPendingOption>::const_iterator it = m_pendingOptions.begin();
	std::vector<SPendingOption>::const_iterator end = m_pendingOptions.end();

	for(; it!=end; ++it)
	{
		const SPendingOption& option = (*it);
		SetOptionValue(option.command.c_str(), option.original.c_str());
	}

	stl::free_container(m_pendingOptions);
}



void CProfileOptions::ResetToDefault()
{
	ClearPendingOptionValues(); // Just to be safe

	std::vector<COption*>::iterator it = m_allOptions.begin();
	std::vector<COption*>::const_iterator end = m_allOptions.end();

	for(; it!=end; ++it)
	{
		COption* pOption = (*it);
		if (!pOption->IsWriteToConfig())
		{
			pOption->ResetDefault();
		}
	}
}



void CProfileOptions::AddOrReplacePendingOption(tukk command, tukk param)
{
	std::vector<SPendingOption>::iterator it = m_pendingOptions.begin();
	std::vector<SPendingOption>::const_iterator end = m_pendingOptions.end();

	for(; it!=end; ++it)
	{
		SPendingOption& option = (*it);
		
		if(option.command.compare(command))
			continue;

		if(option.preview)
		{
			SetOptionValue(command, param);
		}

		if(option.original.compare(param))
		{
			option.param = param;
		}
		else
		{
			m_pendingOptions.erase(it);
		}
		return;
	}

	COption* pOption = GetOption(command);
	if(pOption != NULL)
	{
		if(pOption->IsPreview())
		{
			m_pendingOptions.push_back(SPendingOption(command, param, GetOptionValue(command), true, pOption->IsConfirmation(), pOption->IsRequiresRestart(), pOption->IsWriteToConfig()));
			SetOptionValue(command, param);
		}
		else
		{
			m_pendingOptions.push_back(SPendingOption(command, param, GetOptionValue(command), false, pOption->IsConfirmation(), pOption->IsRequiresRestart(), pOption->IsWriteToConfig()));
		}
	}
}



bool CProfileOptions::WriteGameCfg()
{
	CDebugAllowFileAccess ignoreInvalidFileAccess;

	FILE* pFile = fxopen("%USER%/game.cfg", "wb");
	if (pFile == 0)
		return false;

	fputs("-- [Game-Configuration]\r\n",pFile);
	fputs("-- Attention: This file is re-generated by the system! Editing is not recommended! \r\n\r\n",pFile);

	CCVarSink sink (this, pFile);
	gEnv->pConsole->DumpCVars(&sink);

	fclose(pFile);
	return true;
}

void CProfileOptions::ResetOverscanBorders()
{
	SetOptionValue("OverscanBorderX", 0.f, true);
	SetOptionValue("OverscanBorderY", 0.f, true);
}

void CProfileOptions::CCVarSink::OnElementFound(ICVar *pCVar)
{
	if (pCVar == 0)
		return;

	tukk name = pCVar->GetName();
	tukk val = pCVar->GetString();

	COption* pOption = m_pOptions->GetOptionByCVar(name);
	if(!pOption)
		return;

	if(!pOption->IsWriteToConfig())
		return;

	tukk optionName = pOption->GetName().c_str();
	tukk writeValue = pOption->Get().c_str();

	const std::vector<SPendingOption>& pendingOptions = m_pOptions->GetPendingOptions();

	std::vector<SPendingOption>::const_iterator it = pendingOptions.begin();
	std::vector<SPendingOption>::const_iterator end = pendingOptions.end();

	for(; it!=end; ++it)
	{
		const SPendingOption& option = (*it);

		if(!option.writeToConfig)
			continue;

		if(option.command.compareNoCase(optionName))
			continue;

		writeValue = option.param.c_str();
		break;
	}

	DrxFixedStringT<128> format;
	pOption->GetWriteToConfigString(format, pCVar, writeValue);
	fputs(format.c_str(), m_pFile);
}


//////////////////////////////////////////////////////////////////////////
