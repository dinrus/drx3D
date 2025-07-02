// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QToolBar>
#include "TrackViewCoreComponent.h"

class CTrackViewSequenceToolbar : public QToolBar, public CTrackViewCoreComponent
{
	Q_OBJECT

public:
	CTrackViewSequenceToolbar(CTrackViewCore* pTrackViewCore);
	~CTrackViewSequenceToolbar() {}

protected:
	virtual void        OnTrackViewEditorEvent(ETrackViewEditorEvent event) override {}
	virtual tukk GetComponentTitle() const override                           { return "Sequence Toolbar"; }

private slots:
	void OnAddSelectedEntities();
	void OnShowProperties();
};

