// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SELECTMISSIONOBJECTIVEDIALOG_H__
#define __SELECTMISSIONOBJECTIVEDIALOG_H__
#pragma once

#include "GenericSelectItemDialog.h"

// CSelectSequence dialog

class CSelectMissionObjectiveDialog : public CGenericSelectItemDialog
{
	DECLARE_DYNAMIC(CSelectMissionObjectiveDialog)
	CSelectMissionObjectiveDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectMissionObjectiveDialog() {}

protected:
	virtual BOOL OnInitDialog();

	// Derived Dialogs should override this
	virtual void GetItems(std::vector<SItem>& outItems);
	void         GetItemsInternal(std::vector<SItem>& outItems, tukk path, const bool isOptional);

	// Called whenever an item gets selected
	virtual void ItemSelected();

	bool         GetItemsFromFile(std::vector<SItem>& outItems, tukk fileName);

	struct SObjective
	{
		CString shortText;
		CString longText;
	};

	typedef std::map<CString, SObjective, stl::less_stricmp<CString>> TObjMap;
	TObjMap m_objMap;
};

#endif // __SELECTMISSIONOBJECTIVEDIALOG_H__

