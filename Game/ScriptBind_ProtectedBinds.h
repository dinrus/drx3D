// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Script Binding for anything we only want to be accessed at controlled/protected points in the application

-------------------------------------------------------------------------
История:
- 17:01:2012   11:11 : Created by AndrewB
*************************************************************************/
#ifndef __SCRIPTBIND_PROTECTED_H__
#define __SCRIPTBIND_PROTECTED_H__

//Base class include
#include <drx3D/Script/ScriptHelpers.h>

//Important includes
//#include <IScriptSystem.h>

//Pre-declarations
struct ISystem;
struct IPlayerProfile;

//////////////////////////////////////////////////////////////////////////
class CScriptBind_ProtectedBinds :
	public CScriptableBase
{

public:
	CScriptBind_ProtectedBinds( ISystem *pSystem );
	virtual ~CScriptBind_ProtectedBinds();

	virtual void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}


	// Persistant Stats
	i32 GetPersistantStat(IFunctionHandler *pH, tukk name);
	i32 SetPersistantStat(IFunctionHandler *pH, tukk name, SmartScriptTable valueTab);
	i32 SavePersistantStatsToBlaze(IFunctionHandler *pH);

	//Profile Functions
	i32 GetProfileAttribute( IFunctionHandler *pH, tukk name );
	i32 SetProfileAttribute(  IFunctionHandler *pH, tukk name, SmartScriptTable valueTab );

	i32 ActivateDemoEventEntitlement( IFunctionHandler *pH );

	void	Enable();
	void	Disable();

protected:

private:
	void RegisterGlobals();
	void RegisterMethods();

	IPlayerProfile*		GetCurrentUserProfile();

	ISystem*					m_pSystem;
	bool							m_active;
};

#endif //__SCRIPTBIND_PROTECTED_H__
