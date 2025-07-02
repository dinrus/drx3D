// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FacialEditorDialog_h__
#define __FacialEditorDialog_h__
#pragma once

#include "Dialogs/ToolbarDialog.h"
//#include "FacialEditorView.h"
#include "FacialPreviewDialog.h"
#include "FacialPreviewOptionsDialog.h"
#include "FacialSlidersDialog.h"
#include "FacialExpressionsDialog.h"
#include "FacialSequenceDialog.h"
#include "FacialEdContext.h"
#include "FacialJoystickDialog.h"
#include "FacialVideoFrameDialog.h"
#include "ReportDialog.h"
#include <drx3D/CoreX/Containers/VectorMap.h>
#include <drx3D/CoreX/StlUtils.h>

#define FACIAL_EDITOR_NAME "Facial Editor"
#define FACIAL_EDITOR_VER  "1.00"

class CFacialEditorView : public CView
{
public:
	// Creates view window.
	BOOL         Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual void OnDraw(CDC* pDC);
	virtual void PostNcDestroy() {};
};

class IFacialEditor
{
public:

	virtual i32         GetNumMorphTargets() const = 0;
	virtual tukk GetMorphTargetName(i32 index) const = 0;
	virtual void        PreviewEffector(i32 index, float value) = 0;
	virtual void        ClearAllPreviewEffectors() = 0;
	virtual void        SetForcedNeckRotation(const Quat& rotation) = 0;
	virtual void        SetForcedEyeRotation(const Quat& rotation, EyeType eye) = 0;
	virtual i32         GetJoystickCount() const = 0;
	virtual tukk GetJoystickName(i32 joystickIndex) const = 0;
	virtual void        SetJoystickPosition(i32 joystickIndex, float x, float y) = 0;
	virtual void        GetJoystickPosition(i32 joystickIndex, float& x, float& y) const = 0;
	virtual void        LoadJoystickFile(tukk filename) = 0;
	virtual void        LoadCharacter(tukk filename) = 0;
	virtual void        LoadSequence(tukk filename) = 0;
	virtual void        SetVideoFrameResolution(i32 width, i32 height, i32 bpp) = 0;
	virtual i32         GetVideoFramePitch() = 0;
	virtual uk       GetVideoFrameBits() = 0;
	virtual void        ShowVideoFramePane() = 0;
};

//////////////////////////////////////////////////////////////////////////
//
// Main Dialog for Facial Editor.
//
//////////////////////////////////////////////////////////////////////////
class CFacialEditorDialog : public CXTPFrameWnd, public IFacialEdListener, public IFacialEditor
{
	DECLARE_DYNCREATE(CFacialEditorDialog)
public:
	CFacialEditorDialog();
	~CFacialEditorDialog();

	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd);

	// IFacialEditor
	virtual i32         GetNumMorphTargets() const;
	virtual tukk GetMorphTargetName(i32 index) const;
	virtual void        PreviewEffector(i32 index, float value);
	virtual void        ClearAllPreviewEffectors();
	virtual void        SetForcedNeckRotation(const Quat& rotation);
	virtual void        SetForcedEyeRotation(const Quat& rotation, EyeType eye);
	virtual i32         GetJoystickCount() const;
	virtual tukk GetJoystickName(i32 joystickIndex) const;
	virtual void        SetJoystickPosition(i32 joystickIndex, float x, float y);
	virtual void        GetJoystickPosition(i32 joystickIndex, float& x, float& y) const;
	virtual void        LoadJoystickFile(tukk filename);
	virtual void        LoadCharacter(tukk filename);
	virtual void        LoadSequence(tukk filename);
	virtual void        SetVideoFrameResolution(i32 width, i32 height, i32 bpp);
	virtual i32         GetVideoFramePitch();
	virtual uk       GetVideoFrameBits();
	virtual void        ShowVideoFramePane();

	// IEditorNotifyListener
	//virtual void OnEditorNotifyEvent( EEditorNotifyEvent event );

