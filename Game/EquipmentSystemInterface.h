// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   EquipmentSystemInterface.h
//  Version:     v1.00
//  Created:     07/07/2006 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Interface for Editor to access DrxAction/Game specific equipments
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __EQUIPMENTSYSTEMINTERFACE_H__
#define __EQUIPMENTSYSTEMINTERFACE_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <DrxSandbox/IEditorGame.h>

// some forward decls
class CEditorGame;
struct IItemSystem;
struct IEquipmentUpr;

class CEquipmentSystemInterface : public IEquipmentSystemInterface
{
public:
	CEquipmentSystemInterface(CEditorGame* pEditorGame, IGameToEditorInterface* pGTE);
	~CEquipmentSystemInterface();

	// return iterator with all available equipment items
	virtual IEquipmentItemIteratorPtr CreateEquipmentItemIterator(tukk type="");
	virtual IEquipmentItemIteratorPtr CreateEquipmentAccessoryIterator(tukk type);

	// delete all equipment packs
	virtual void DeleteAllEquipmentPacks();

	// load a single equipment pack from an XmlNode
	// Equipment Pack is basically
	// <EquipPack name="BasicPack">
	//   <Items>
	//      <Scar type="Weapon" />
	//      <SOCOM type="Weapon" />
	//   </Items>
	//   <Ammo Scar="50" SOCOM="70" />
	// </EquipPack>

	virtual bool LoadEquipmentPack(const XmlNodeRef& rootNode);	

	// set the players equipment pack. maybe we enable this, but normally via FG only
	// virtual void SetPlayerEquipmentPackName(tukk packName);
protected:
	void InitItems(IGameToEditorInterface* pGTE);

protected:
	class CIterator;

	CEditorGame* m_pEditorGame;
	IItemSystem* m_pIItemSystem;
	IEquipmentUpr* m_pIEquipmentUpr;

	typedef std::vector<string> TNameArray;
	typedef std::map<string, TNameArray> TItemMap;
	typedef std::map<string, TItemMap> TAccessoryMap;
	// maybe make it a multimap, or contain real item desc instead of only name
	TItemMap m_itemMap;
	TAccessoryMap m_accessoryMap;
};

#endif
