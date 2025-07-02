// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <DrxMovie/IMovieSystem.h>
#include <DrxMovie/AnimKey.h>
#include <QItemSelection>

#include "Controls/EditorDialog.h"

//////////////////////////////////////////////////////////////////////////
// TODO

class QSpinBox;

class QCustomResolutionDlg : public CEditorDialog
{
public:
	QCustomResolutionDlg(i32 width, i32 height);

	bool GetResult(i32& widthOut, i32& heightOut) const;

protected:
	void OnConfirmed();

	QSpinBox* m_pWidth;
	QSpinBox* m_pHeight;
	bool      m_bConfirmed;
};

// ~TODO
//////////////////////////////////////////////////////////////////////////

class QSpinBox;
class QCheckBox;
class QProgressBar;
class QTextEdit;
class QLineEdit;
class QListView;
class QLabel;
class QVBoxLayout;
class QTimer;
class QComboBox;

class QMenuComboBox;

class CTrackViewBatchRenderDlg
	: public CEditorDialog
	  , public IMovieListener
{
	typedef TRange<SAnimTime> AnimRange;

	struct SInputGroup
	{
		QMenuComboBox* pSequenceField;
		QMenuComboBox* pDirectorField;
		QSpinBox*      pCaptureStartField;
		QSpinBox*      pCaptureEndField;

		SInputGroup();
	};

	struct SOutputGroup
	{
		struct SCaptureOptionsGroup
		{
			QMenuComboBox* pFormatField;
			QMenuComboBox* pBufferField;
			QLineEdit*     pFilePrefixField;
			QCheckBox*     pCreateVideoField;

			SCaptureOptionsGroup();
		};

		struct SCustomConfigGroup
		{
			QTextEdit* pCVarsField;

			SCustomConfigGroup();
		};

		QMenuComboBox*       pResolutionField;
		// TODO: change to QMenuComboBox when the widget is editable
		QComboBox*           pFpsField;
		SCaptureOptionsGroup captureOptionGroup;
		SCustomConfigGroup   customConfigGroup;

		QLineEdit*           pFolderField;

		QPushButton*         pPickFolderButton;
		QPushButton*         pLoadPresetButton;
		QPushButton*         pSavePresetButton;

		SOutputGroup();
	};

	struct SBatchGroup
	{
		QListView*   pBatchesView;

		QPushButton* pAddButton;
		QPushButton* pRemoveButton;
		QPushButton* pClearButton;
		QPushButton* pUpdateButton;

		QPushButton* pLoadBatchButton;
		QPushButton* pSaveBatchButton;

		SBatchGroup();
	};

	struct SRenderItem
	{
		IAnimSequence*      pSequence;
		IAnimNode*          pDirectorNode;

		string              folder;
		string              prefix;
		std::vector<string> cvars;

		AnimRange           frameRange;
		i32               resW;
		i32               resH;
		i32               fps;
		i32               formatIndex;
		i32               bufferIndex;
		i32               frameStart;
		i32               frameEnd;

		bool                bCreateVideo;

		SRenderItem();
		bool operator==(const SRenderItem& item) const;
	};

	struct SRenderContext
	{
		IAnimNode*  pActiveDirectorBU;
		AnimRange   rangeBU;
		i32       currentItemIndex;
		i32       expectedTotalTicks;
		i32       spentTicks;
		i32       flagBU;
		i32       cvarCustomResWidthBU;
		i32       cvarCustomResHeightBU;
		SCaptureKey captureOptions;
		DWORD       timeWarmingUpStarted;
		bool        bFFMPEGProcessing;
		bool        bWarmingUpAfterResChange;

		SRenderContext();
		bool IsInRendering() const;
	};

public:
	CTrackViewBatchRenderDlg();
	~CTrackViewBatchRenderDlg();

private:
	void Initialize();

	void InitInputGroup(QVBoxLayout* pFormLayout);
	void InitOutputGroup(QVBoxLayout* pFormLayout);
	void InitBatchGroup(QVBoxLayout* pFormLayout);

	// Input Group
	void OnSequenceSelectionChanged(i32 index);
	void OnStartFrameChanged();
	void OnEndFrameChanged();

	// Output Group
	void OnResolutionSelectionChanged(i32 index);
	void OnFpsSelectionChanged(i32 index);
	void OnFpsTextChanged(const QString& text);
	void OnBufferSelectionChanged(i32 index);
	void OnFormatSelectionChanged(i32 index);

	void UpdateFrameRangeForCurrentSequence();
	void UpdateCreateVideoCheckBox(i32 state);

	void OnPickFolderButtonPressed();

	void OnSavePresetButtonPressed() const;
	void SaveOutputPreset(tukk szPathName) const;

	void OnLoadPresetButtonPressed();
	bool LoadOutputPreset(tukk szPathName);

	// Batch Group
	void OnBatchViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	void OnBatchAddButtonPressed();
	bool WriteDialogValuesToRenderItem(SRenderItem& itemOut);
	void AddRenderItem(const SRenderItem& item);

	void OnBatchRemoveButtonPressed();
	void OnBatchClearButtonPressed();
	void OnBatchUpdateButtonPressed();

	void OnLoadBatchButtonPressed();
	void OnSaveBatchButtonPressed();

	// Dialog info/actions
	void OnActionButtonPressed();
	void OnCancelButtonPressed();

	void InitializeRenderContext();
	void BegCaptureItem();
	void EndCaptureItem(IAnimSequence* pSequence);

	void Update();

	// IMovieListener
	virtual void OnMovieEvent(IMovieListener::EMovieEvent event, IAnimSequence* pSequence) override;
	// ~IMovieListener

	void          keyPressEvent(QKeyEvent* pEvent);

	u32        GetCurrentTicksPerFrame() const;
	void          SetCurrentSequence(IAnimSequence* pSequence);
	static string GenerateCaptureItemString(const SRenderItem& item);

	// UI
	QProgressBar* m_pProgressBar;
	QLabel*       m_pStatusMessage;
	QLabel*       m_pluginMessage;
	QPushButton*  m_pActionButton;
	QPushButton*  m_pCancelButton;

	QTimer*       m_pUpdateTimer;

	SInputGroup   m_inputGroup;
	SOutputGroup  m_outputGroup;
	SBatchGroup   m_batchGroup;

	// Capturing
	std::vector<SRenderItem> m_renderItems;
	SRenderContext           m_renderContext;

	AnimRange                m_currentAnimRange;
	IAnimSequence*           m_pCurrentSequence;

	i32                    m_customResW;
	i32                    m_customResH;
	i32                    m_customFPS;
	bool                     m_bIsLoadingPreset;

	SDisplayContextKey       m_displayContextKey;
	i32                    m_viewPortResW;
	i32                    m_viewPortResH;
};

