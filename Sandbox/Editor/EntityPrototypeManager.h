// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __entityprototypemanager_h__
#define __entityprototypemanager_h__
#pragma once

#include "BaseLibraryManager.h"
#include "EntityPrototype.h"

class CEntityPrototype;
class CEntityPrototypeLibrary;

/** Manages all entity prototypes and prototype libraries.
 */
class SANDBOX_API CEntityPrototypeManager : public CBaseLibraryManager
{
public:
	CEntityPrototypeManager();
	~CEntityPrototypeManager();

	void ClearAll();

	//! Find prototype by fully specified prototype name.
	//! Name is given in this form: Library.Group.ItemName (ex. AI.Cover.Merc)
	CEntityPrototype* FindPrototype(DrxGUID guid)
	{
		CEntityPrototype* pPrototype = (CEntityPrototype*)FindItem(guid);
		if (pPrototype == NULL)
		{
			pPrototype = FindPrototypeFromXMLFiles(guid);
		}
		return pPrototype;
	}

	//! Loads a new entity archetype from a xml node.
	CEntityPrototype* LoadPrototype(CEntityPrototypeLibrary* pLibrary, XmlNodeRef& node);

	void              ExportPrototypes(const string& path, const string& levelName, CPakFile& levelPakFile);

private:

	CEntityPrototype*         FindPrototypeFromXMLFiles(DrxGUID guid);

	virtual CBaseLibraryItem* MakeNewItem();
	virtual CBaseLibrary*     MakeNewLibrary();
	//! Root node where this library will be saved.
	virtual string           GetRootNodeName();
	//! Path to libraries in this manager.
	virtual string           GetLibsPath();

	string m_libsPath;
};

#endif // __entityprototypemanager_h__

