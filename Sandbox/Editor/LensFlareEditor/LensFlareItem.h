// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2011.
// -------------------------------------------------------------------------
//  File name:   LensFlareItem.h
//  Created:     12/Dec/2012 by Jaesik.
////////////////////////////////////////////////////////////////////////////
#include "BaseLibraryItem.h"

class IOpticsElementBase;

class CLensFlareItem : public CBaseLibraryItem
{
public:

	CLensFlareItem();
	~CLensFlareItem();

	EDataBaseItemType GetType() const
	{
		return EDB_TYPE_FLARE;
	}

	void                  SetName(const string& name);
	void                  SetName(const string& name, bool bRefreshWhenUndo, bool bRefreshWhenRedo);
	void                  Serialize(SerializeContext& ctx);

	void                  CreateOptics();

	IOpticsElementBasePtr GetOptics() const
	{
		return m_pOptics;
	}

	void       ReplaceOptics(IOpticsElementBasePtr pNewData);

	XmlNodeRef CreateXmlData() const;
	void       UpdateLights(IOpticsElementBasePtr pSrcOptics = NULL);

private:

	void        GetLightEntityObjects(std::vector<CEntityObject*>& outEntityLights) const;
	static void AddChildOptics(IOpticsElementBasePtr pParentOptics, XmlNodeRef& pNode);

	IOpticsElementBasePtr m_pOptics;
};

