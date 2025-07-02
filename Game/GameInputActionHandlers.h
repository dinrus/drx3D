// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$

Описание:  Все обработчики вводимых действий игры.

-------------------------------------------------------------------------
История:
- 16.04.10   : Created by Benito G.R.
*************************************************************************/

#pragma once

#ifndef _GAMEINPUT_ACTIONHANDLERS_H_
#define _GAMEINPUT_ACTIONHANDLERS_H_

/*********** WARNING **************
/
/ При добавлении нового обработчика в этот список аккуратно используйте
/ предварительные объявления классов, которые могут создавать проблемы.
/ Предпочитейте #include.
/
/***********************************************/

#include <IActionMapUpr.h>
#include <drx3D/Game/Weapon.h>
#include <drx3D/Game/HeavyMountedWeapon.h>
#include <drx3D/Game/JAW.h>
#include <drx3D/Game/LTAG.h>
#include <drx3D/Game/Binocular.h>
#include <drx3D/Game/PickAndThrowWeapon.h>
#include <drx3D/Game/NoWeapon.h>

class CGameInputActionHandlers
{
public:
	typedef TActionHandler<CWeapon> TWeaponActionHandler;
	typedef TActionHandler<CPickAndThrowWeapon> TPickAndThrowWeaponActionHandler;
	typedef TActionHandler<CLTag> TLTagActionHandler;
	typedef TActionHandler<CJaw> TJawActionHandler;
	typedef TActionHandler<CHeavyMountedWeapon> THMGActionHandler;
	typedef TActionHandler<CBinocular> TBinocularActionHandler;
	typedef TActionHandler<CNoWeapon> TNoWeaponActionHandler;

	ILINE TWeaponActionHandler& GetCWeaponActionHandler() { return m_weaponActionHandler; }
	ILINE TPickAndThrowWeaponActionHandler& GetCPickAndThrowWeaponActionHandler() { return m_pickAndThrowWeaponActionHandler; }
	ILINE TLTagActionHandler& GetCLtagActionHandler() { return m_ltagActionHandler; }
	ILINE TJawActionHandler& GetCJawActionHandler() { return m_jawActionHandler; }
	ILINE THMGActionHandler& GetCHMGActionHandler() { return m_hmgActionHandler; }
	ILINE TBinocularActionHandler& GetCBinocularActionHandler() { return m_binocularActionHandler; }
	ILINE TNoWeaponActionHandler& GetCNoWeaponActionHandler() { return m_NoWeaponActionHandler; }

private:

	TWeaponActionHandler	m_weaponActionHandler;
	TPickAndThrowWeaponActionHandler m_pickAndThrowWeaponActionHandler;
	TLTagActionHandler m_ltagActionHandler;
	TJawActionHandler m_jawActionHandler;
	THMGActionHandler m_hmgActionHandler;
	TBinocularActionHandler m_binocularActionHandler;
	TNoWeaponActionHandler m_NoWeaponActionHandler;

};

#endif //_GAMEINPUT_ACTIONHANDLERS_H_
