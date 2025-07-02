// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <memory>
#include <QWidget>
#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Color.h>
#include <drx3D/CoreX/Serialization/Forward.h>

class BlockPalette;
struct BlockPaletteItem;
class QPropertyTree;

namespace CharacterTool {

struct System;
struct AnimEventPreset;
struct AnimEventPresetCollection;

typedef std::vector<std::pair<u32, ColorB>> EventContentToColorMap;

class AnimEventPresetPanel : public QWidget
{
	Q_OBJECT
public:
	AnimEventPresetPanel(QWidget* parent, System* system);
	~AnimEventPresetPanel();

	const AnimEventPreset*        GetPresetByHotkey(i32 number) const;
	const EventContentToColorMap& GetEventContentToColorMap() const { return m_eventContentToColorMap; }
	void                          LoadPresets();
	void                          SavePresets();

	void                          Serialize(Serialization::IArchive& ar);
signals:
	void                          SignalPutEvent(const AnimEventPreset& preset);
	void                          SignalPresetsChanged();
protected slots:
	void                          OnPropertyTreeChanged();
	void                          OnPaletteSelectionChanged();
	void                          OnPaletteItemClicked(const BlockPaletteItem& item);
	void                          OnPaletteChanged();

private:
	void WritePalette();
	void ReadPalette();
	void PresetsChanged();
	EventContentToColorMap                     m_eventContentToColorMap;
	BlockPalette*                              m_palette;
	QPropertyTree*                             m_tree;
	std::unique_ptr<AnimEventPresetCollection> m_presetCollection;
};

}

