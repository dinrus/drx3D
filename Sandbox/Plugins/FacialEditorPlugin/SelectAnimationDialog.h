// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SELECTANIMATIONDIALOG_H__
#define __SELECTANIMATIONDIALOG_H__

#include "Dialogs/GenericSelectItemDialog.h"

struct ICharacterInstance;

class CSelectAnimationDialog : public CGenericSelectItemDialog
{
	DECLARE_DYNAMIC(CSelectAnimationDialog)
	CSelectAnimationDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectAnimationDialog() {}

	void    SetCharacterInstance(ICharacterInstance* pCharacterInstance);
	CString GetSelectedItem();

protected:
	virtual BOOL OnInitDialog();

	// Derived Dialogs should override this
	virtual void GetItems(std::vector<SItem>& outItems);

	// Called whenever an item gets selected
	virtual void ItemSelected();

	ICharacterInstance* m_pCharacterInstance;
};

#endif //__SELECTANIMATIONDIALOG_H__

