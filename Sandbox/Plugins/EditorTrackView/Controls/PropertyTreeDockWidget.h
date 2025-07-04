// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QWidget>

#include "TrackViewCoreComponent.h"

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/IArchiveHost.h>
#include <drx3D/CoreX/Serialization/Enum.h>

class QVBoxLayout;
class QPropertyTree;
class CTrackViewSequenceTabWidget;

namespace
{
struct SSelectedKey;
}

class CTrackViewPropertyTreeWidget : public QWidget, public CTrackViewCoreComponent
{
	Q_OBJECT

	struct STrackViewPropertiesRoot
	{
		CTrackViewCore&         core;
		Serialization::SStructs structs;
		yasli::Context          entityContext;
		yasli::Context          sequenceContext;
		yasli::Context          animTimeContext;

		STrackViewPropertiesRoot(CTrackViewCore& _core);

		void Serialize(Serialization::IArchive& ar);

		void SetEntityContext(Serialization::IArchive& ar, const SSelectedKey& key, CTrackViewSequenceTabWidget* pTabWidget);
		void Clear();
		operator Serialization::SStruct();
	};

public:
	CTrackViewPropertyTreeWidget(CTrackViewCore* pTrackViewCore);
	~CTrackViewPropertyTreeWidget() {}

	// CTrackViewCoreComponent
	virtual void        OnTrackViewEditorEvent(ETrackViewEditorEvent event) override;
	virtual tukk GetComponentTitle() const override { return "Property Pane"; }
	// ~CTrackViewCoreComponent

	virtual QSize sizeHint() const { return QSize(80, 100); }

	void          Update();

protected:
	// CTrackViewCoreComponent
	virtual void OnNodeChanged(CTrackViewNode* pNode, ENodeChangeType type) override;
	// ~CTrackViewCoreComponent

private slots:
	void OnPropertiesUndoPush();
	void OnPropertiesChanged();

private:
	bool                     m_bUndoPush;
	//bool m_bDontUpdateProperties;
	QPropertyTree*           m_pPropertyTree;
	std::vector<STrackViewPropertiesRoot> m_properties;
};

