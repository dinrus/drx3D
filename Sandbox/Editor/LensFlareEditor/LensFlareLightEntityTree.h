// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2011.
// -------------------------------------------------------------------------
//  File name:   LensFlareLightEntityTree.h
//  Created:     18/Dec/2012 by Jaesik.
////////////////////////////////////////////////////////////////////////////

#include "ILensFlareListener.h"

class CLensFlareLightEntityTree : public CXTTreeCtrl, public ILensFlareChangeItemListener
{
public:
	CLensFlareLightEntityTree();
	~CLensFlareLightEntityTree();

	void OnLensFlareDeleteItem(CLensFlareItem* pLensFlareItem);
	void OnLensFlareChangeItem(CLensFlareItem* pLensFlareItem);
	void OnObjectEvent(CBaseObject* pObject, i32 nEvent);

protected:

	HTREEITEM FindItem(HTREEITEM hStartItem, CBaseObject* pObject) const;
	void      AddLightEntity(CEntityObject* pEntity);

	_smart_ptr<CLensFlareItem> m_pLensFlareItem;

	DECLARE_MESSAGE_MAP()

	void OnTvnItemDoubleClicked(NMHDR* pNMHDR, LRESULT* pResult);
};

