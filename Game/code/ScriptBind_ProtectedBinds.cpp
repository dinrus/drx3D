// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Script Binding for anything we only want to be accessed at controlled/protected points in the application

-------------------------------------------------------------------------
История:
- 17:01:2012   11:11 : Created by AndrewB
*************************************************************************/

//////////////////////////////////////////////////////////////////////////
// Pre-compiled header
#include <drx3D/Game/StdAfx.h>
// This include
#include <drx3D/Game/ScriptBind_ProtectedBinds.h>
#include <DrxFlowGraph/IFlowSystem.h>
#include <drx3D/Act/IPlayerProfiles.h>
#include <drx3D/Game/PersistantStats.h>
#include <drx3D/Game/DLCUpr.h>

//------------------------------------------------------------------------
// Constructor
CScriptBind_ProtectedBinds::CScriptBind_ProtectedBinds( ISystem *pSystem )
: m_pSystem( pSystem )
, m_active( false )
{
}

//------------------------------------------------------------------------
// Destructor
CScriptBind_ProtectedBinds::~CScriptBind_ProtectedBinds()
{
	Disable();
}

//------------------------------------------------------------------------
void CScriptBind_ProtectedBinds::Enable()
{
	if( ! m_active )
	{
		//setup
		Init(m_pSystem->GetIScriptSystem(), m_pSystem);

		RegisterMethods();
		RegisterGlobals();

		//allow global access
		SetGlobalName( "ProtectedBinds" );

		//activate
		m_active = true;
	}
}


//------------------------------------------------------------------------
void CScriptBind_ProtectedBinds::Disable()
{
	if( m_active )
	{
		//replace the global reference to the table with a NULL reference and release table
		Done();

		//deactivate
		m_active = false;
	}
}

//------------------------------------------------------------------------
void CScriptBind_ProtectedBinds::RegisterMethods()
{
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_ProtectedBinds::

	SCRIPT_REG_TEMPLFUNC( GetPersistantStat, "name" );
	SCRIPT_REG_TEMPLFUNC( SetPersistantStat, "name, valueTab ");
	SCRIPT_REG_TEMPLFUNC( SavePersistantStatsToBlaze, "");
	SCRIPT_REG_TEMPLFUNC( GetProfileAttribute, "name" );
	SCRIPT_REG_TEMPLFUNC( SetProfileAttribute, "name, valueTab" );
	SCRIPT_REG_TEMPLFUNC( ActivateDemoEventEntitlement, "");
	
#undef SCRIPT_REG_CLASSNAME
}

//------------------------------------------------------------------------
void CScriptBind_ProtectedBinds::RegisterGlobals()
{

}

