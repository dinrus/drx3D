// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <DrxParticleSystem/IParticlesPfx2.h>

#include <NodeGraph/IDrxGraphEditor.h>
#include <NodeGraph/NodeHeaderIconWidget.h>

namespace DrxParticleEditor {

class CParentPinItem;
class CFeaturePinItem;
class CFeatureItem;

typedef std::vector<CFeatureItem*> FeatureItemArray;

enum EIcon : i16
{
	Icon_Enabled,
	Icon_Disabled,
	Icon_NodeSelected,
	Icon_NodeDeactivated,

	Icon_Count
};

typedef DrxGraphEditor::CIconArray<Icon_Count> IconMap;

class CEmitterActiveIcon : public DrxGraphEditor::CNodeHeaderIcon
{
public:
	CEmitterActiveIcon(DrxGraphEditor::CNodeWidget& nodeWidget);
	~CEmitterActiveIcon();

	// DrxGraphEditor::CNodeHeaderIcon
	virtual void OnClicked();
	// ~DrxGraphEditor::CNodeHeaderIcon

protected:
	void OnNodeSelectionChanged(bool isSelected);
	void OnDeactivatedChanged(bool isDeactivated);

private:
	static IconMap s_iconMap;
};

class CSoloEmitterModeIcon : public DrxGraphEditor::CNodeHeaderIcon
{
public:
	CSoloEmitterModeIcon(DrxGraphEditor::CNodeWidget& nodeWidget);
	~CSoloEmitterModeIcon();

	// DrxGraphEditor::CNodeHeaderIcon
	virtual void OnClicked();
	// ~DrxGraphEditor::CNodeHeaderIcon

protected:
	void OnNodeSelectionChanged(bool isSelected);
	void OnVisibilityChanged(bool isVisible);
	void OnDeactivatedChanged(bool isDeactivated);

	void UpdateIcon(bool isSelected, bool isVisible, bool isDeactivated, bool isSoloNode);

private:
	static IconMap s_iconMap;
};

class CEmitterVisibleIcon : public DrxGraphEditor::CNodeHeaderIcon
{
public:
	CEmitterVisibleIcon(DrxGraphEditor::CNodeWidget& nodeWidget);
	~CEmitterVisibleIcon();

	// DrxGraphEditor::CNodeHeaderIcon
	virtual void OnClicked();
	// ~DrxGraphEditor::CNodeHeaderIcon

protected:
	void OnNodeSelectionChanged(bool isSelected);
	void OnVisibilityChanged(bool isVisible);
	void OnDeactivatedChanged(bool isDeactivated);

	void UpdateIcon(bool isSelected, bool isVisible, bool isDeactivated);

private:
	static IconMap s_iconMap;
};

}

