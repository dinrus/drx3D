// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Controls/EditorDialog.h"

class QLabel;
class QCheckBox;

class QXMLExporterDlg : public CEditorDialog
{
public:
	QXMLExporterDlg();

	bool GetResult(bool& bExportAnimation, bool& bExportAudio) const;

protected:
	void OnConfirmed();

	QLabel*    m_pLabel;
	QCheckBox* m_pExportAnimation;
	QCheckBox* m_pExportAudio;

	bool       m_bConfirmed;
};

