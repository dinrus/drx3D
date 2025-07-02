// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2002-2012.
// -------------------------------------------------------------------------
//  File name:   TrackViewFBXImportPreviewDialog.h
//  Version:     v1.00
//  Created:     3/12/2012 by Konrad.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio 2010
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __TRACKVIEW_FBX_IMPORT_PREVIEW_DIALOG_H__
#define __TRACKVIEW_FBX_IMPORT_PREVIEW_DIALOG_H__
#pragma once

class CTrackViewFBXImportPreviewDialog : public CDialog
{
	DECLARE_DYNAMIC(CTrackViewFBXImportPreviewDialog)

public:
	CTrackViewFBXImportPreviewDialog();
	virtual ~CTrackViewFBXImportPreviewDialog(){}

	void AddTreeItem(const CString& objectName);
	bool IsObjectSelected(const CString& objectName) { return m_fBXItemNames[objectName]; };

private:

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	void         OnClickTree(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()

private:
	typedef std::map<CString, bool> TItemsMap;
	CTreeCtrl m_tree;
	TItemsMap m_fBXItemNames;
};

#endif

