// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2002-2013.
// -------------------------------------------------------------------------
//  File name:   TrackViewExportKeyTimeDlg.h
//  Version:     v1.00
//  Created:     25/6/2013 by Konrad.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio 2010
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __TRACKVIEW_EXPORT_KEY_TIME_DIALOG_H__
#define __TRACKVIEW_EXPORT_KEY_TIME_DIALOG_H__
#pragma once

class CTrackViewExportKeyTimeDlg : public CDialog
{
	DECLARE_DYNAMIC(CTrackViewExportKeyTimeDlg)

public:
	CTrackViewExportKeyTimeDlg();
	virtual ~CTrackViewExportKeyTimeDlg(){}

	bool IsAnimationExportChecked() { return m_bAnimationTimeExport; };
	bool IsSoundExportChecked()     { return m_bSoundTimeExport; };

private:

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	void         SetChecks();

	DECLARE_MESSAGE_MAP()

private:
	CButton m_animationTimeExport;
	CButton m_soundTimeExport;
	bool    m_bAnimationTimeExport;
	bool    m_bSoundTimeExport;

};
#endif

