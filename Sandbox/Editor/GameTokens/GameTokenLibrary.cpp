// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "GameTokenLibrary.h"
#include "GameTokenItem.h"

//////////////////////////////////////////////////////////////////////////
// CGameTokenLibrary implementation.
//////////////////////////////////////////////////////////////////////////
bool CGameTokenLibrary::Save()
{
	return SaveLibrary("GameTokensLibrary");
}

//////////////////////////////////////////////////////////////////////////
bool CGameTokenLibrary::Load(const string& filename)
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
void CGameTokenLibrary::Serialize(XmlNodeRef& root, bool bLoading)
{
	if (bLoading)
	{
		// Loading.
		string name = GetName();
		root->getAttr("Name", name);
		SetName(name);
		for (i32 i = 0; i < root->getChildCount(); i++)
		{
			XmlNodeRef itemNode = root->getChild(i);
			// Only accept nodes with correct name.
			if (stricmp(itemNode->getTag(), "GameToken") != 0)
				continue;
			CBaseLibraryItem* pItem = new CGameTokenItem;
			AddItem(pItem);

			CBaseLibraryItem::SerializeContext ctx(itemNode, bLoading);
			pItem->Serialize(ctx);
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
			XmlNodeRef itemNode = root->newChild("GameToken");
			CBaseLibraryItem::SerializeContext ctx(itemNode, bLoading);
			GetItem(i)->Serialize(ctx);
		}
	}
}

