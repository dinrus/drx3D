// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "EntityPrototypeLibrary.h"

#include "EntityPrototype.h"

//////////////////////////////////////////////////////////////////////////
// CEntityPrototypeLibrary implementation.
//////////////////////////////////////////////////////////////////////////
bool CEntityPrototypeLibrary::Save()
{
	return SaveLibrary("EntityPrototypeLibrary");
}

//////////////////////////////////////////////////////////////////////////
bool CEntityPrototypeLibrary::Load(const string& filename)
{
	if (filename.IsEmpty())
		return false;
	SetFilename(filename);
	XmlNodeRef root = XmlHelpers::LoadXmlFromFile(filename);
	if (!root)
		return false;

	Serialize(root, true);
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CEntityPrototypeLibrary::Serialize(XmlNodeRef& root, bool bLoading)
{
	if (bLoading)
	{
		// Loading.
		RemoveAllItems();
		string name = GetName();
		root->getAttr("Name", name);
		SetName(name);
		for (i32 i = 0; i < root->getChildCount(); i++)
		{
			CEntityPrototype* prototype = new CEntityPrototype;
			AddItem(prototype);
			XmlNodeRef itemNode = root->getChild(i);
			CBaseLibraryItem::SerializeContext ctx(itemNode, bLoading);
			prototype->Serialize(ctx);
		}
		SetModified(false);
	}
	else
	{
		// Saving.
		root->setAttr("Name", GetName());

		// Serialize prototypes.
		for (i32 i = 0; i < GetItemCount(); i++)
		{
			XmlNodeRef itemNode = root->newChild("EntityPrototype");
			CBaseLibraryItem::SerializeContext ctx(itemNode, bLoading);
			GetItem(i)->Serialize(ctx);
		}
	}
}

