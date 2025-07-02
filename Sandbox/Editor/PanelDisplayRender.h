// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#pragma once

#include <QWidget>
#include <drx3D/Sandbox/Editor/EditorCommon/QtViewPane.h>
#include "QScrollableBox.h"
#include <drx3D/Sandbox/Editor/EditorCommon/QtViewPane.h>
#include "Viewport.h"

struct ICVar;
class CViewport;

class CPanelDisplayRender : public CDockableWidgetT<QScrollableBox>, public IAutoEditorNotifyListener
{
public:
	CPanelDisplayRender(QWidget* parent = nullptr, CViewport* viewport = nullptr);
	~CPanelDisplayRender();

	void        Serialize(Serialization::IArchive& ar);
	QSize       sizeHint() const override;

	tukk GetPaneTitle() const
	{
		return "Display Settings";
	}

	void OnCVarChangedCallback();

protected:
	void SetupCallbacks();
	void RemoveCallbacks();

	//////////////////////////////////////////////////////////////////////////
	// CEditorNotifyListener
	virtual void OnEditorNotifyEvent(EEditorNotifyEvent event);
	//////////////////////////////////////////////////////////////////////////

	void OnChangeRenderFlag();
	void OnChangeDebugFlag();
	void OnDisplayAll();
	void OnDisplayNone();
	void OnDisplayInvert();

	void OnHideObjectsAll();
	void OnHideObjectsNone();
	void OnHideObjectsInvert();

	void RegisterChangeCallback(tukk szVariableName, ConsoleVarFunc fnCallbackFunction);
	void UnregisterChangeCallback(tukk szVariableName);

	void SetObjectHideMask(u32 mask);

	void SerializeStereo(Serialization::IArchive& ar);

	void showEvent(QShowEvent* e) override;

protected:
	class QPropertyTree*               m_propertyTree;
	std::unordered_map<ICVar*, uint64> m_varCallbackMap;
	CViewport*                         m_pViewport;
};

