// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание: This file contains a game parameters registry/repository class, where all kind of
             game data, that might be shared between instances of the same class, is stored.
						 Things like weapon firemode / zoom parameters, item actions, ammo parameters...

						 Also it provides an interface to be implemented by data structures that might be
						 shared between instances. The idea is to force an implementation that uses smart pointers
						 to handle this data, and to have a common way to retrieve such data.

-------------------------------------------------------------------------
История:
- 16:09:2008   Benito G.R.

*************************************************************************/
#ifndef __GAME_PARAMETERS_H__
#define __GAME_PARAMETERS_H__

#include <drx3D/Game/ItemParamsRegistration.h>

//Interface that should be implemented by any data structure that could
//be potentially shared between instances of the same class.
struct IGameSharedParameters
{
	IGameSharedParameters() : m_refs(0) {}
	virtual	~IGameSharedParameters(){}

	//To be compatible with smart pointers
	void AddRef() const { ++m_refs; };
	u32 GetRefCount() const { return m_refs; };
	void Release() const { 
		if (--m_refs == 0)
			delete this;
	};

	virtual void GetMemoryUsage(IDrxSizer* s) const= 0;
	virtual tukk GetDataType() const = 0;
	virtual void ReleaseLevelResources() = 0;

private:
	mutable u32	m_refs;
};

class CItemSharedParams;
class CWeaponSharedParams;
class CItemResourceCache;

ITEM_PARAM_STRUCT_TYPES(FORWARD_DECLARE_STRUCTS)

//All shared game data/parameters go here
//Data is stored in different maps per game object types (items, ammo,...)
//and the classified per class.
class CGameSharedParametersStorage
{
public:
	typedef std::map<string, _smart_ptr<IGameSharedParameters> > TSharedParamsMap;

	CGameSharedParametersStorage();
	~CGameSharedParametersStorage();

	//Item parameters
	CItemSharedParams* GetItemSharedParameters(tukk className, bool create);
	void ResetItemParameters(){ m_itemParametersMap.clear(); };

	CItemResourceCache& GetItemResourceCache() { return *m_pItemResourceCache; }
	void ReleaseLevelResources();

	//Weapon parameters
	i32 GetWeaponSharedParametersCount() const;
	tukk GetWeaponSharedParametersName(i32 index) const;
	CWeaponSharedParams *GetWeaponSharedParameters(tukk className, bool create);
	void ResetWeaponParameters() { m_weaponParametersMap.clear(); };

	void GetMemoryStatistics(IDrxSizer *s);

	void GetDetailedMemoryStatics(TSharedParamsMap* paramsMap, tukk typeName);
	void GetDetailedItemParamMemoryStatistics() { GetDetailedMemoryStatics(&m_itemParametersMap, "Item"); };
	void GetDetailedWeaponParamMemoryStatistics() { GetDetailedMemoryStatics(&m_weaponParametersMap, "Weapon"); };

	void ClearItemParamSets();

	ITEM_PARAM_STRUCT_TYPES(DECLARE_SET_FUNCS)

private:
	void ReleaseLevelResources(TSharedParamsMap& paramsMap);

	CItemResourceCache* m_pItemResourceCache;

	TSharedParamsMap m_itemParametersMap;
	TSharedParamsMap m_weaponParametersMap;

	ITEM_PARAM_STRUCT_TYPES(INSTANCE_SETS)
};

#endif