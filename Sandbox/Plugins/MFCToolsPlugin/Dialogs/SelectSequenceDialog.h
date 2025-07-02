// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SELECTSEQUENCEDIALOG_H__
#define __SELECTSEQUENCEDIALOG_H__
#pragma once

#include "GenericSelectItemDialog.h"

// CSelectSequence dialog

class CSelectSequenceDialog : public CGenericSelectItemDialog
{
	DECLARE_DYNAMIC(CSelectSequenceDialog)
	CSelectSequenceDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectSequenceDialog() {}

protected:
	virtual BOOL OnInitDialog();

	// Derived Dialogs should override this
	virtual void GetItems(std::vector<SItem>& outItems);
};

#endif // __SELECTSEQUENCEDIALOG_H__

