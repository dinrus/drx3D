// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 08:12:2010		Created by Ben Parbury
*************************************************************************/

#ifndef __WEAPONALIAS_H__
#define __WEAPONALIAS_H__

struct IEntityClass;

class CWeaponAlias
{
public:
	CWeaponAlias();
	virtual ~CWeaponAlias();

	void AddAlias(tukk pParentName, tukk pChildName);
	void Reset();

	//Will update pClass if they have a parent
	void UpdateClass(IEntityClass** ppClass) const;

	//Return pParentClass (or NULL if they don't have one)
	const IEntityClass* GetParentClass(const IEntityClass* pClass) const;
	const IEntityClass* GetParentClass(tukk pClassName) const;

	bool IsAlias(tukk pAliasName) const;

protected:
	struct SWeaponAlias
	{
		SWeaponAlias(tukk pParentName, tukk pName);

		IEntityClass* m_pParentClass;
		IEntityClass* m_pClass;
	};

	typedef std::vector<SWeaponAlias> TWeaponAliasVec;
	TWeaponAliasVec m_aliases;
};

#endif // __WEAPONALIAS_H__
