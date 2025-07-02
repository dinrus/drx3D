// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "LensFlareManager.h"
#include "LensFlareLibrary.h"
#include "LensFlareItem.h"
#include <drx3D/CoreX/Renderer/IFlares.h>

bool CLensFlareLibrary::Save()
{
	return SaveLibrary("LensFlareLibrary");
}

bool CLensFlareLibrary::Load(const string& filename)
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

void CLensFlareLibrary::Serialize(XmlNodeRef& root, bool bLoading)
{
	if (bLoading)
	{
		RemoveAllItems();
		string name = GetName();
		root->getAttr("Name", name);
		SetName(name);
		for (i32 i = 0; i < root->getChildCount(); i++)
		{
			XmlNodeRef itemNode = root->getChild(i);
			CLensFlareItem* pLensFlareItem = new CLensFlareItem;
			AddItem(pLensFlareItem);
			CBaseLibraryItem::SerializeContext ctx(itemNode, bLoading);
			pLensFlareItem->Serialize(ctx);
		}
		SetModified(false);
	}
	else
	{
		root->setAttr("Name", GetName());

		for (i32 i = 0; i < GetItemCount(); i++)
		{
			CLensFlareItem* pItem = (CLensFlareItem*)GetItem(i);
			CBaseLibraryItem::SerializeContext ctx(pItem->CreateXmlData(), bLoading);
			root->addChild(ctx.node);
			pItem->Serialize(ctx);
		}
	}
}

IOpticsElementBasePtr CLensFlareLibrary::GetOpticsOfItem(tukk szflareName)
{
	IOpticsElementBasePtr pOptics = NULL;
	for (i32 i = 0; i < GetItemCount(); i++)
	{
		CLensFlareItem* pItem = (CLensFlareItem*)GetItem(i);

		if (pItem->GetFullName() == szflareName)
		{
			pOptics = pItem->GetOptics();
			break;
		}
	}

	return pOptics;
}

