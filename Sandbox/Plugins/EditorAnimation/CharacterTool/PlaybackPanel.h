// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QWidget>
#include <vector>
#include <memory>
#include <drx3D/CoreX/Serialization/Forward.h>
#include <DrxMovie/AnimTime.h>
#include "DrxIcon.h"

class QPushButton;
class QLineEdit;
class QDoubleSpinBox;
class QLabel;
class CTimeline;

class QMenuComboBox;
struct STimelineContent;

namespace Explorer
{
struct ExplorerEntryModifyEvent;
}

namespace CharacterTool
{
using std::vector;
using std::unique_ptr;

struct AnimEventPreset;
struct System;
class AnimEventPresetPanel;

enum ETimeUnits
{
	TIME_IN_SECONDS,
	TIME_IN_FRAMES,
	TIME_NORMALIZED
};

class PlaybackPanel : public QWidget
{
	Q_OBJECT
public:
	PlaybackPanel(QWidget* parent, System* system, AnimEventPresetPanel* presetPalette);
	virtual ~PlaybackPanel();

	void Serialize(Serialization::IArchive& ar);

	bool HandleKeyEvent(i32 key);

public slots:
	void PutAnimEvent(const AnimEventPreset& preset);
	void OnPresetsChanged();

protected slots:
	void      OnTimelineScrub(bool scrubThrough);
	void      OnTimelineChanged(bool continuousChange);
	void      OnTimelineSelectionChanged(bool continuousChange);
	void      OnTimelineHotkey(i32 number);
	void      OnTimelineUndo();
	void      OnTimelineRedo();
	void      OnPlaybackTimeChanged();
	void      OnPlaybackStateChanged();
	void      OnPlaybackOptionsChanged();
	void      OnExplorerEntryModified(Explorer::ExplorerEntryModifyEvent& ev);
	void      OnDocumentActiveAnimationSwitched();

	void      OnPlayPausePushed();
	void      OnTimeEditEditingFinished();
	void      OnTimeEditValueChanged(double newTime);
	void      OnTimeUnitsChanged(i32 index);
	void      OnTimelinePlay();
	void      OnSpeedChanged(i32 index);
	void      OnLoopToggled(bool);
	void      OnOptionRestartOnSelection();
	void      OnOptionWrapSlider();
	void      OnOptionSmoothTimelineSlider();
	void      OnOptionFirstFrameAtEnd();
	void      OnOptionPlayFromTheStart();
	void      OnSubselectionChanged(i32 layer);

	void      OnEventsImport();
	void      OnEventsExport();
private:
	void      ResetTimelineZoom();
	void      WriteTimeline();
	void      ReadTimeline();

	void      UpdateTimeUnitsUI(bool timeChanged, bool durationChanged);
	float     FrameRate() const;

	SAnimTime AnimEventTimeToTimelineTime(float animEventTime) const;
	float     TimelineTimeToAnimEventTime(SAnimTime timelineTime) const;

	CTimeline*                   m_timeline;
	AnimEventPresetPanel*        m_presetPanel;
	QDoubleSpinBox*              m_timeEdit;
	bool                         m_timeEditChanging;
	QLabel*                      m_timeTotalLabel;
	QPushButton*                 m_playPauseButton;
	QMenuComboBox*               m_timeUnitsCombo;
	QMenuComboBox*               m_speedCombo;
	ETimeUnits                   m_timeUnits;
	QPushButton*                 m_loopButton;
	QPushButton*                 m_optionsButton;
	QAction*                     m_actionRestartOnSelection;
	QAction*                     m_actionWrapSlider;
	QAction*                     m_actionSmoothTimelineSlider;
	QAction*                     m_actionFirstFrameAtEnd;
	QAction*                     m_actionPlayFromTheStart;
	vector<QWidget*>             m_activeControls;
	System*                      m_system;
	unique_ptr<STimelineContent> m_timelineContent;
	vector<i32>                  m_selectedEvents;

	QString                      m_sAnimationProgressStatusReason;
	QString                      m_sAnimationName;
	QString                      m_sAnimationIDName;
	bool                         m_playing;
	DrxIcon                      m_playIcon;
	DrxIcon                      m_pauseIcon;

	float                        m_normalizedTime;
	float                        m_duration;
};

}

