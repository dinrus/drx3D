// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Game/Option.h>

#include <drx3D/Game/Game.h>
#include <drx3D/Act/IPlayerProfiles.h>
#include <drx3D/Game/ScreenResolution.h>

#include <drx3D/Game/UI/WarningsUpr.h>

//////////////////////////////////////////////////////////////////////////


COption::COption()
	: m_currentProfile(0)
	, m_preview(false)
	, m_confirmation(false)
	, m_restart(false)
	, m_writeToConfig(false)
{
}



COption::COption(tukk name, tukk val)
	: m_currentProfile(0)
	, m_preview(false)
	, m_confirmation(false)
	, m_restart(false)
	, m_writeToConfig(false)
{
	m_currentName = name;
	m_currentValue = val;
	m_defaultValue = val;
}



COption::~COption()
{
}


void COption::SetPlayerProfile(IPlayerProfile* profile)
{
	m_currentProfile = profile;
}


void COption::InitializeFromProfile()
{
	if(m_currentProfile)
	{
		m_currentProfile->GetAttribute(m_currentName,m_currentValue);
	}
}



void COption::InitializeFromCVar()
{
}



void COption::Set(tukk param)
{
	m_currentValue = param;
	if(m_currentProfile)
	{
		m_currentProfile->SetAttribute(m_currentName,m_currentValue);
	}
}

void COption::SetToProfile()
{
	if(m_currentProfile)
	{
		m_currentProfile->SetAttribute(m_currentName,m_currentValue);
	}
}

void COption::ResetDefault()
{
	Set(m_defaultValue.c_str());
}


const string& COption::Get() const
{
	return m_currentValue;
}


const string& COption::GetDefault() const
{
	return m_defaultValue;
}


void COption::SetPreview(const bool prev)
{
	m_preview = prev;
}


void COption::SetConfirmation(const bool conf)
{
	m_confirmation = conf;
}


void COption::SetRequiresRestart(const bool rest)
{
	m_restart = rest;
}


