// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "ViewportMode.h"
#include <QObject>
#include <memory>
#include <drx3D/CoreX/Serialization/Forward.h>

namespace Manip {
class CScene;
}

class QAction;

namespace CharacterTool
{

struct CharacterDefinition;

using std::unique_ptr;

class ModeCharacter : public QObject, public IViewportMode
{
	Q_OBJECT
public:
	ModeCharacter();

	void Serialize(Serialization::IArchive& ar) override;
	void EnterMode(const SModeContext& context) override;
	void LeaveMode() override;

	void GetPropertySelection(vector<ukk>* selection) const override;
	void SetPropertySelection(const vector<ukk>& items) override;

	void OnViewportRender(const SRenderContext& rc) override;
	void OnViewportKey(const SKeyEvent& ev) override;
	void OnViewportMouse(const SMouseEvent& ev) override;

	bool OnViewportMouseProxy(const SMouseEvent& ev);

	void CommenceRagdollTest();
protected slots:
	void OnTransformPanelChanged();
	void OnTransformPanelChangeFinished();
	void OnTransformPanelSpaceChanged();

	void OnSceneSelectionChanged();
	void OnSceneElementsChanged(u32 layerBits);
	void OnSceneElementContinuousChange(u32 layerBits);
	void OnScenePropertiesChanged();
	void OnSceneManipulationModeChanged();
	void OnSceneUndo();
	void OnSceneRedo();

	void OnBindPoseModeChanged();
	void OnDisplayOptionsChanged();
	void OnSubselectionChanged(i32 layer);
	void OnGizmoChanged();

	void OnActionRotateTool();
	void OnActionMoveTool();
	void OnActionScaleTool();

private:
	void WriteScene(const CharacterDefinition& def);
	void ReadScene(CharacterDefinition* def) const;
	void WriteTransformPanel();
	void UpdateToolbar();
	void HandleSceneChange(i32 layerMask, bool continuous);
	void OnClickProxyCreate();

	ICharacterInstance*         m_character;
	unique_ptr<QAction>         m_actionMoveTool;
	unique_ptr<QAction>         m_actionRotateTool;
	unique_ptr<QAction>         m_actionScaleTool;

	unique_ptr<Manip::CScene>   m_scene;
	bool                        m_ignoreSceneSelectionChange;
	System*                     m_system;
	CharacterDocument*          m_document;
	CharacterToolForm*          m_window;
	TransformPanel*             m_transformPanel;
	std::vector<QPropertyTree*> m_layerPropertyTrees;

	string                      m_curBoneName;
	Vec2                        m_posMouse;
	bool                        m_isCurBoneFree;
};

}