inline CTrackViewBatchRenderDlg::SInputGroup::SInputGroup()
	: pSequenceField(nullptr)
	, pDirectorField(nullptr)
	, pCaptureStartField(nullptr)
	, pCaptureEndField(nullptr)
{}

inline CTrackViewBatchRenderDlg::SOutputGroup::SCaptureOptionsGroup::SCaptureOptionsGroup()
	: pFormatField(nullptr)
	, pBufferField(nullptr)
	, pFilePrefixField(nullptr)
	, pCreateVideoField(nullptr)
{}

inline CTrackViewBatchRenderDlg::SOutputGroup::SCustomConfigGroup::SCustomConfigGroup()
	: pCVarsField(nullptr)
{}

inline CTrackViewBatchRenderDlg::SOutputGroup::SOutputGroup()
	: pResolutionField(nullptr)
	, pFpsField(nullptr)
	, pFolderField(nullptr)
	, pPickFolderButton(nullptr)
	, pLoadPresetButton(nullptr)
	, pSavePresetButton(nullptr)
{}

inline CTrackViewBatchRenderDlg::SBatchGroup::SBatchGroup()
	: pBatchesView(nullptr)
	, pAddButton(nullptr)
	, pRemoveButton(nullptr)
	, pClearButton(nullptr)
	, pUpdateButton(nullptr)
	, pLoadBatchButton(nullptr)
	, pSaveBatchButton(nullptr)
{}

inline CTrackViewBatchRenderDlg::SRenderItem::SRenderItem()
	: pSequence(nullptr)
	, pDirectorNode(nullptr)
	, bCreateVideo(false)
{}

inline bool CTrackViewBatchRenderDlg::SRenderItem::operator==(const SRenderItem& item) const
{
	return (
	  pSequence == item.pSequence &&
	  pDirectorNode == item.pDirectorNode &&
	  frameRange == item.frameRange &&
	  resW == item.resW && resH == item.resH &&
	  fps == item.fps &&
	  frameStart == item.frameStart &&
	  frameEnd == item.frameEnd &&
	  formatIndex == item.formatIndex &&
	  bufferIndex == item.bufferIndex &&
	  folder == item.folder &&
	  prefix == item.prefix &&
	  cvars == item.cvars &&
	  bCreateVideo == item.bCreateVideo
	  );
}

inline CTrackViewBatchRenderDlg::SRenderContext::SRenderContext()
	: currentItemIndex(-1)
	, expectedTotalTicks(0)
	, spentTicks(0)
	, flagBU(0)
	, pActiveDirectorBU(nullptr)
	, cvarCustomResWidthBU(0)
	, cvarCustomResHeightBU(0)
	, bWarmingUpAfterResChange(false)
	, timeWarmingUpStarted(0)
	, bFFMPEGProcessing(false)
{}

inline bool CTrackViewBatchRenderDlg::SRenderContext::IsInRendering() const
{
	return currentItemIndex >= 0;
}