protected:
	DECLARE_MESSAGE_MAP()

	virtual void            OnOK()     {};
	virtual void            OnCancel() {};

	virtual BOOL            PreTranslateMessage(MSG* pMsg);
	virtual BOOL            OnInitDialog();
	virtual void            DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	virtual void            PostNcDestroy();
	afx_msg void            OnBeginComponentsDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void            OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void            OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void            OnSize(UINT nType, i32 cx, i32 cy);
	afx_msg void            OnSetFocus(CWnd* pOldWnd);
	afx_msg void            OnDestroy();
	afx_msg void            OnClose();
	afx_msg void            OnPaint();
	afx_msg BOOL            OnEraseBkgnd(CDC* pDC);
	afx_msg void            SetProjectTitle(CString& getTitle);

	afx_msg void            OnProjectNew();
	afx_msg void            OnProjectOpen();
	afx_msg void            OnProjectSave();
	afx_msg void            OnProjectSaveAs();

	afx_msg void            OnLibraryNew();
	afx_msg void            OnLibraryOpen();
	afx_msg void            OnLibrarySave();
	afx_msg void            OnLibrarySaveAs();
	afx_msg void            OnLibraryExport();
	afx_msg void            OnLibraryImport();
	afx_msg void            OnLibraryBatchUpdateLibraries();

	afx_msg void            OnSequenceNew();
	afx_msg void            OnSequenceOpen();
	afx_msg void            OnSequenceSave();
	afx_msg void            OnSequenceSaveAs();
	afx_msg void            OnSequenceLoadSound();
	afx_msg void            OnSequenceLoadSkeletonAnimation();
	afx_msg void            OnSequenceLipSync();
	afx_msg void            OnSequenceExportSelectedExpressions();
	afx_msg void            OnSequenceImportExpressions();
	afx_msg void            OnSequenceBatchUpdateSequences();
	afx_msg void            OnSequenceLoadVideoExtractedSequence();
	afx_msg void            OnSequenceLoadVideoIgnoreSequence();
	afx_msg void            OnSequenceLoadC3DFile();
	afx_msg void            OnSeqenceLoadGroupFile();
	afx_msg void            OnSequenceImportFBX();

	afx_msg void            OnJoysticksNew();
	afx_msg void            OnJoysticksOpen();
	afx_msg void            OnJoysticksSave();
	afx_msg void            OnJoysticksSaveAs();
	afx_msg void            OnCreateExpressionFromCurrentPositions();

	afx_msg void            OnLoadCharacter();

	afx_msg LRESULT         OnDockingPaneNotify(WPARAM wParam, LPARAM lParam);

	afx_msg BOOL            OnToggleBar(UINT nID);
	afx_msg void            OnUpdateControlBar(CCmdUI* pCmdUI);
	afx_msg void            OnBatchProcess_PhonemeExtraction();
	afx_msg void            OnBatchProcess_ApplyExpression();

	afx_msg void            OnMorphCheck();
	afx_msg BOOL            OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	afx_msg void            OnCustomize();
	afx_msg void            OnExportShortcuts();
	afx_msg void            OnImportShortcuts();

	virtual BOOL            OnCmdMsg(UINT nID, i32 nCode, uk pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

	CXTPDockingPaneManager* GetDockingPaneManager() { return &m_paneManager; }

protected:
	void OnStartStop();
	void OnNextKey();
	void OnPrevKey();
	void OnNextFrame();
	void OnPrevFrame();
	void OnKeyAll();
	void OnSelectAll();

	class PhonemeExtractionSoundEntry
	{
	public:
		tukk sOriginalActorLine;
		tukk sUtf8TranslatedActorLine;
	};
	typedef VectorMap<string, PhonemeExtractionSoundEntry, stl::less_stricmp<tukk >> PhonemeExtractionSoundDatabase;
	void                                CreatePhonemeExtractionSoundDatabase(PhonemeExtractionSoundDatabase& db);
	CString                             MakeSafeText(const string& text);

	virtual void                        OnFacialEdEvent(EFacialEdEvent event, IFacialEffector* pEffector, i32 nChannelCount, IFacialAnimChannel** ppChannels);

	void                                SetContext(CFacialEdContext* pContext);
	bool                                SaveCurrentProject(bool bSaveAs = false);
	bool                                SaveCurrentLibrary(bool bSaveAs = false);
	bool                                SaveCurrentSequence(bool bSaveAs = false);
	bool                                SaveCurrentJoysticks(bool bSaveAs = false);
	void                                MakeNewProject();
	bool                                CloseCurrentSequence();
	bool                                CloseCurrentLibrary();
	bool                                CloseCurrentJoysticks();

	void                                UpdateVideoSequencePlayback(float time);
	void                                MergeVideoExtractedSequence(tukk filename, float startTime, bool loadSequence = true);
	void                                LoadGroupFile(tukk filename);
	void                                LoadC3DFile(tukk filename);
	void                                LoadFBXToSequence(tukk filename);

	void                                DisplayCurrentVideoFrame();

	_smart_ptr<IFacialEffectorsLibrary> CreateLibraryOfSelectedEffectors();
	void                                UpdateSkeletonAnimText();

	// Helper Functions, do not call directly - Call ::LoadFBXToSequence instead
	bool ImportFBXAsSequence(IFacialAnimSequence* newSequence, tukk filename);

private:
	CFacialPreviewDialog        m_panePreview;
	CFacialPreviewOptionsDialog m_panePreviewOptions;
	CTabCtrl                    m_slidersTab;
	CFacialExpressionsDialog    m_paneExpressions;
	CFacialSlidersDialog        m_paneSliders;
	CFacialSequenceDialog       m_paneSequence;
	CFacialJoystickDialog       m_paneJoysticks;
	CFacialVideoFrameDialog     m_paneVideoFrame;

	CFacialEditorView           m_view;
	CXTPDockingPaneManager      m_paneManager;
	CImageList*                 m_pDragImage;

	CString                     m_lastCharacter;
	CString                     m_lastSequence;
	CString                     m_lastLibrary;
	CString                     m_lastProject;

	CFacialEdContext*           m_pContext;
	CReportDialog               m_morphCheckReportDialog;
	CReport                     m_morphCheckReport;
};

#endif // __FacialEditorDialog_h__

