// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __materiallibrary_h__
#define __materiallibrary_h__
#pragma once

#include "BaseLibrary.h"

/** Library of prototypes.
 */
class SANDBOX_API CMaterialLibrary : public CBaseLibrary
{
public:
	CMaterialLibrary(CBaseLibraryManager* pManager) : CBaseLibrary(pManager) {};
	virtual bool Save();
	virtual bool Load(const string& filename);
	virtual void Serialize(XmlNodeRef& node, bool bLoading);

	//////////////////////////////////////////////////////////////////////////
	// CBaseLibrary override.
	//////////////////////////////////////////////////////////////////////////
	void           AddItem(IDataBaseItem* item, bool bRegister = true);
	i32            GetItemCount() const { return m_items.size(); }
	IDataBaseItem* GetItem(i32 index);
	void           RemoveItem(IDataBaseItem* item);
	void           RemoveAllItems();
	IDataBaseItem* FindItem(const string& name);
private:
	std::vector<CBaseLibraryItem*> m_items;
};

#endif // __materiallibrary_h__