void COption::SetWriteToConfig(const bool write)
{
	m_writeToConfig = write;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


CCVarOption::CCVarOption()
{
}



CCVarOption::CCVarOption(tukk name, tukk val, tukk cvar)
{
	m_currentName = name;
	m_currentValue = val;
	m_defaultValue = val;
	m_cVar = cvar;
}



CCVarOption::~CCVarOption()
{
}



void CCVarOption::InitializeFromProfile()
{
	if(m_currentProfile)
	{
		m_currentProfile->GetAttribute(m_currentName, m_currentValue);
	}

	ICVar* var = gEnv->pConsole->GetCVar(m_cVar);

	// Marcio: Let's not override *.cfg files if we're in Editor mode!
	if (var != NULL && ((var->GetFlags() & (VF_WASINCONFIG | VF_SYSSPEC_OVERWRITE)) == 0))
	{
		if (IsRequiresRestart() && IsWriteToConfig()) // Don't overwrite cvars which are read from config at startup
		{
			m_currentValue = var->GetString();	
		}
		else
		{
			var->Set(m_currentValue.c_str());
		}
	}
	else if(var)
	{
		m_currentValue = var->GetString();
	}
}



void CCVarOption::InitializeFromCVar()
{
	ICVar* var = gEnv->pConsole->GetCVar(m_cVar);
	if(var)
		m_currentValue = var->GetString();

	if(m_currentProfile)
	{
		m_currentProfile->SetAttribute(m_currentName,m_currentValue);
	}
}

#if DRX_PLATFORM_WINDOWS
void CCVarOption::SetToProfile()
{
	if(!IsRequiresRestart())
	{
		InitializeFromCVar();
	}

	COption::SetToProfile();
}
#endif

void CCVarOption::Set(tukk param)
{
	m_currentValue = param;
	if(m_currentProfile)
	{
		m_currentProfile->SetAttribute(m_currentName,m_currentValue);
	}

	if(!IsRequiresRestart())
	{
		ICVar* var = gEnv->pConsole->GetCVar(m_cVar);
		// Marcio: Let's not override *.cfg files if we're in Editor mode!

		if (var != NULL && (!gEnv->IsEditor() || ((var->GetFlags() & VF_WASINCONFIG) == 0)))
			var->Set(m_currentValue.c_str());
	}
}


void CCVarOption::GetWriteToConfigString(DrxFixedStringT<128> &outString, ICVar* pCVar, tukk param) const
{
	if(!param || !pCVar)
		return;

	if(pCVar->GetType()==CVAR_STRING)
	{
		outString.Format("%s = \"%s\"\r\n", m_cVar.c_str(), param);
	}
	else
	{
		outString.Format("%s = %s\r\n", m_cVar.c_str(), param);
	}
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


CScreenResolutionOption::CScreenResolutionOption()
{
}



CScreenResolutionOption::~CScreenResolutionOption()
{
}



CScreenResolutionOption::CScreenResolutionOption(tukk name, tukk val)
{
	m_currentName = name;
	m_currentValue = val;
	m_defaultValue = val;
}



void CScreenResolutionOption::InitializeFromProfile()
{
	if(m_currentProfile)
	{
		m_currentProfile->GetAttribute(m_currentName, m_currentValue);
	}

	ICVar* widthCVar = gEnv->pConsole->GetCVar("r_Width");
	ICVar* heightCVar = gEnv->pConsole->GetCVar("r_Height");
	if(!widthCVar || !heightCVar)
	{
		return;
	}

	const bool wasInConfig = ((widthCVar->GetFlags() & VF_WASINCONFIG) != 0) || ((heightCVar->GetFlags() & VF_WASINCONFIG) != 0);
	if(!wasInConfig)
	{
		i32 index = atoi(m_currentValue.c_str());

		if(index!=-1)
		{
			ScreenResolution::SScreenResolution resolution;
			if (ScreenResolution::GetScreenResolutionAtIndex(index, resolution))
			{
				widthCVar->Set(resolution.iWidth);
				heightCVar->Set(resolution.iHeight);
				return;
			}
		}
	}

	i32k width = widthCVar->GetIVal();
	i32k height = heightCVar->GetIVal();

	//either we haven't had a valid resolution index or width and/or hight cvars were in a config file
	//so try to find a fitting resolution index and write it to profile instead
	m_currentValue.Format("%d", ScreenResolution::GetNearestResolution(width, height));
}



void CScreenResolutionOption::InitializeFromCVar()
{
	i32 width = 1280;
	i32 height = 960;


	ICVar* widthCVar = gEnv->pConsole->GetCVar("r_Width");
	ICVar* heightCVar = gEnv->pConsole->GetCVar("r_Height");

	// Marcio: Let's not override *.cfg files if we're in Editor mode!
	if (widthCVar)
	{
		width = widthCVar->GetIVal();
	}
	if (heightCVar)
	{
		height = heightCVar->GetIVal();
	}

	m_currentValue.Format("%d", ScreenResolution::GetNearestResolution(width, height));
}



void CScreenResolutionOption::Set(tukk param)
{
	m_currentValue = param;

	i32 index = atoi(param);

	if(index!=-1)
	{
		ScreenResolution::SScreenResolution resolution;
		if (ScreenResolution::GetScreenResolutionAtIndex(index, resolution))
		{
			ICVar* widthCVar = gEnv->pConsole->GetCVar("r_Width");
			ICVar* heightCVar = gEnv->pConsole->GetCVar("r_Height");

			// Marcio: Let's not override *.cfg files if we're in Editor mode!
			if (widthCVar != NULL && (!gEnv->IsEditor() || ((widthCVar->GetFlags() & VF_WASINCONFIG) == 0)))
			{
				widthCVar->Set(resolution.iWidth);
			}

			if (heightCVar != NULL && (!gEnv->IsEditor() || ((heightCVar->GetFlags() & VF_WASINCONFIG) == 0)))
			{
				heightCVar->Set(resolution.iHeight);
			}
		}
	}
	if(m_currentProfile)
	{
		m_currentProfile->SetAttribute(m_currentName,m_currentValue);
	}
}


void CScreenResolutionOption::GetWriteToConfigString(DrxFixedStringT<128> &outString, ICVar* pCVar, tukk param) const
{
	if(!param)
		return;

	i32 index = atoi(param);

	if(index!=-1)
	{
		ScreenResolution::SScreenResolution resolution;
		if (ScreenResolution::GetScreenResolutionAtIndex(index, resolution))
		{
			i32k writeValue = (stricmp("r_width", pCVar->GetName())==0) ? resolution.iWidth : resolution.iHeight;
			outString.Format("%s = %d\r\n", pCVar->GetName(), writeValue);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


string CSysSpecOption::s_customEntry = "-1";

CSysSpecOption::CSysSpecOption()
: CCVarOption()
{
}



CSysSpecOption::~CSysSpecOption()
{
}



CSysSpecOption::CSysSpecOption(tukk name, tukk val, tukk cvarName)
: CCVarOption(name, val, cvarName)
{
}



bool CSysSpecOption::IsValid() const
{
	ICVar* pCVar = gEnv->pConsole->GetCVar(m_cVar.c_str());
	if(pCVar && pCVar->GetIVal() != pCVar->GetRealIVal())
	{
		return false;
	}
	return true;
}



const string& CSysSpecOption::Get() const
{
	return IsValid() ? m_currentValue : s_customEntry;
}



void CSysSpecOption::Set(tukk param)
{
	CCVarOption::Set(param);

	// Now that its changed, reset post effects to avoid problems such as screen still being blurry when switchin to ultra settings
	gEnv->p3DEngine->ResetPostEffects(true);
}



void CSysSpecOption::InitializeFromProfile()
{
#if !DRX_PLATFORM_DESKTOP
	return;
#endif
	if(m_currentProfile)
	{
		m_currentProfile->GetAttribute(m_currentName, m_currentValue);
	}

	ICVar* var = gEnv->pConsole->GetCVar(m_cVar);
	if(!var)
		return;

	bool wasInConfig = true;

	ICVar* pSysSpecCVar = gEnv->pConsole->GetCVar("sys_spec");
	if(pSysSpecCVar)
	{
		wasInConfig = (pSysSpecCVar->GetFlags() & VF_WASINCONFIG) != 0;
	}

	//revert the VF_WASINCONFIG check for sys_spec_*
	//if it wasn't in the config, then it is the first time we start the game, so use whatever autodetect delivered
	if(!wasInConfig)
	{
		m_currentValue = var->GetString();
	}
	else
	{
		if(gEnv->pSystem->IsDevMode() && pSysSpecCVar && (pSysSpecCVar->GetFlags() & VF_SYSSPEC_OVERWRITE))
		{
			m_currentValue.Format("%d", pSysSpecCVar->GetIVal());//reset to what was specified by cfg's
		}
		ScopedConsoleLoadConfigType consoleType(GetISystem()->GetIConsole(), eLoadConfigGame);
		var->Set(m_currentValue.c_str());
	}
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


CSysSpecAllOption::CSysSpecAllOption()
: CSysSpecOption()
{
}



CSysSpecAllOption::~CSysSpecAllOption()
{
}



CSysSpecAllOption::CSysSpecAllOption(tukk name, tukk val, tukk cvarName)
: CSysSpecOption(name, val, cvarName)
{
}



bool CSysSpecAllOption::IsValid() const
{
	ICVar* pCVar = gEnv->pConsole->GetCVar("sys_spec_Full");
	if(pCVar && pCVar->GetIVal() != pCVar->GetRealIVal())
	{
		return false;
	}
	return true;
}



void CSysSpecAllOption::InitializeFromProfile()
{
#if !DRX_PLATFORM_DESKTOP
	return;
#endif

	ICVar* pCVar = gEnv->pConsole->GetCVar(m_cVar.c_str());
	if(!pCVar)
		return;

	const bool wasInConfig = (pCVar->GetFlags() & VF_WASINCONFIG) != 0;
	const bool wasAutoDetect = pCVar->GetIVal() > 0;

	if(!wasInConfig)
	{
		if(wasAutoDetect)
		{
			m_currentValue = pCVar->GetString();
		}
		else
		{
			if(m_currentProfile)
			{
				m_currentProfile->GetAttribute(m_currentName, m_currentValue);
				pCVar->Set(m_currentValue.c_str());
			}
		}
	}
	else
	{
		bool uniform = true;

		if(m_currentProfile)
		{
			m_currentProfile->GetAttribute(m_currentName, m_currentValue);

			i32 gameEffects = 3;
			i32 objectDetail = 3;
			i32 particles = 3;
			i32 postProcessing = 3;
			i32 shading = 3;
			i32 shadows = 3;
			i32 water = 3;

			m_currentProfile->GetAttribute("SysSpecGameEffects", gameEffects);
			m_currentProfile->GetAttribute("SysSpecObjectDetail", objectDetail);
			m_currentProfile->GetAttribute("SysSpecParticles", particles);
			m_currentProfile->GetAttribute("SysSpecPostProcessing", postProcessing);
			m_currentProfile->GetAttribute("SysSpecShading", shading);
			m_currentProfile->GetAttribute("SysSpecShadows", shadows);
			m_currentProfile->GetAttribute("SysSpecWater", water);

			uniform =			gameEffects == objectDetail
				&&	gameEffects == particles
				&&	gameEffects == postProcessing
				&&	gameEffects == shading
				&&	gameEffects == shadows
				&&	gameEffects == water;
			if(uniform)
			{
				m_currentValue.Format("%d", gameEffects);
			}
		}

		if(uniform)
		{
			//do not overwrite setting from a system.cfg setting
			bool applyValue = true;
			if(gEnv->pSystem->IsDevMode())
			{
				if(m_cVar == "sys_spec" && (pCVar->GetFlags() & VF_SYSSPEC_OVERWRITE))
				{
					applyValue = false;
				}
			}
			if(applyValue)
				pCVar->Set(m_currentValue.c_str());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


CNotSavedOption::CNotSavedOption()
: CCVarOption()
{
}



CNotSavedOption::~CNotSavedOption()
{
}



CNotSavedOption::CNotSavedOption(tukk name, tukk val, tukk cvarName)
: CCVarOption(name, val, cvarName)
{
}



void CNotSavedOption::Set(tukk param)
{
	m_currentValue = param;

	if(!IsRequiresRestart())
	{
		ICVar* var = gEnv->pConsole->GetCVar(m_cVar);
		// Marcio: Let's not override *.cfg files if we're in Editor mode!

		if (var != NULL && (!gEnv->IsEditor() || ((var->GetFlags() & VF_WASINCONFIG) == 0)))
			var->Set(m_currentValue.c_str());
	}
}



void CNotSavedOption::InitializeFromCVar()
{
	ICVar* var = gEnv->pConsole->GetCVar(m_cVar);
	if(var)
		m_currentValue = var->GetString();
}


void CNotSavedOption::InitializeFromProfile()
{
	ICVar* var = gEnv->pConsole->GetCVar(m_cVar);
	if(var)
		m_currentValue = var->GetString();
}


void CNotSavedOption::SetToProfile()
{
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


CAutoResetOption::CAutoResetOption()
{
}



CAutoResetOption::CAutoResetOption(tukk name, tukk val)
{
	m_currentName = name;
	m_defaultValue = val;
	ICVar* pMode = gEnv->pConsole->GetCVar("r_StereoMode");
	if(pMode)
	{
		m_currentValue = val;
	}
}



CAutoResetOption::~CAutoResetOption()
{
}



void CAutoResetOption::InitializeFromProfile()
{
}

//////////////////////////////////////////////////////////////////////////
