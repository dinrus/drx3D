// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "Controls/EditorDialog.h"

class CWnd;

class StandaloneMaterialEditor : public CEditorDialog
{
public:
	StandaloneMaterialEditor();
	~StandaloneMaterialEditor();

	void Execute();

private:

	virtual void closeEvent(QCloseEvent * event) override;

	void OnIdleCallback();

	CWnd* m_materialWnd;
};

