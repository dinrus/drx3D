// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SCRIPT_BIND__TURRET__H__
#define __SCRIPT_BIND__TURRET__H__

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>


class CTurret;

class CScriptBind_Turret
	: public CScriptableBase
{
public:
	CScriptBind_Turret( ISystem* pSystem );
	virtual ~CScriptBind_Turret();

	void AttachTo( CTurret* pTurret );
	void DettachFrom( CTurret* pTurret );

	i32 Enable( IFunctionHandler* pH );
	i32 Disable( IFunctionHandler* pH );

	i32 OnPropertyChange( IFunctionHandler* pH );
	i32 OnHit( IFunctionHandler* pH, SmartScriptTable scriptHitInfo );
	i32 SetStateById( IFunctionHandler* pH, i32 stateId );

	i32 SetFactionToPlayerFaction( IFunctionHandler* pH );
};

#endif