//------------------------------------------------------------------------
i32 CScriptBind_ProtectedBinds::GetPersistantStat(IFunctionHandler *pH, tukk name)
{
	if( ! m_active )
	{
		//abort
		return pH->EndFunction();
	}

	if (CPersistantStats *pPersistantStats = CPersistantStats::GetInstance())
	{
		ScriptAnyValue scriptVal;
		bool found=false;

		EIntPersistantStats intStat = CPersistantStats::GetIntStatFromName(name);
		if (intStat != EIPS_Invalid)
		{
			i32 intStatValue = pPersistantStats->GetStat(intStat);
			DrxLog("CScriptBind_ProtectedBinds::GetPersistantStat() found stat=%s is an intStat=%d with value=%d", name, intStat, intStatValue);
			scriptVal = intStatValue;
			found=true;
		}
		else 
		{
			EFloatPersistantStats floatStat = CPersistantStats::GetFloatStatFromName(name);
			if (floatStat != EFPS_Invalid)
			{
				float floatStatValue = pPersistantStats->GetStat(floatStat);
				DrxLog("CScriptBind_ProtectedBinds::GetPersistantStat() found stat=%s is a floatStat=%d with value=%f", name, floatStat, floatStatValue);
				scriptVal = floatStatValue;
				found=true;
			}
		}

		if (found)
		{
			return pH->EndFunction( scriptVal );
		}
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_ProtectedBinds::SetPersistantStat(IFunctionHandler *pH, tukk name, SmartScriptTable valueTab)
{
	//@Todo: Steam

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_ProtectedBinds::SavePersistantStatsToBlaze(IFunctionHandler *pH)
{
	if( ! m_active )
	{
		//abort
		return pH->EndFunction();
	}


	//@Todo: Steam
	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_ProtectedBinds::GetProfileAttribute( IFunctionHandler *pH, tukk name )
{
	if( ! m_active )
	{
		//abort
		return pH->EndFunction();
	}

	if( IPlayerProfile* pProfile = GetCurrentUserProfile() )
	{
		TFlowInputData dataVal;
		if( pProfile->GetAttribute( name, dataVal ) )
		{
			ScriptAnyValue scriptVal;

			//find the type of the value to access it

			switch( dataVal.GetType() )
			{
				case eFDT_Int:			{
															i32* intVal = dataVal.GetPtr<i32>();
															scriptVal = *intVal;
															break;
														}
				case eFDT_Float:		{
															float* floatVal = dataVal.GetPtr<float>();
															scriptVal = *floatVal;
															break;
														}

				case eFDT_Vec3:			{
															Vec3* vecVal = dataVal.GetPtr<Vec3>();
															scriptVal = *vecVal;
															break;
														}
				case eFDT_String:		{
															string* stringVal = dataVal.GetPtr<string>();
															scriptVal = stringVal->c_str();
															break;
														}
				case eFDT_Bool:			{
															bool* boolVal = dataVal.GetPtr<bool>();
															scriptVal = *boolVal;
															break;
														}
	
			}	

			return pH->EndFunction( scriptVal );
		}		
	}

	return pH->EndFunction();
}		

//------------------------------------------------------------------------
i32 CScriptBind_ProtectedBinds::SetProfileAttribute( IFunctionHandler *pH, tukk name, SmartScriptTable valueTab )
{
	if( ! m_active )
	{
		//abort
		return pH->EndFunction(0);
	}
	
	if( IPlayerProfile* pProfile = GetCurrentUserProfile() )
	{
		TFlowInputData dataVal;

		ScriptAnyValue scriptVal;

		valueTab->GetValueAny( "value", scriptVal );

		switch( scriptVal.type )
		{
		case ANY_TBOOLEAN: 			{
															bool boolVal;
															scriptVal.CopyTo( boolVal );
															dataVal.Set( boolVal );
															break;
														}
		case ANY_TNUMBER:				{
															//numbers could be float or i32 in the profile
															//get the attribute and try to match the current type if it exists
															bool isInt = false;
															TFlowInputData originalData;
															if( pProfile->GetAttribute( name, originalData ) )
															{
																if( originalData.GetType() == eFDT_Int	)
																{
																	isInt = true;
																}
															}
															//if it doesn't exist or is currently a non-number type then we assume float

															if( isInt )
															{
																i32 intVal;
																scriptVal.CopyTo( intVal );
																dataVal.Set( intVal );
															}
															else
															{
																float floatVal;
																scriptVal.CopyTo( floatVal );
																dataVal.Set( floatVal );
															}
															
															break;
														}
		case ANY_TSTRING:				{
															tukk charVal;
															scriptVal.CopyTo( charVal );
															string stringVal( charVal );
															dataVal.Set( stringVal );
															break;
														}
		case ANY_TVECTOR:				{
															Vec3 vecVal;
															scriptVal.CopyTo( vecVal );
															dataVal.Set( vecVal );
															break;
														}
		}

		pProfile->SetAttribute( name, dataVal );
	}

	return pH->EndFunction(0);
}

//------------------------------------------------------------------------
i32 CScriptBind_ProtectedBinds::ActivateDemoEventEntitlement( IFunctionHandler *pH )
{
	if( ! m_active )
	{
		//abort
		return pH->EndFunction(0);
	}

	return pH->EndFunction(0);
}

//------------------------------------------------------------------------
IPlayerProfile* CScriptBind_ProtectedBinds::GetCurrentUserProfile()
{
	IPlayerProfile* pProfile = NULL;
	if( IPlayerProfileUpr *pProfileMan = gEnv->pGame->GetIGameFramework()->GetIPlayerProfileUpr() )
	{
		tukk user = pProfileMan->GetCurrentUser();
		pProfile = pProfileMan->GetCurrentProfile( user );
	}

	return pProfile;
}
