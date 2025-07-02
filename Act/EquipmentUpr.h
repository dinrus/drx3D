// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   EquipmentUpr.h
//  Version:     v1.00
//  Created:     07/07/2006 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: EquipmentUpr to handle item packs
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __EQUIPMENTMANAGER_H__
#define __EQUIPMENTMANAGER_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Act/IItemSystem.h>

class CItemSystem;

class CEquipmentUpr : public IEquipmentUpr
{
public:
	CEquipmentUpr(CItemSystem* pItemSystem);
	~CEquipmentUpr();

	void Reset();

	// Clear all equipment packs
	virtual void DeleteAllEquipmentPacks();

	// Loads equipment packs from rootNode
	virtual void LoadEquipmentPacks(const XmlNodeRef& rootNode);

	// Load all equipment packs from a certain path
	virtual void LoadEquipmentPacksFromPath(tukk path);

	// Load an equipment pack from an XML node
	virtual bool LoadEquipmentPack(const XmlNodeRef& rootNode, bool bOverrideExisting = true);

	// Give an equipment pack (resp. items/ammo) to an actor
	virtual bool GiveEquipmentPack(IActor* pActor, tukk packName, bool bAdd, bool selectPrimary = false);

	// Pre-cache all resources needed for the items included in the given pack
	virtual void PreCacheEquipmentPackResources(tukk packName, IEquipmentPackPreCacheCallback& preCacheCallback);

	// return iterator with all available equipment packs
	virtual IEquipmentUpr::IEquipmentPackIteratorPtr CreateEquipmentPackIterator();

	virtual void                                         RegisterListener(IListener* pListener);
	virtual void                                         UnregisterListener(IListener* pListener);

	// listener callbacks
	void OnBeginGiveEquipmentPack();
	void OnEndGiveEquipmentPack();

	void DumpPacks();

	struct SEquipmentPack
	{
		struct SEquipmentItem
		{
			typedef std::vector<IEntityClass*> TSetupVector;

			SEquipmentItem(tukk name, tukk type, tukk setup)
				: m_name(name), m_type(type)
			{
				ParseSetup(setup);
			}

			void ParseSetup(tukk setup)
			{
				if (setup && (setup[0] != '\0'))
				{
					IEntityClassRegistry* pClassRegistry = gEnv->pEntitySystem->GetClassRegistry();
					tukk cur = setup;

					if (cur[0] == '|')
					{
						cur++;
					}

					tukk nxt = strstr(cur, "|");
					char stringBuffer[128];
					while (nxt)
					{
						drx_strcpy(stringBuffer, cur, (size_t)(nxt - cur));

						if (stringBuffer[0] != '\0')
						{
							if (IEntityClass* pClass = pClassRegistry->FindClass(stringBuffer))
							{
								m_setup.push_back(pClass);
							}
						}

						cur = nxt + 1;
						nxt = strstr(nxt + 1, "|");
					}

					if (*cur != '\0')
					{
						if (IEntityClass* pClass = pClassRegistry->FindClass(cur))
						{
							m_setup.push_back(pClass);
						}
					}
				}
			}

			string       m_name;
			string       m_type;
			TSetupVector m_setup;

			void         GetMemoryUsage(IDrxSizer* pSizer) const
			{
				pSizer->AddObject(m_name);
				pSizer->AddObject(m_type);
				pSizer->AddObject(m_setup);
			}
		};

		void Init(tukk name)
		{
			m_name.assign(name);
			m_primaryItem.assign("");
			m_items.clear();
			m_ammoCount.clear();
		}

		void PrepareForItems(size_t count)
		{
			m_items.reserve(m_items.size() + count);
		}

		bool AddItem(tukk name, tukk type, tukk setup)
		{
			if (HasItem(name))
				return false;
			m_items.push_back(SEquipmentItem(name, type, setup));
			return true;
		}

		bool HasItem(tukk name) const
		{
			for (std::vector<SEquipmentItem>::const_iterator iter = m_items.begin(), iterEnd = m_items.end();
			     iter != iterEnd; ++iter)
			{
				if (iter->m_name == name)
					return true;
			}
			return false;
		}

		inline i32 NumItems() const
		{
			return m_items.size();
		}

		const SEquipmentItem* GetItemByIndex(i32 index) const
		{
			if (index < 0 || index >= (i32)m_items.size())
				return 0;
			return &m_items[index];
		}

		void GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(m_name);
			pSizer->AddObject(m_primaryItem);
			pSizer->AddObject(m_items);
			pSizer->AddObject(m_ammoCount);
		}
		string                      m_name;
		string                      m_primaryItem;
		std::vector<SEquipmentItem> m_items;
		std::map<string, i32>       m_ammoCount;
	};
	typedef std::vector<SEquipmentPack*> TEquipmentPackVec;

	SEquipmentPack* GetPack(tukk packName) const;
	void            DumpPack(const SEquipmentPack* pPack) const;

	void            GetMemoryUsage(IDrxSizer* s) const;

protected:

	typedef std::vector<IEquipmentUpr::IListener*> TListenerVec;

	friend class CScriptBind_ItemSystem;

	CItemSystem*      m_pItemSystem;
	TEquipmentPackVec m_equipmentPacks;
	TListenerVec      m_listeners;
};

#endif
