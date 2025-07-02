// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// DrxEngine Header File.
// Copyright (C), DinrusPro 3D, 1999-2014.

#pragma once

//#include "ui_MainWindow.h"

#include "Timeline.h"
#include "CurveEditor.h"
#include "AnimationContext.h"
#include "TrackViewSequenceManager.h"
#include "Nodes/TrackViewSequence.h"

#include <DrxMovie/AnimKey.h>
#include <drx3D/CoreX/Extension/DrxGUID.h>
#include <IViewPane.h>

class CTrackViewPropertyTreeDockWidget;
class CTrackViewSequencesListDockWidget;
class CTrackViewSequenceTabContainerWidget;
class CTrackViewPlaybackControlsToolbar;
class CTrackViewSequenceToolbar;
class CTrackViewKeysToolbar;
class CTrackViewFramerateSettingsToolbar;

class QListWidgetItem;

namespace
{
struct SAddableNode
{
	EAnimNodeType m_type;
	string        m_name;
};

struct SAddableTrack
{
	CAnimParamType m_type;
	string         m_name;
};
}

class CTrackViewMainWindow : /*public CEditor,*/ public CDockableWindow,
	                           public IAnimationContextListener,
	                           public ITrackViewSequenceManagerListener,
	                           public ITrackViewSequenceListener
{
	Q_OBJECT
public:
	CTrackViewMainWindow();
	~CTrackViewMainWindow();

	void                                      ShowNode(CTrackViewAnimNode* pNode);

	static std::vector<CTrackViewMainWindow*> GetTrackViewWindows() { return ms_trackViewWindows; }

	//////////////////////////////////////////////////////////
	// CEditor implementation
	//  virtual bool OnNew()    override { return true; }
	//  virtual bool OnOpen()   override { return true; }
	//  virtual bool OnClose()  override { return true; }
	//  virtual bool OnSave()   override { return true; }
	//  virtual bool OnSaveAs() override { return true; }
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// CDockableWindow implementation
	virtual tukk                       GetPaneTitle() const override        { return "Track View"; };
	virtual IViewPaneClass::EDockingDirection GetDockingDirection() const override { return IViewPaneClass::DOCK_BOTTOM; }
	//////////////////////////////////////////////////////////

private slots:
	void OnDopeSheetTabChanged(i32 index);
	void OnDopeSheetTabClose(i32 index);

	void OnSequenceListItemPressed(QListWidgetItem* pItem);
	void OnSequenceListItemDoubleClicked(QListWidgetItem* pItem);

	void OnDopesheetScrub();
	void OnDopesheetContentChanged(bool continuous);
	void OnDopesheetTrackSelectionChanged();
	void OnDopesheetTreeContextMenu(const QPoint& point);
	void OnDopesheetTracksBeginDrag(STimelineTrack* pTarget, bool& bValid);
	void OnDopesheetTracksDragging(STimelineTrack* pTarget, bool& bValid);
	void OnDopesheetTracksDragged(STimelineTrack* pTarget);

	void OnCurveEditorContentChanged();
	void OnCurveEditorScrub();

	void OnPropertiesUndoPush();
	void OnPropertiesChanged();

	void OnAddNodeButtonClicked();
	void OnAddTrack();
	void OnAddNode();
	void OnDeleteNodes();
	void OnSelectEntity();

	void OnPlayPause(bool bState);
	void OnRecord(bool bState);
	void OnStop();

	void OnGoToStart();
	void OnGoToEnd();

	void OnAddSelectedEntities();
	void OnAddDirectorNode();

	void OnGoToPreviousKey();
	void OnGoToNextKey();

	void OnNoSnapping(bool bState);
	void OnSnapTime(bool bState);
	void OnSnapKey(bool bState);

	void OnSyncSelectedTracksToBase();
	void OnSyncSelectedTracksFromBase();

	void OnNewSequence();
	void OnSequencesFilterChanged();

private:
	struct SSequenceData
	{
		SSequenceData()
			: m_startTime(0.0f)
			, m_endTime(0.0f)
			, m_bPlaying(false)
			, m_pDopeSheet(nullptr)
			, m_pCurveEditor(nullptr)
		{}

		SAnimTime                          m_startTime;
		SAnimTime                          m_endTime;
		bool                               m_bPlaying;

		CTimeline*                         m_pDopeSheet;
		CCurveEditor*                      m_pCurveEditor;

		STimelineContent                   m_timelineContent;
		SCurveEditorContent                m_curveEditorContent;
		std::map<DrxGUID, STimelineTrack*> m_uIdToTimelineTrackMap;
	};

	virtual bool eventFilter(QObject* pWatched, QEvent* pEvent) override;

	void         OpenSequenceTab(const DrxGUID sequenceGUID);

	// Dopesheet -> TrackView Model
	void ApplyDopeSheetChanges(CTimeline* pDopeSheet);
	void ApplyTrackChangesRecursive(const CTrackViewSequence* pSequence, STimelineTrack* pTrack);
	void UpdateTrackViewTrack(CTrackViewTrack* pTrack);
	void RemoveTrackViewNode(CTrackViewNode* pNode);

	// TrackView Model -> Dopesheet
	void InsertDopeSheetTrack(STimelineTrack* pParentTrack, STimelineTrack* pTrack, CTrackViewNode* pNode);
	void AddNodeToDopeSheet(CTrackViewNode* pNode);
	void RemoveNodeFromDopeSheet(CTrackViewNode* pNode);
	void UpdateDopeSheetTrack(CTrackViewTrack* pTrack);

	// TrackView Model -> curve editor
	void                UpdateCurveEditor(CTimeline* pDopeSheet, CCurveEditor* pCurveEditor);

	void                UpdateProperties();
	void                ShowKeyProperties();
	void                ShowNodeProperties();
	void                UpdateSequenceList();
	void                DeleteSelectedSequences();
	void                UpdateTabNames();
	void                FillAddTrackMenu(QMenu& addTrackMenu, const std::vector<SAddableTrack>& addableTracks) const;
	void                RemoveDeletedTracks(STimelineTrack* pTrack);
	CTrackViewAnimNode* CheckValidReparenting(CTimeline* pDopeSheet, const std::vector<STimelineTrack*>& selectedTracks, STimelineTrack* pTarget);

	DrxGUID             GetSequenceGUIDFromTabIndex(i32 index);
	DrxGUID             GetActiveSequenceGUID();
	SSequenceData*      GetActiveSequenceData();
	CTrackViewSequence* GetActiveSequence();

public:
	CTimeline*    GetActiveDopeSheet();
	CTimeline*    GetDopeSheetFromSequenceGUID(const DrxGUID& guid);

	CCurveEditor* GetActiveCurveEditor();
	CCurveEditor* GetCurveEditorFromSequenceGUID(const DrxGUID& guid);

private:
	CTrackViewSequence*         GetSequenceFromDopeSheet(const CTimeline* pDopeSheet);
	CTrackViewNode*             GetNodeFromDopeSheetTrack(const CTrackViewSequence* pSequence, const STimelineTrack* pDopeSheetTrack);
	STimelineTrack*             GetDopeSheetTrackFromNode(const CTrackViewNode* pNode);
	DrxGUID                     GetGUIDFromDopeSheetTrack(const STimelineTrack* pDopeSheetTrack) const;

	_smart_ptr<IAnimKeyWrapper> GetWrappedKeyFromElement(const STimelineTrack& track, const STimelineElement& element);

	void                        UpdateActiveSequence();

	// IAnimationContextListener
	virtual void OnSequenceChanged(CTrackViewSequence* pSequence) override;
	virtual void OnTimeChanged(SAnimTime newTime) override;
	virtual void OnPlaybackStateChanged(bool bPlaying) override;
	virtual void OnRecordingStateChanged(bool bRecording) override;
	// ~IAnimationContextListener

	// ITrackViewSequenceManagerListener
	virtual void OnSequenceAdded(CTrackViewSequence* pSequence) override;
	virtual void OnSequenceRemoved(CTrackViewSequence* pSequence) override;
	// ~ITrackViewSequenceManagerListener

	// ITrackViewSequenceListener
	virtual void OnSequenceSettingsChanged(CTrackViewSequence* pSequence) override;
	virtual void OnNodeChanged(CTrackViewNode* pNode, ENodeChangeType type) override;
	virtual void OnNodeRenamed(CTrackViewNode* pNode, tukk pOldName) override;
	// ~ITrackViewSequenceListener

	QTabWidget*                           m_pDopesheetTabs;
	CTrackViewSequenceTabContainerWidget* m_pTabContainerWidget;
	CTrackViewPropertyTreeDockWidget*     m_pPropertyTreeWidget;
	CTrackViewSequencesListDockWidget*    m_pSequencesListWidget;
	CTrackViewPlaybackControlsToolbar*    m_pPlaybackToolbar;
	CTrackViewSequenceToolbar*            m_pSequenceToolbar;
	CTrackViewKeysToolbar*                m_pKeysToolbar;
	CTrackViewFramerateSettingsToolbar*   m_pFrameSettingsToolbar;

	DrxGUID                               m_activeSequenceUId;

	std::map<DrxGUID, SSequenceData>      m_idToSequenceDataMap;
	std::map<QWidget*, DrxGUID>           m_tabToSequenceUIdMap;

	struct SSelectedKey
	{
		STimelineTrack*             m_pTrack;
		STimelineElement*           m_pElement;
		_smart_ptr<IAnimKeyWrapper> m_key;
	};

	CTimeline*                m_pCurrentSelectionDopeSheet;
	CCurveEditor*             m_pCurrentSelectionCurveEditor;
	std::vector<SSelectedKey> m_currentKeySelection;
	CTrackViewNode*           m_pCurrentSelectedNode;
	bool                      m_bDontUpdateProperties;
	bool                      m_bDontUpdateDopeSheet;
	bool                      m_bDontUpdateCurveEditor;

	enum eSnapMode
	{
		eSnapMode_NoSnapping = 0,
		eSnapMode_TimeSnapping,
		eSnapMode_KeySnapping
	};

	void OnSnapModeChanged(eSnapMode newMode, bool bEnabled);
	eSnapMode m_SnapMode;

	bool      m_bSnapModeCanUpdate;
	//bool m_bSnapTime;

	// Node context menu
	QPoint                                    m_contextMenuLocation;
	CTrackViewNode*                           m_pContextMenuNode;
	std::vector<SAddableNode>                 m_addableNodes;
	std::vector<SAddableTrack>                m_addableTracks;

	static std::vector<CTrackViewMainWindow*> ms_trackViewWindows;
};

