// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "TrackViewBatchRenderDlg.h"

#include "Util/EditorUtils.h"
#include "Util/FileUtil.h"
#include "Controls/QMenuComboBox.h"

#pragma warning(push)
#pragma warning(disable:4355) // 'this': used in base member initializer list
#include <ppl.h>
#include <agents.h>
#pragma warning(pop)

#include <QFormLAyout>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QProgressBar>
#include <QCheckBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QTimer>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QListView>
#include <QPushButton>
#include <QEvent.h>

#include <drx3D/Sandbox/Editor/Plugin/ICommandManager.h>
#include "GameEngine.h"
#include "RenderViewport.h"
#include "Viewmanager.h"


Q_DECLARE_METATYPE(IAnimSequence*)

i32k maxRes = 8192;
i32k fixedResolutionOptions[][2] = {
	{ 1280, 720 }, { 1920, 1080 }, { 1998, 1080 }, { 2048, 858 }, { 2560, 1440 }
};
tukk szCustomResFormat = "Custom(%d x %d) ...";
tukk szCustomValueText = "Custom ...";

// 1000ms == 1s
i32k warmingUpDuration = 1000;

// This version number should be incremented
// every time available options like the list of formats,
// the list of buffers change.
i32k batchRenderFileVersion = 1;

struct SFpsDesc
{
	i32         fps;
	tukk szDesc;
};
SFpsDesc fixedFpsOptions[] = {
	{ 24, "Film(24)"       },
	{ 25, "PAL(25)"        },
	{ 30, "NTSC(30)"       },
	{ 48, "Show(48)"       },
	{ 50, "PAL Field(50)"  },
	{ 60, "NTSC Field(60)" }
};

tukk szDefaultPresetFilename = "defaultBatchRender.preset";

//////////////////////////////////////////////////////////////////////////
// TODO

#include <QDialogButtonBox>

#define MIN_RES 64
#define MAX_RES 8192

QCustomResolutionDlg::QCustomResolutionDlg(i32 width, i32 height)
	: CEditorDialog("Custom Resolution")
	, m_bConfirmed(false)
{
	QVBoxLayout* pDialogLayout = new QVBoxLayout(this);
	QFormLayout* pFormLayout = new QFormLayout();
	pDialogLayout->addLayout(pFormLayout);

	m_pWidth = new QSpinBox();
	m_pWidth->setRange(MIN_RES, MAX_RES);
	m_pWidth->setSingleStep(1);
	m_pWidth->setVal(width);
	pFormLayout->addRow(QObject::tr("&Width:"), m_pWidth);

	m_pHeight = new QSpinBox();
	m_pHeight->setRange(MIN_RES, MAX_RES);
	m_pHeight->setSingleStep(1);
	m_pHeight->setVal(height);
	pFormLayout->addRow(QObject::tr("&Height:"), m_pHeight);

	QDialogButtonBox* pButtonBox = new QDialogButtonBox(this);
	pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	pDialogLayout->addWidget(pButtonBox);
	QObject::connect(pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	QObject::connect(pButtonBox, &QDialogButtonBox::accepted, this, &QCustomResolutionDlg::OnConfirmed);
}

bool QCustomResolutionDlg::GetResult(i32& widthOut, i32& heightOut) const
{
	if (m_bConfirmed)
	{
		widthOut = m_pWidth->value();
		heightOut = m_pHeight->value();
		return true;
	}
	return false;
}

void QCustomResolutionDlg::OnConfirmed()
{
	m_bConfirmed = true;
	QDialog::accept();
}

// ~TODO
//////////////////////////////////////////////////////////////////////////

CTrackViewBatchRenderDlg::CTrackViewBatchRenderDlg()
	: CEditorDialog("Render Output")
	, m_pProgressBar(nullptr)
	, m_pStatusMessage(nullptr)
	, m_pluginMessage(nullptr)
	, m_pActionButton(nullptr)
	, m_pCancelButton(nullptr)
	, m_pUpdateTimer(nullptr)
	, m_pCurrentSequence(nullptr)
	, m_customResW(-1)
	, m_customResH(-1)
	, m_customFPS(-1)
	, m_bIsLoadingPreset(false)
{
	Initialize();
}

CTrackViewBatchRenderDlg::~CTrackViewBatchRenderDlg()
{

}

void CTrackViewBatchRenderDlg::Initialize()
{
	QVBoxLayout* pDialogLayout = new QVBoxLayout(this);

	InitInputGroup(pDialogLayout);
	InitOutputGroup(pDialogLayout);
	InitBatchGroup(pDialogLayout);

	m_pProgressBar = new QProgressBar();
	m_pProgressBar->setRange(0, 100);
	pDialogLayout->addWidget(m_pProgressBar);

	m_pStatusMessage = new QLabel("Not running");
	pDialogLayout->addWidget(m_pStatusMessage);

	QDialogButtonBox* pButtonBox = new QDialogButtonBox(this);

	m_pActionButton = new QPushButton("Start");
	m_pActionButton->setDisabled(true);
	pButtonBox->addButton(m_pActionButton, QDialogButtonBox::ActionRole);
	QObject::connect(m_pActionButton, &QPushButton::pressed, this, &CTrackViewBatchRenderDlg::OnActionButtonPressed);

	m_pCancelButton = new QPushButton("Done");
	pButtonBox->addButton(m_pCancelButton, QDialogButtonBox::ActionRole);
	QObject::connect(m_pCancelButton, &QPushButton::pressed, this, &CTrackViewBatchRenderDlg::OnCancelButtonPressed);

	pDialogLayout->addWidget(pButtonBox);

	this->setLayout(pDialogLayout);

	m_pUpdateTimer = new QTimer(this);
	QObject::connect(m_pUpdateTimer, &QTimer::timeout, this, &CTrackViewBatchRenderDlg::Update);
	m_pUpdateTimer->start(1000);

	m_inputGroup.pSequenceField->SetChecked(0);
	OnSequenceSelectionChanged(0);

	stack_string defaultFilePath = GetIEditor()->GetUserFolder();
	defaultFilePath.append(szDefaultPresetFilename);

	if (!CFileUtil::FileExists(defaultFilePath.c_str()))
	{
		SaveOutputPreset(defaultFilePath.c_str());
	}

	LoadOutputPreset(defaultFilePath.c_str());
}

void CTrackViewBatchRenderDlg::InitInputGroup(QVBoxLayout* pFormLayout)
{
	QGroupBox* pGroupBox = new QGroupBox(QObject::tr("Input"));
	pFormLayout->addWidget(pGroupBox);

	QFormLayout* pLeftLayout = new QFormLayout();
	QFormLayout* pRightLayout = new QFormLayout();

	QHBoxLayout* pHBoxLayout = new QHBoxLayout();
	pHBoxLayout->addLayout(pLeftLayout);
	pHBoxLayout->addLayout(pRightLayout);
	pGroupBox->setLayout(pHBoxLayout);

	// Sequence
	m_inputGroup.pSequenceField = new QMenuComboBox();
	m_inputGroup.pSequenceField->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_inputGroup.pSequenceField->SetMultiSelect(false);
	m_inputGroup.pSequenceField->SetCanHaveEmptySelection(false);
	pLeftLayout->addRow(QObject::tr("Sequence:"), m_inputGroup.pSequenceField);

	i32k numSequences = GetIEditor()->GetMovieSystem()->GetNumSequences();
	for (i32 seqIdx = 0; seqIdx < numSequences; ++seqIdx)
	{
		IAnimSequence* pSequence = GetIEditor()->GetMovieSystem()->GetSequence(seqIdx);
		m_inputGroup.pSequenceField->AddItem(pSequence->GetName(), QVariant::fromValue(pSequence));
	}

	m_inputGroup.pSequenceField->signalCurrentIndexChanged.Connect(this, &CTrackViewBatchRenderDlg::OnSequenceSelectionChanged);

	// Director
	m_inputGroup.pDirectorField = new QMenuComboBox();
	m_inputGroup.pDirectorField->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_inputGroup.pDirectorField->SetMultiSelect(false);
	m_inputGroup.pDirectorField->SetCanHaveEmptySelection(false);
	m_inputGroup.pDirectorField->setDisabled(true);
	pLeftLayout->addRow(QObject::tr("Director:"), m_inputGroup.pDirectorField);

	// StartFrame
	m_inputGroup.pCaptureStartField = new QSpinBox();
	m_inputGroup.pCaptureStartField->setRange(0, INT_MAX);
	m_inputGroup.pCaptureStartField->setSingleStep(1);
	m_inputGroup.pCaptureStartField->setVal(0);
	pRightLayout->addRow("Start Frame:", m_inputGroup.pCaptureStartField);

	QObject::connect(
	  m_inputGroup.pCaptureStartField, &QSpinBox::editingFinished,
	  this, &CTrackViewBatchRenderDlg::OnStartFrameChanged
	  );

	// EndFrame
	m_inputGroup.pCaptureEndField = new QSpinBox();
	m_inputGroup.pCaptureEndField->setRange(0, INT_MAX);
	m_inputGroup.pCaptureEndField->setSingleStep(1);
	m_inputGroup.pCaptureEndField->setVal(0);
	pRightLayout->addRow("End Frame:", m_inputGroup.pCaptureEndField);

	QObject::connect(
	  m_inputGroup.pCaptureEndField, &QSpinBox::editingFinished,
	  this, &CTrackViewBatchRenderDlg::OnEndFrameChanged
	  );
}

void CTrackViewBatchRenderDlg::InitOutputGroup(QVBoxLayout* pFormLayout)
{
	QGroupBox* pGroupBox = new QGroupBox(QObject::tr("Output"));
	pFormLayout->addWidget(pGroupBox);

	QVBoxLayout* pLayout = new QVBoxLayout();
	pGroupBox->setLayout(pLayout);

	// Resolution/FPS
	QHBoxLayout* pResAndFpsRow = new QHBoxLayout();
	pLayout->addLayout(pResAndFpsRow);

	m_outputGroup.pResolutionField = new QMenuComboBox();
	m_outputGroup.pResolutionField->SetMultiSelect(false);
	m_outputGroup.pResolutionField->SetCanHaveEmptySelection(false);
	for (i32k(&res)[2] : fixedResolutionOptions)
	{
		QString resText = QObject::tr("%0 x %1").arg(res[0]).arg(res[1]);
		m_outputGroup.pResolutionField->AddItem(resText);
	}
	m_outputGroup.pResolutionField->AddItem(szCustomValueText);
	m_outputGroup.pResolutionField->signalCurrentIndexChanged.Connect(this, &CTrackViewBatchRenderDlg::OnResolutionSelectionChanged);

	pResAndFpsRow->addWidget(new QLabel(QObject::tr("Resolution:")));
	pResAndFpsRow->addWidget(m_outputGroup.pResolutionField);

	m_outputGroup.pFpsField = new QComboBox();
	for (const SFpsDesc& fpsOption : fixedFpsOptions)
	{
		m_outputGroup.pFpsField->addItem(QObject::tr(fpsOption.szDesc));
		m_outputGroup.pFpsField->setFixedWidth(125);
	}
	m_outputGroup.pFpsField->addItem(szCustomValueText);

	QObject::connect(
	  m_outputGroup.pFpsField,
	  static_cast<void (QComboBox::*)(i32)>(&QComboBox::currentIndexChanged),
	  this,
	  &CTrackViewBatchRenderDlg::OnFpsSelectionChanged
	  );
	QObject::connect(m_outputGroup.pFpsField, &QComboBox::editTextChanged, this, &CTrackViewBatchRenderDlg::OnFpsTextChanged);

	pResAndFpsRow->addWidget(new QLabel(QObject::tr("FPS:")));
	pResAndFpsRow->addWidget(m_outputGroup.pFpsField);

	// Capture Options
	QGroupBox* pCaptureOptions = new QGroupBox("Capture Options");
	pLayout->addWidget(pCaptureOptions);

	QFormLayout* pCaptureOptionsLayout = new QFormLayout();
	pCaptureOptions->setLayout(pCaptureOptionsLayout);

	m_outputGroup.captureOptionGroup.pFormatField = new QMenuComboBox();
	m_outputGroup.captureOptionGroup.pFormatField->SetMultiSelect(false);
	m_outputGroup.captureOptionGroup.pFormatField->SetCanHaveEmptySelection(false);
	for (i32 i = 0; i < SCaptureFormatInfo::eCaptureFormat_Num; ++i)
	{
		const SCaptureFormatInfo::ECaptureFileFormat captureFormat = static_cast<SCaptureFormatInfo::ECaptureFileFormat>(i);
		m_outputGroup.captureOptionGroup.pFormatField->AddItem(QString(SCaptureFormatInfo::GetCaptureFormatExtension(captureFormat)));
	}

	pCaptureOptionsLayout->addRow("Format:", m_outputGroup.captureOptionGroup.pFormatField);
	m_outputGroup.captureOptionGroup.pFormatField->signalCurrentIndexChanged.Connect(this, &CTrackViewBatchRenderDlg::OnFormatSelectionChanged);

	m_outputGroup.captureOptionGroup.pBufferField = new QMenuComboBox();
	m_outputGroup.captureOptionGroup.pBufferField->SetMultiSelect(false);
	m_outputGroup.captureOptionGroup.pBufferField->SetCanHaveEmptySelection(false);
	for (i32 i = 0; i < SCaptureFormatInfo::eCaptureBuffer_Num; ++i)
	{
		const SCaptureFormatInfo::ECaptureBuffer captureBuffer = static_cast<SCaptureFormatInfo::ECaptureBuffer>(i);
		m_outputGroup.captureOptionGroup.pBufferField->AddItem(QString(SCaptureFormatInfo::GetCaptureBufferName(captureBuffer)));
	}

	pCaptureOptionsLayout->addRow("Buffer(s) to capture:", m_outputGroup.captureOptionGroup.pBufferField);
	m_outputGroup.captureOptionGroup.pBufferField->signalCurrentIndexChanged.Connect(this, &CTrackViewBatchRenderDlg::OnBufferSelectionChanged);

	m_outputGroup.captureOptionGroup.pFilePrefixField = new QLineEdit();
	pCaptureOptionsLayout->addRow("File prefix:", m_outputGroup.captureOptionGroup.pFilePrefixField);

	if (GetIEditor()->GetICommandManager()->IsRegistered("plugin", "ffmpeg_encode"))
	{
		m_outputGroup.captureOptionGroup.pCreateVideoField = new QCheckBox("Create video (mpeg4/.mp4)");
		pCaptureOptionsLayout->addRow(m_outputGroup.captureOptionGroup.pCreateVideoField);
		QObject::connect(
		  m_outputGroup.captureOptionGroup.pCreateVideoField,
		  static_cast<void (QCheckBox::*)(i32)>(&QCheckBox::stateChanged),
		  this,
		  &CTrackViewBatchRenderDlg::UpdateCreateVideoCheckBox
		  );
	}
	/*else
	   {
	   pCaptureOptionsLayout->addRow(new QLabel("FFMPEG plug-in isn't found (creating a video isn't supported)!"));
	   }*/

	// Custom Configs
	QGroupBox* pCustomConfigs = new QGroupBox("Custom Configs");
	pLayout->addWidget(pCustomConfigs);

	QVBoxLayout* pCustomConfigLayout = new QVBoxLayout();
	pCustomConfigs->setLayout(pCustomConfigLayout);

	pCustomConfigLayout->addWidget(new QLabel("Input cvars for further customization:"));

	m_outputGroup.customConfigGroup.pCVarsField = new QTextEdit();
	m_outputGroup.customConfigGroup.pCVarsField->setFixedHeight(64);
	m_outputGroup.customConfigGroup.pCVarsField->setObjectName("CVarsField");
	m_outputGroup.customConfigGroup.pCVarsField->setAccessibleName("CVarsField");

	pCustomConfigLayout->addWidget(m_outputGroup.customConfigGroup.pCVarsField);

	// Destination
	QHBoxLayout* pDestinationRow = new QHBoxLayout();
	pLayout->addLayout(pDestinationRow);

	pDestinationRow->addWidget(new QLabel("Destination Folder:"));

	m_outputGroup.pFolderField = new QLineEdit();
	pDestinationRow->addWidget(m_outputGroup.pFolderField);

	m_outputGroup.pPickFolderButton = new QPushButton("Pick Folder");
	pDestinationRow->addWidget(m_outputGroup.pPickFolderButton);
	QObject::connect(m_outputGroup.pPickFolderButton, &QPushButton::pressed, this, &CTrackViewBatchRenderDlg::OnPickFolderButtonPressed);

	// Buttons
	QHBoxLayout* pButtonRow = new QHBoxLayout();
	pLayout->addLayout(pButtonRow);
	m_outputGroup.pLoadPresetButton = new QPushButton("Load Preset");
	pButtonRow->addWidget(m_outputGroup.pLoadPresetButton);
	QObject::connect(m_outputGroup.pLoadPresetButton, &QPushButton::pressed, this, &CTrackViewBatchRenderDlg::OnLoadPresetButtonPressed);

	m_outputGroup.pSavePresetButton = new QPushButton("Save Preset");
	pButtonRow->addWidget(m_outputGroup.pSavePresetButton);
	QObject::connect(m_outputGroup.pSavePresetButton, &QPushButton::pressed, this, &CTrackViewBatchRenderDlg::OnSavePresetButtonPressed);
}

void CTrackViewBatchRenderDlg::InitBatchGroup(QVBoxLayout* pFormLayout)
{
	QGroupBox* pGroupBox = new QGroupBox("Batch");
	pFormLayout->addWidget(pGroupBox);

	QVBoxLayout* pButtonLayout = new QVBoxLayout();
	QHBoxLayout* pHBoxLayout = new QHBoxLayout();
	pGroupBox->setLayout(pHBoxLayout);

	m_batchGroup.pBatchesView = new QListView(this);
	m_batchGroup.pBatchesView->setModel(new QStandardItemModel());
	pHBoxLayout->addWidget(m_batchGroup.pBatchesView);
	QObject::connect(
	  m_batchGroup.pBatchesView->selectionModel(),
	  &QItemSelectionModel::selectionChanged,
	  this,
	  &CTrackViewBatchRenderDlg::OnBatchViewSelectionChanged
	  );

	pHBoxLayout->addLayout(pButtonLayout);

	m_batchGroup.pAddButton = new QPushButton("Add");
	pButtonLayout->addWidget(m_batchGroup.pAddButton);
	QObject::connect(m_batchGroup.pAddButton, &QPushButton::pressed, this, &CTrackViewBatchRenderDlg::OnBatchAddButtonPressed);

	m_batchGroup.pRemoveButton = new QPushButton("Remove");
	m_batchGroup.pRemoveButton->setDisabled(true);
	pButtonLayout->addWidget(m_batchGroup.pRemoveButton);
	QObject::connect(m_batchGroup.pRemoveButton, &QPushButton::pressed, this, &CTrackViewBatchRenderDlg::OnBatchRemoveButtonPressed);

	m_batchGroup.pClearButton = new QPushButton("Clear");
	pButtonLayout->addWidget(m_batchGroup.pClearButton);
	QObject::connect(m_batchGroup.pClearButton, &QPushButton::pressed, this, &CTrackViewBatchRenderDlg::OnBatchClearButtonPressed);

	m_batchGroup.pUpdateButton = new QPushButton("Update");
	m_batchGroup.pUpdateButton->setDisabled(true);
	pButtonLayout->addWidget(m_batchGroup.pUpdateButton);
	QObject::connect(m_batchGroup.pUpdateButton, &QPushButton::pressed, this, &CTrackViewBatchRenderDlg::OnBatchUpdateButtonPressed);

	m_batchGroup.pLoadBatchButton = new QPushButton("Load Batch");
	pButtonLayout->addWidget(m_batchGroup.pLoadBatchButton);
	QObject::connect(m_batchGroup.pLoadBatchButton, &QPushButton::pressed, this, &CTrackViewBatchRenderDlg::OnLoadBatchButtonPressed);

	m_batchGroup.pSaveBatchButton = new QPushButton("Save Batch");
	pButtonLayout->addWidget(m_batchGroup.pSaveBatchButton);
	QObject::connect(m_batchGroup.pSaveBatchButton, &QPushButton::pressed, this, &CTrackViewBatchRenderDlg::OnSaveBatchButtonPressed);
}

void CTrackViewBatchRenderDlg::OnSequenceSelectionChanged(i32 index)
{
	if (IAnimSequence* pSequence = m_inputGroup.pSequenceField->GetData(index).value<IAnimSequence*>())
	{
		SetCurrentSequence(pSequence);

		m_inputGroup.pDirectorField->Clear();
		for (i32 i = 0, e = pSequence->GetNodeCount(); i < e; ++i)
		{
			if (pSequence->GetNode(i)->GetType() == eAnimNodeType_Director)
			{
				m_inputGroup.pDirectorField->AddItem(QObject::tr(pSequence->GetNode(i)->GetName()));
			}
		}
		m_inputGroup.pDirectorField->SetChecked(0);
		m_inputGroup.pDirectorField->setEnabled(true);
	}
	else
	{
		m_inputGroup.pDirectorField->setDisabled(true);
	}
}

void CTrackViewBatchRenderDlg::SetCurrentSequence(IAnimSequence* pSequence)
{
	m_currentAnimRange.start = pSequence->GetTimeRange().start;
	m_currentAnimRange.end = pSequence->GetTimeRange().end;
	m_pCurrentSequence = pSequence;
	UpdateFrameRangeForCurrentSequence();
}

void CTrackViewBatchRenderDlg::OnStartFrameChanged()
{
	i32 newValue = m_inputGroup.pCaptureStartField->value();
	if (newValue >= m_inputGroup.pCaptureEndField->value())
	{
		m_inputGroup.pCaptureEndField->setVal(newValue);
	}

	if (m_pCurrentSequence)
	{
		u32k currentTicksPerFrame = GetCurrentTicksPerFrame();
		m_currentAnimRange.start = SAnimTime(m_inputGroup.pCaptureStartField->value() * (i32)currentTicksPerFrame);
		m_currentAnimRange.end = SAnimTime(m_inputGroup.pCaptureEndField->value() * (i32)currentTicksPerFrame);
		UpdateFrameRangeForCurrentSequence();
	}
}

void CTrackViewBatchRenderDlg::OnEndFrameChanged()
{
	i32 newValue = m_inputGroup.pCaptureEndField->value();
	if (newValue <= m_inputGroup.pCaptureStartField->value())
	{
		m_inputGroup.pCaptureStartField->setVal(newValue);
	}

	if (m_pCurrentSequence)
	{
		u32k currentTicksPerFrame = GetCurrentTicksPerFrame();
		m_currentAnimRange.start = SAnimTime(m_inputGroup.pCaptureStartField->value() * (i32)currentTicksPerFrame);
		m_currentAnimRange.end = SAnimTime(m_inputGroup.pCaptureEndField->value() * (i32)currentTicksPerFrame);
		UpdateFrameRangeForCurrentSequence();
	}
}

void CTrackViewBatchRenderDlg::OnResolutionSelectionChanged(i32 index)
{
	if (m_bIsLoadingPreset)
		return;

	i32k customResIdx = arraysize(fixedResolutionOptions);
	if (index == customResIdx)
	{
		i32 w = fixedResolutionOptions[0][0];
		i32 h = fixedResolutionOptions[0][1];

		QByteArray byteArray = m_outputGroup.pResolutionField->GetText().toUtf8();
		sscanf(byteArray.data(), szCustomResFormat, &w, &h);

		QCustomResolutionDlg resDlg(w, h);
		resDlg.exec();
		if (resDlg.GetResult(w, h))
		{
			m_customResW = min(w, maxRes);
			m_customResH = min(h, maxRes);

			DrxStackStringT<char, 32> resText;
			resText.Format(szCustomResFormat, m_customResW, m_customResH);

			m_outputGroup.pResolutionField->SetItemText(customResIdx, QObject::tr(resText));
		}
		else
		{
			m_outputGroup.pResolutionField->SetChecked(0);
		}
	}
}

void CTrackViewBatchRenderDlg::OnFpsSelectionChanged(i32 index)
{
	if (index == arraysize(fixedFpsOptions))
	{
		m_outputGroup.pFpsField->setEditable(true);
		m_outputGroup.pFpsField->setItemText(index, "");
	}
	else
	{
		m_outputGroup.pFpsField->setEditable(false);
		m_outputGroup.pFpsField->setItemText(arraysize(fixedFpsOptions), szCustomValueText);
	}
	UpdateFrameRangeForCurrentSequence();
}

void CTrackViewBatchRenderDlg::OnFpsTextChanged(const QString& text)
{
	i32 fps;
	if (sscanf(text.toUtf8().data(), "%d", &fps) == 1 && fps > 0)
	{
		m_customFPS = fps;
	}
}

void CTrackViewBatchRenderDlg::OnBufferSelectionChanged(i32 index)
{
	i32 selectedBufferIndex = m_outputGroup.captureOptionGroup.pBufferField->GetCheckedItem();
	switch (selectedBufferIndex)
	{
	case SCaptureFormatInfo::eCaptureBuffer_Color:
		{
			i32 formatCount = m_outputGroup.captureOptionGroup.pFormatField->GetItemCount();
			if (formatCount > SCaptureFormatInfo::eCaptureFormat_Num)
			{
				m_outputGroup.captureOptionGroup.pFormatField->SetChecked(0);
				do
				{
					m_outputGroup.captureOptionGroup.pFormatField->RemoveItem(--formatCount);
				}
				while (formatCount > SCaptureFormatInfo::eCaptureFormat_Num);
			}

			m_outputGroup.captureOptionGroup.pFormatField->setEnabled(true);
		}
		break;
	}

	if (m_outputGroup.captureOptionGroup.pCreateVideoField)
	{
		UpdateCreateVideoCheckBox(m_outputGroup.captureOptionGroup.pCreateVideoField->checkState());
	}
}

void CTrackViewBatchRenderDlg::OnFormatSelectionChanged(i32 index)
{
	if (m_outputGroup.captureOptionGroup.pCreateVideoField)
	{
		UpdateCreateVideoCheckBox(m_outputGroup.captureOptionGroup.pCreateVideoField->checkState());
	}
}

void CTrackViewBatchRenderDlg::UpdateFrameRangeForCurrentSequence()
{
	if (m_pCurrentSequence)
	{
		u32k ticksPerFrame = GetCurrentTicksPerFrame();
		if (ticksPerFrame > 0)
		{
			u32k captureLimitStart = m_pCurrentSequence->GetTimeRange().start.GetTicks() / ticksPerFrame;
			u32k captureLimitEnd = m_pCurrentSequence->GetTimeRange().end.GetTicks() / ticksPerFrame;
			m_inputGroup.pCaptureStartField->setRange(captureLimitStart, captureLimitEnd);
			m_inputGroup.pCaptureStartField->setVal(m_currentAnimRange.start.GetTicks() / ticksPerFrame);
			m_inputGroup.pCaptureEndField->setRange(captureLimitStart, captureLimitEnd);
			m_inputGroup.pCaptureEndField->setVal(m_currentAnimRange.end.GetTicks() / ticksPerFrame);
		}
	}
}

void CTrackViewBatchRenderDlg::UpdateCreateVideoCheckBox(i32 state)
{
	const bool bColorBufferSelected = (m_outputGroup.captureOptionGroup.pBufferField->GetCheckedItem() == SCaptureFormatInfo::eCaptureBuffer_Color);
	const bool bJPGFormatSelected = (m_outputGroup.captureOptionGroup.pFormatField->GetCheckedItem() == SCaptureFormatInfo::eCaptureFormat_JPEG);

	if (bColorBufferSelected && bJPGFormatSelected)
	{
		m_outputGroup.captureOptionGroup.pCreateVideoField->setEnabled(true);
	}
	else
	{
		m_outputGroup.captureOptionGroup.pCreateVideoField->setEnabled(state);
	}
}

void CTrackViewBatchRenderDlg::OnPickFolderButtonPressed()
{
	QFileDialog fileDialog(this);
	fileDialog.setDirectory(m_outputGroup.pFolderField->text());
	fileDialog.setFileMode(QFileDialog::DirectoryOnly);

	if (fileDialog.exec())
	{
		m_outputGroup.pFolderField->setText(fileDialog.directory().absolutePath());
	}
}

void CTrackViewBatchRenderDlg::OnSavePresetButtonPressed() const
{
	string savePath;
	if (CFileUtil::SelectSaveFile("Preset Files (*.preset)|*.preset||", "preset", GetIEditor()->GetUserFolder(), savePath))
	{
		SaveOutputPreset(savePath.GetString());
	}
}

void CTrackViewBatchRenderDlg::SaveOutputPreset(tukk szPathName) const
{
	XmlNodeRef batchRenderOptionsNode = XmlHelpers::CreateXmlNode("batchrenderoptions");
	batchRenderOptionsNode->setAttr("version", batchRenderFileVersion);

	// Resolution
	i32k selectedResIdx = m_outputGroup.pResolutionField->GetCheckedItem();

	XmlNodeRef resolutionNode = batchRenderOptionsNode->newChild("resolution");
	resolutionNode->setAttr("cursel", selectedResIdx);
	if (selectedResIdx == arraysize(fixedResolutionOptions))
	{
		resolutionNode->setContent(m_outputGroup.pResolutionField->GetItem(selectedResIdx).toUtf8().data());
	}

	// FPS
	i32k selectedFpsIdx = m_outputGroup.pFpsField->currentIndex();

	XmlNodeRef fpsNode = batchRenderOptionsNode->newChild("fps");
	fpsNode->setAttr("cursel", selectedFpsIdx);
	if (selectedFpsIdx == arraysize(fixedFpsOptions))
	{
		fpsNode->setContent(m_outputGroup.pFpsField->currentText().toUtf8().data());
	}

	// Capture options (format, buffer, prefix, create_video)
	XmlNodeRef imageNode = batchRenderOptionsNode->newChild("image");
	imageNode->setAttr("format", m_outputGroup.captureOptionGroup.pFormatField->GetCheckedItem() % SCaptureFormatInfo::eCaptureFormat_Num);
	imageNode->setAttr("bufferstocapture", m_outputGroup.captureOptionGroup.pBufferField->GetCheckedItem());
	imageNode->setAttr("prefix", m_outputGroup.captureOptionGroup.pFilePrefixField->text().toUtf8().data());

	if (m_outputGroup.captureOptionGroup.pCreateVideoField)
	{
		imageNode->setAttr("createvideo", m_outputGroup.captureOptionGroup.pCreateVideoField->checkState() == Qt::Checked);
	}

	// Custom configs
	XmlNodeRef cvarsNode = batchRenderOptionsNode->newChild("cvars");

	const QStringList cvars = m_outputGroup.customConfigGroup.pCVarsField->toPlainText().split("\n", QString::SkipEmptyParts);
	for (const QString& cvar : cvars)
	{
		cvarsNode->newChild("cvar")->setContent(cvar.toUtf8().data());
	}

	// Destination
	XmlNodeRef destinationNode = batchRenderOptionsNode->newChild("destination");
	destinationNode->setContent(m_outputGroup.pFolderField->text().toUtf8().data());

	XmlHelpers::SaveXmlNode(batchRenderOptionsNode, szPathName);
}

void CTrackViewBatchRenderDlg::OnLoadPresetButtonPressed()
{
	string loadPath;
	if (CFileUtil::SelectFile("Preset Files (*.preset)|*.preset||", GetIEditor()->GetUserFolder(), loadPath))
	{
		if (LoadOutputPreset(loadPath.GetString()) == false)
		{
			//MessageBox("The file version is different!", "Cannot load", MB_OK|MB_ICONERROR);
		}
	}
}

bool CTrackViewBatchRenderDlg::LoadOutputPreset(tukk szPathName)
{
	m_bIsLoadingPreset = true;

	if (!CFileUtil::FileExists(szPathName))
	{
		return false;
	}

	XmlNodeRef batchRenderOptionsNode = XmlHelpers::LoadXmlFromFile(szPathName);
	if (batchRenderOptionsNode == nullptr)
	{
		return false;
	}

	i32 version = 0;
	batchRenderOptionsNode->getAttr("version", version);
	if (version != batchRenderFileVersion)
	{
		return false;
	}

	// Resolution
	if (XmlNodeRef resolutionNode = batchRenderOptionsNode->findChild("resolution"))
	{
		i32 selectedResIdx = -1;
		resolutionNode->getAttr("cursel", selectedResIdx);

		if (selectedResIdx == arraysize(fixedResolutionOptions))
		{
			tukk szCustomResText = resolutionNode->getContent();
			sscanf(szCustomResText, szCustomResText, &m_customResW, &m_customResH);

			m_outputGroup.pResolutionField->RemoveItem(selectedResIdx);
			m_outputGroup.pResolutionField->AddItem(QObject::tr(szCustomResText));
		}
		m_outputGroup.pResolutionField->SetChecked(selectedResIdx);
	}

	// FPS
	if (XmlNodeRef fpsNode = batchRenderOptionsNode->findChild("fps"))
	{
		i32 selectedFpsIdx = -1;
		fpsNode->getAttr("cursel", selectedFpsIdx);

		if (selectedFpsIdx == -1)
		{
			tukk szCustomFps = fpsNode->getContent();
			sscanf(szCustomFps, "%d", &m_customFPS);

			m_outputGroup.pFpsField->setCurrentIndex(-1);
			m_outputGroup.pFpsField->setCurrentText(QObject::tr(szCustomFps));
		}
		else
		{
			m_outputGroup.pFpsField->setCurrentIndex(selectedFpsIdx);
		}
	}

	// Capture options (format, buffer, prefix, create_video)
	if (XmlNodeRef imageNode = batchRenderOptionsNode->findChild("image"))
	{
		i32 selectedFormatIdx = -1;
		imageNode->getAttr("format", selectedFormatIdx);
		m_outputGroup.captureOptionGroup.pFormatField->SetChecked(selectedFormatIdx);

		i32 selectedBufferIdx = -1;
		imageNode->getAttr("bufferstocapture", selectedBufferIdx);
		m_outputGroup.captureOptionGroup.pBufferField->SetChecked(selectedBufferIdx);

		tukk szFilePrefix = imageNode->getAttr("prefix");
		m_outputGroup.captureOptionGroup.pFilePrefixField->setText(QObject::tr(szFilePrefix));

		if (m_outputGroup.captureOptionGroup.pCreateVideoField)
		{
			bool bCreateVideo = false;
			imageNode->getAttr("createvideo", bCreateVideo);
			m_outputGroup.captureOptionGroup.pCreateVideoField->setCheckState(bCreateVideo ? Qt::Checked : Qt::Unchecked);
		}
	}

	// Custom configs
	if (XmlNodeRef cvarsNode = batchRenderOptionsNode->findChild("cvars"))
	{
		QString cvarsText;
		for (i32 i = 0, e = cvarsNode->getChildCount(); i < e; ++i)
		{
			cvarsText += cvarsNode->getChild(i)->getContent();
			if (i < cvarsNode->getChildCount() - 1)
				cvarsText += "\n";
		}
		m_outputGroup.customConfigGroup.pCVarsField->setPlainText(cvarsText);
	}

	// Destination
	if (XmlNodeRef destinationNode = batchRenderOptionsNode->findChild("destination"))
	{
		tukk szFolder = destinationNode->getContent();
		m_outputGroup.pFolderField->setText(szFolder);
	}

	m_bIsLoadingPreset = false;

	return true;
}

void CTrackViewBatchRenderDlg::OnBatchViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	if (selected.indexes().count() == 1)
	{
		i32 index = selected.indexes().at(0).row();
		m_batchGroup.pUpdateButton->setDisabled(true);

		if (index == -1)
		{
			m_batchGroup.pRemoveButton->setDisabled(true);
			return;
		}

		m_batchGroup.pRemoveButton->setEnabled(true);

		const SRenderItem& item = m_renderItems[index];

		// Input->Sequence
		QString sequenceName(tr(item.pSequence->GetName()));
		for (i32 i = 0; i < m_inputGroup.pSequenceField->GetItemCount(); ++i)
		{
			if (sequenceName == m_inputGroup.pSequenceField->GetItem(i))
			{
				m_inputGroup.pSequenceField->SetChecked(i);
				break;
			}
		}

		// Input->Director
		QString directorName(tr(item.pDirectorNode->GetName()));
		for (i32 i = 0; i < m_inputGroup.pDirectorField->GetItemCount(); ++i)
		{
			if (directorName == m_inputGroup.pDirectorField->GetItem(i))
			{
				m_inputGroup.pDirectorField->SetChecked(i);
				break;
			}
		}

		// Input->Frame start/end
		m_inputGroup.pCaptureStartField->setVal(item.frameStart);
		m_inputGroup.pCaptureEndField->setVal(item.frameEnd);

		// Output->Destination folder
		m_outputGroup.pFolderField->setText(tr(item.folder.c_str()));

		// Output->FPS
		i32 selectedFpsOptionIdx = 0;
		for (; selectedFpsOptionIdx < arraysize(fixedFpsOptions); ++selectedFpsOptionIdx)
		{
			if (item.fps == fixedFpsOptions[selectedFpsOptionIdx].fps)
			{
				m_outputGroup.pFpsField->setCurrentIndex(selectedFpsOptionIdx);
				break;
			}
		}

		if (selectedFpsOptionIdx == arraysize(fixedFpsOptions))
		{
			m_customFPS = item.fps;
			DrxStackStringT<char, 8> fpsText;
			fpsText.Format("%d", item.fps);

			m_outputGroup.pFpsField->setCurrentIndex(-1);
			m_outputGroup.pFpsField->setCurrentText(QObject::tr(fpsText.c_str()));
		}

		// Output->CaptureOptions->Buffers
		m_outputGroup.captureOptionGroup.pBufferField->SetChecked(item.bufferIndex);

		// Output->CreateVideo
		if (m_outputGroup.captureOptionGroup.pCreateVideoField)
		{
			m_outputGroup.captureOptionGroup.pCreateVideoField->setCheckState(item.bCreateVideo ? Qt::Checked : Qt::Unchecked);
		}

		// Output->Resolution
		i32 selectedResolutionIdx = 0;
		for (; selectedResolutionIdx < arraysize(fixedResolutionOptions); ++selectedResolutionIdx)
		{
			i32k(&res)[2] = fixedResolutionOptions[selectedResolutionIdx];

			if (item.resW == res[0] && item.resH == res[1])
			{
				m_outputGroup.pResolutionField->SetChecked(selectedResolutionIdx);
				break;
			}
		}
		if (selectedResolutionIdx == arraysize(fixedResolutionOptions))
		{
			DrxStackStringT<char, 16> resText;
			resText.Format(szCustomResFormat, item.resW, item.resH);

			m_customResW = item.resW;
			m_customResH = item.resH;

			m_outputGroup.pResolutionField->RemoveItem(selectedResolutionIdx);
			m_outputGroup.pResolutionField->AddItem(QObject::tr(resText));
			m_outputGroup.pResolutionField->SetChecked(selectedResolutionIdx);
		}

		// Output->CVars
		stack_string cvarsText;
		for (const string& cvar : item.cvars)
		{
			cvarsText += cvar + "\n";
		}
		m_outputGroup.customConfigGroup.pCVarsField->setText(tr(cvarsText.c_str()));
	}
}

void CTrackViewBatchRenderDlg::OnBatchAddButtonPressed()
{
	// If there is no director node, it cannot be added.
	if (m_inputGroup.pDirectorField->GetCheckedItem() == -1)
	{
		//MessageBox("No director available!", "Cannot add", MB_OK|MB_ICONERROR);
		return;
	}

	/// Set up a new render item.
	SRenderItem item;
	if (WriteDialogValuesToRenderItem(item))
	{
		/// Check a duplication before adding.
		for (const SRenderItem& renderItem : m_renderItems)
		{
			if (renderItem == item)
			{
				//MessageBox("The same item already exists!", "Cannot add", MB_OK|MB_ICONERROR);
				return;
			}
		}

		AddRenderItem(item);
	}
}

bool CTrackViewBatchRenderDlg::WriteDialogValuesToRenderItem(SRenderItem& itemOut)
{
	QString sequenceName = m_inputGroup.pSequenceField->GetText();
	QString directorName = m_inputGroup.pDirectorField->GetText();

	// Output->Folder
	itemOut.folder = m_outputGroup.pFolderField->text().toUtf8().data();
	if (itemOut.folder.empty())
	{
		//MessageBox("The output folder should be specified!", "Cannot add", MB_OK|MB_ICONERROR);
		return false;
	}

	// Input->Sequence
	itemOut.pSequence = GetIEditor()->GetMovieSystem()->FindSequence(sequenceName.toUtf8().data());
	DRX_ASSERT(itemOut.pSequence);
	if (!itemOut.pSequence)
	{
		return false;
	}

	// Input->Director
	for (i32 i = 0, e = itemOut.pSequence->GetNodeCount(); i < e; ++i)
	{
		IAnimNode* pNode = itemOut.pSequence->GetNode(i);
		if (pNode->GetType() == eAnimNodeType_Director && directorName == pNode->GetName())
		{
			itemOut.pDirectorNode = pNode;
			break;
		}
	}

	if (itemOut.pDirectorNode == nullptr)
	{
		return false;
	}

	itemOut.frameStart = m_inputGroup.pCaptureStartField->value();
	itemOut.frameEnd = m_inputGroup.pCaptureEndField->value();

	// Output->FPS
	i32k fpsOptionIdx = m_outputGroup.pFpsField->currentIndex();
	if (fpsOptionIdx == arraysize(fixedFpsOptions))
	{
		itemOut.fps = m_customFPS;
	}
	else
	{
		itemOut.fps = fixedFpsOptions[fpsOptionIdx].fps;
	}

	// Input->Frame range
	u32k ticksPerFrame = GetCurrentTicksPerFrame();
	itemOut.frameRange.start = SAnimTime(itemOut.frameStart * (i32)ticksPerFrame);
	itemOut.frameRange.end = SAnimTime(itemOut.frameEnd * (i32)ticksPerFrame);

	// Output->Capture Options->Buffer
	itemOut.bufferIndex = m_outputGroup.captureOptionGroup.pBufferField->GetCheckedItem();

	// Output->Capture Options->Prefix
	itemOut.prefix = m_outputGroup.captureOptionGroup.pFilePrefixField->text().toUtf8().data();

	// Output->Capture Options->Format
	itemOut.formatIndex = m_outputGroup.captureOptionGroup.pFormatField->GetCheckedItem() % SCaptureFormatInfo::eCaptureFormat_Num;

	// Output->Capture Options->Create Video
	if (m_outputGroup.captureOptionGroup.pCreateVideoField)
	{
		itemOut.bCreateVideo = m_outputGroup.captureOptionGroup.pCreateVideoField->checkState() == Qt::Checked;
	}

	// Output->Resolution
	i32k selectedResIdx = m_outputGroup.pResolutionField->GetCheckedItem();
	if (selectedResIdx < arraysize(fixedResolutionOptions))
	{
		itemOut.resW = fixedResolutionOptions[selectedResIdx][0];
		itemOut.resH = fixedResolutionOptions[selectedResIdx][1];
	}
	else
	{
		itemOut.resW = m_customResW;
		itemOut.resH = m_customResH;
	}

	// Output->CustomConfig->CVars
	const QStringList cvars = m_outputGroup.customConfigGroup.pCVarsField->toPlainText().split("\n", QString::SkipEmptyParts);
	for (const QString& cvar : cvars)
	{
		itemOut.cvars.emplace_back(string(cvar.toUtf8().data()));
	}

	return true;
}

void CTrackViewBatchRenderDlg::AddRenderItem(const SRenderItem& item)
{
	m_renderItems.emplace_back(item);
	QStandardItemModel* pModel = static_cast<QStandardItemModel*>(m_batchGroup.pBatchesView->model());
	pModel->appendRow(new QStandardItem(GenerateCaptureItemString(item).c_str()));
	m_pActionButton->setEnabled(true);
}

void CTrackViewBatchRenderDlg::OnBatchRemoveButtonPressed()
{
	QAbstractItemModel* pModel = m_batchGroup.pBatchesView->model();
	QModelIndex index = m_batchGroup.pBatchesView->currentIndex();
	if (index.isValid())
	{
		pModel->removeRow(index.row(), index.parent());
		m_renderItems.erase(m_renderItems.begin() + index.row());

		if (m_renderItems.empty())
		{
			m_batchGroup.pRemoveButton->setDisabled(true);
			m_batchGroup.pUpdateButton->setDisabled(true);
			m_pActionButton->setDisabled(true);
		}
		else
		{
			m_batchGroup.pBatchesView->setCurrentIndex(pModel->index(0, 0));
		}
	}
}

void CTrackViewBatchRenderDlg::OnBatchClearButtonPressed()
{
	QAbstractItemModel* pModel = m_batchGroup.pBatchesView->model();
	pModel->removeRows(0, pModel->rowCount());
	m_renderItems.clear();

	m_batchGroup.pRemoveButton->setDisabled(true);
	m_batchGroup.pUpdateButton->setDisabled(true);
	m_pActionButton->setDisabled(true);
}

void CTrackViewBatchRenderDlg::OnBatchUpdateButtonPressed()
{
	QModelIndex index = m_batchGroup.pBatchesView->currentIndex();
	if (index.isValid())
	{
		/// Set up a new render item.
		SRenderItem& item = m_renderItems[index.row()];
		WriteDialogValuesToRenderItem(item);

		m_batchGroup.pUpdateButton->setDisabled(true);

		QStandardItemModel* pModel = static_cast<QStandardItemModel*>(m_batchGroup.pBatchesView->model());
		if (QStandardItem* pItem = pModel->item(index.row()))
		{
			pItem->setText(GenerateCaptureItemString(item).c_str());
		}
	}
}

void CTrackViewBatchRenderDlg::OnLoadBatchButtonPressed()
{
	string loadPath;
	if (CFileUtil::SelectFile("Render Batch Files (*.batch)|*.batch||", GetIEditor()->GetUserFolder(), loadPath))
	{
		XmlNodeRef batchRenderListNode = XmlHelpers::LoadXmlFromFile(loadPath);
		if (batchRenderListNode == nullptr)
		{
			return;
		}

		i32 version = 0;
		batchRenderListNode->getAttr("version", version);
		if (version != batchRenderFileVersion)
		{
			//MessageBox("The file version is different!", "Cannot load", MB_OK|MB_ICONERROR);
			return;
		}

		OnBatchClearButtonPressed();

		SRenderItem item;
		for (i32 i = 0, e = batchRenderListNode->getChildCount(); i < e; ++i)
		{
			XmlNodeRef itemNode = batchRenderListNode->getChild(i);

			// Inputr
			tukk szSeqquenceName = itemNode->getAttr("sequence");
			item.pSequence = GetIEditor()->GetMovieSystem()->FindSequence(szSeqquenceName);
			if (item.pSequence == nullptr)
			{
				/*string warningMsg;
				   warningMsg.Format("A sequence of '%s' not found! This'll be skipped.", seqName.GetString());
				   MessageBox(warningMsg, "Sequence not found", MB_OK|MB_ICONWARNING);*/
				continue;
			}

			tukk szDirectorName = itemNode->getAttr("director");
			for (i32 sqeNodeIdx = 0; sqeNodeIdx < item.pSequence->GetNodeCount(); ++sqeNodeIdx)
			{
				IAnimNode* pNode = item.pSequence->GetNode(sqeNodeIdx);
				if (pNode->GetType() == eAnimNodeType_Director && strcmp(szDirectorName, pNode->GetName()) == 0)
				{
					item.pDirectorNode = pNode;
					break;
				}
			}
			if (item.pDirectorNode == nullptr)
			{
				/*string warningMsg;
				   warningMsg.Format("A director node of '%s' not found in the sequence of '%s'! This'll be skipped.",
				   directorName.GetString(), seqName.GetString());
				   MessageBox(warningMsg, "Director node not found", MB_OK|MB_ICONWARNING);*/
				continue;
			}

			i32 ticks = 0;
			itemNode->getAttr("startframe", ticks);
			item.frameRange.start = SAnimTime(ticks);

			itemNode->getAttr("endframe", ticks);
			item.frameRange.end = SAnimTime(ticks);

			// Output
			itemNode->getAttr("width", item.resW);
			itemNode->getAttr("height", item.resH);
			itemNode->getAttr("fps", item.fps);

			itemNode->getAttr("format", item.formatIndex);
			itemNode->getAttr("bufferstocapture", item.bufferIndex);
			item.prefix = itemNode->getAttr("prefix");
			itemNode->getAttr("createvideo", item.bCreateVideo);

			for (i32 cvarIdx = 0; cvarIdx < itemNode->getChildCount(); ++cvarIdx)
			{
				tukk szCVar = itemNode->getChild(cvarIdx)->getContent();
				item.cvars.emplace_back(string(szCVar));
			}

			item.folder = itemNode->getAttr("folder");

			AddRenderItem(item);
		}
	}
}

void CTrackViewBatchRenderDlg::OnSaveBatchButtonPressed()
{
	string savePath;
	if (CFileUtil::SelectSaveFile("Render Batch Files (*.batch)|*.batch||", "batch", GetIEditor()->GetUserFolder(), savePath))
	{
		XmlNodeRef batchRenderListNode = XmlHelpers::CreateXmlNode("batchrenderlist");
		batchRenderListNode->setAttr("version", batchRenderFileVersion);

		for (const SRenderItem& item : m_renderItems)
		{
			XmlNodeRef itemNode = batchRenderListNode->newChild("item");

			// Input
			itemNode->setAttr("sequence", item.pSequence->GetName());
			itemNode->setAttr("director", item.pDirectorNode->GetName());
			itemNode->setAttr("startframe", item.frameRange.start.GetTicks());
			itemNode->setAttr("endframe", item.frameRange.end.GetTicks());

			// Output
			itemNode->setAttr("width", item.resW);
			itemNode->setAttr("height", item.resH);
			itemNode->setAttr("fps", item.fps);

			itemNode->setAttr("format", item.formatIndex);
			itemNode->setAttr("bufferstocapture", item.bufferIndex);
			itemNode->setAttr("prefix", item.prefix.c_str());
			itemNode->setAttr("createvideo", item.bCreateVideo);

			for (const string& cvar : item.cvars)
			{
				itemNode->newChild("cvar")->setContent(cvar.c_str());
			}

			itemNode->setAttr("folder", item.folder.c_str());
		}

		XmlHelpers::SaveXmlNode(batchRenderListNode, savePath);
	}
}

void CTrackViewBatchRenderDlg::OnActionButtonPressed()
{
	if (m_renderContext.IsInRendering())
	{
		bool bNoCancellation = m_renderContext.bWarmingUpAfterResChange || m_renderContext.bFFMPEGProcessing;
		if (!bNoCancellation)
		{
			GetIEditor()->GetMovieSystem()->AbortSequence(m_renderItems[m_renderContext.currentItemIndex].pSequence);
		}
	}
	else
	{
		/// Start a new batch.
		m_pActionButton->setText("Cancel");
		//m_pActionButton->setIcon();

		// Inform the movie system that it soon will be in a batch-rendering mode.
		GetIEditor()->GetMovieSystem()->EnableBatchRenderMode(true);

		InitializeRenderContext();
		BegCaptureItem();
	}
}

void CTrackViewBatchRenderDlg::OnCancelButtonPressed()
{
	if (m_renderContext.IsInRendering())
	{
		bool bNoCancellation = m_renderContext.bWarmingUpAfterResChange || m_renderContext.bFFMPEGProcessing;
		if (!bNoCancellation)
		{
			GetIEditor()->GetMovieSystem()->AbortSequence(m_renderItems[m_renderContext.currentItemIndex].pSequence);
		}
	}
	else
	{
		// Save options when closed.
		string defaultPresetPath = GetIEditor()->GetUserFolder();
		defaultPresetPath += szDefaultPresetFilename;
		SaveOutputPreset(defaultPresetPath.GetString());

		reject();
	}
}

void CTrackViewBatchRenderDlg::InitializeRenderContext()
{
	m_renderContext.currentItemIndex = 0;
	m_renderContext.spentTicks = 0;
	m_renderContext.expectedTotalTicks = 0;
	if (gEnv && gEnv->pRenderer)
	{
		CRenderViewport* pGameViewport = (CRenderViewport*)GetIEditor()->GetViewManager()->GetGameViewport();
		if (pGameViewport)
		{
			auto displayContext = pGameViewport->GetDisplayContext();
			m_displayContextKey = displayContext.GetDisplayContextKey();
			pGameViewport->GetResolution(m_viewPortResW, m_viewPortResH);
		}
		else
		{
			m_displayContextKey = {};
			m_viewPortResW = 0;
			m_viewPortResH = 0;
		}
	}

	for (SRenderItem renderItem : m_renderItems)
	{
		AnimRange rng = renderItem.frameRange;
		m_renderContext.expectedTotalTicks += (rng.end - rng.start).GetTicks();
	}
}

void CTrackViewBatchRenderDlg::BegCaptureItem()
{
	const SRenderItem& renderItem = m_renderItems[m_renderContext.currentItemIndex];

	IAnimSequence* pNextSequence = renderItem.pSequence;

	// Initialize the next one for the batch rendering.
	// Set the active shot.
	m_renderContext.pActiveDirectorBU = pNextSequence->GetActiveDirector();
	pNextSequence->SetActiveDirector(renderItem.pDirectorNode);

	// Back up flags and range of the sequence.
	m_renderContext.flagBU = pNextSequence->GetFlags();
	m_renderContext.rangeBU = pNextSequence->GetTimeRange();

	// A margin value to capture the precise number of frames
	AnimRange newRange = renderItem.frameRange;
	i32 newEnd = newRange.end.GetTicks() + SAnimTime::numTicksPerSecond;
	newRange.end = SAnimTime(newEnd);
	pNextSequence->SetTimeRange(newRange);

	// Set up the custom config cvars for this item.
	for (const string& cvar : renderItem.cvars)
	{
		GetIEditor()->GetSystem()->GetIConsole()->ExecuteString(cvar.c_str());
	}

	// Set specific capture options for this item.
	m_renderContext.captureOptions.m_frameRate = renderItem.fps;
	m_renderContext.captureOptions.m_timeStep = 1.0f / renderItem.fps;
	m_renderContext.captureOptions.m_bufferToCapture = static_cast<SCaptureFormatInfo::ECaptureBuffer>(renderItem.bufferIndex);

	drx_strcpy(m_renderContext.captureOptions.m_prefix, renderItem.prefix.c_str());

	m_renderContext.captureOptions.m_captureFormat = (SCaptureFormatInfo::ECaptureFileFormat)renderItem.formatIndex;
	m_renderContext.captureOptions.m_time = renderItem.frameRange.start;
	m_renderContext.captureOptions.m_duration = (renderItem.frameRange.end - renderItem.frameRange.start);
	m_renderContext.captureOptions.m_bOnce = (m_renderContext.captureOptions.m_duration == SAnimTime(0));

	const QModelIndex selectedEntryIdx = m_batchGroup.pBatchesView->model()->index(m_renderContext.currentItemIndex, 0);
	if (!selectedEntryIdx.isValid())
	{
		DrxLog("Couldn't process batch item with index %d, canceling...", m_renderContext.currentItemIndex);
		OnCancelButtonPressed();
		return;
	}

	QString itemText = m_batchGroup.pBatchesView->model()->data(selectedEntryIdx, Qt::DisplayRole).value<QString>();
	itemText.replace('/', '-'); // A full sequence name can have slash characters which aren't suitable for a file name.

	string folder(renderItem.folder.c_str());
	folder += "\\";
	folder += itemText.toUtf8().data();

	string finalFolder(folder);
	i32 i = 2;
	while (CFileUtil::PathExists(folder) && CFileUtil::PathExists(finalFolder))
	{
		finalFolder = folder;
		string suffix;
		suffix.Format("_v%d", i);
		finalFolder += suffix;
		++i;
	}
	drx_strcpy(m_renderContext.captureOptions.m_folder, finalFolder.GetString());

	/// Change the resolution. NOT
	ICVar* pCVarCustomResWidth = gEnv->pConsole->GetCVar("r_CustomResWidth");
	ICVar* pCVarCustomResHeight = gEnv->pConsole->GetCVar("r_CustomResHeight");
	if (pCVarCustomResWidth && pCVarCustomResHeight)
	{
		// If available, use the custom resolution cvars.
		m_renderContext.cvarCustomResWidthBU = pCVarCustomResWidth->GetIVal();
		m_renderContext.cvarCustomResHeightBU = pCVarCustomResHeight->GetIVal();
		pCVarCustomResWidth->Set(renderItem.resW);
		pCVarCustomResHeight->Set(renderItem.resH);
	}
	// resize viewport for each renderitem during sequence capture
	gEnv->pRenderer->ResizeContext(m_displayContextKey, renderItem.resW, renderItem.resH);

	// The capturing doesn't actually start here. It just flags the warming-up and
	// once it's done, then the capturing really begins.
	// The warming-up is necessary to settle down some post-fxs after the resolution change.
	m_renderContext.bWarmingUpAfterResChange = true;
	m_renderContext.timeWarmingUpStarted = GetTickCount();
}

void CTrackViewBatchRenderDlg::EndCaptureItem(IAnimSequence* pSequence)
{
	GetIEditor()->GetMovieSystem()->RemoveMovieListener(pSequence, this);
	GetIEditor()->SetInGameMode(false);
	GetIEditor()->GetGameEngine()->Update();
	GetIEditor()->Notify(eNotify_OnIdleUpdate);
	// to do: remove this cvar diddling?
	ICVar* pCVarCustomResWidth = gEnv->pConsole->GetCVar("r_CustomResWidth");
	ICVar* pCVarCustomResHeight = gEnv->pConsole->GetCVar("r_CustomResHeight");
	if (pCVarCustomResWidth && pCVarCustomResHeight)
	{
		// Restore the custom resolution cvars.
		pCVarCustomResWidth->Set(m_renderContext.cvarCustomResWidthBU);
		pCVarCustomResHeight->Set(m_renderContext.cvarCustomResHeightBU);
	}

	// Restore flags, range and the active director of the sequence.
	pSequence->SetFlags(m_renderContext.flagBU);
	pSequence->SetTimeRange(m_renderContext.rangeBU);
	pSequence->SetActiveDirector(m_renderContext.pActiveDirectorBU);

	SRenderItem renderItem = m_renderItems[m_renderContext.currentItemIndex];
	if(m_renderContext.currentItemIndex==m_renderItems.size()-1)  // after last item, reset viewport.
		gEnv->pRenderer->ResizeContext(m_displayContextKey, m_viewPortResW, m_viewPortResH);

	if (renderItem.bCreateVideo)
	{
		// Create a video using the ffmpeg plug-in from captured images.
		m_renderContext.bFFMPEGProcessing = true;

		stack_string outputFolder = m_renderContext.captureOptions.m_folder;

		Concurrency::single_assignment<bool> bDone;
		Concurrency::task_group ffmpegTask;
		ffmpegTask.run(
		  [&renderItem, &outputFolder, &bDone]
		{
			stack_string outputFile = outputFolder;
			outputFile += "\\";
			outputFile += renderItem.prefix;

			stack_string inputFile;
			inputFile = outputFile;
			inputFile += "%06d.jpg";

			outputFile += ".mp4";

			GetIEditor()->ExecuteCommand(
			  "plugin.ffmpeg_encode '%s' '%s' '%s' %d '-r %d -vf crop=%d:%d:0:0'",
			  inputFile.c_str(),
			  outputFile.c_str(),
			  "mpeg4",
			  10240,
			  renderItem.fps,
			  renderItem.resW,
			  renderItem.resH
			  );

			Concurrency::send(bDone, true);
		}
		  );

		bool bDummy;
		do
		{
			Update();
		}
		while (Concurrency::try_receive(bDone, bDummy) == false);
		m_renderContext.bFFMPEGProcessing = false;
	}
}

void CTrackViewBatchRenderDlg::Update()
{
	const char rotatingCursor[] = { '|', '/', '-', '\\' };

	static i32 count = 0;
	if (m_renderContext.IsInRendering())
	{
		if (m_renderContext.bWarmingUpAfterResChange)  /// A warming-up phase
		{
			DrxStackStringT<char, 32> msg("Warming up ");
			msg += rotatingCursor[count++ % arraysize(rotatingCursor)];
			m_pStatusMessage->setText(msg.c_str());

			GetIEditor()->GetGameEngine()->Update();
			GetIEditor()->Notify(eNotify_OnIdleUpdate);

			if (GetTickCount() > m_renderContext.timeWarmingUpStarted + warmingUpDuration)
			{
				// The warming-up done.
				m_renderContext.bWarmingUpAfterResChange = false;
				count = 0;

				// Start capturing item
				const SRenderItem& renderItem = m_renderItems[m_renderContext.currentItemIndex];

				IAnimSequence* pNextSequence = renderItem.pSequence;
				GetIEditor()->GetMovieSystem()->StartCapture(pNextSequence, m_renderContext.captureOptions);
				GetIEditor()->SetInGameMode(true);
				GetIEditor()->GetMovieSystem()->AddMovieListener(pNextSequence, this);
			}
		}
		else if (m_renderContext.bFFMPEGProcessing) /// A ffmpeg-processing phase
		{
			DrxStackStringT<char, 32> msg("FFMPEG processing ");
			msg += rotatingCursor[count++ % arraysize(rotatingCursor)];
			m_pStatusMessage->setText(msg.c_str());

			GetIEditor()->GetGameEngine()->Update();
			GetIEditor()->Notify(eNotify_OnIdleUpdate);
		}
		else /// A capturing phase
		{
			// Progress bar
			IAnimSequence* pCurSequence = m_renderItems[m_renderContext.currentItemIndex].pSequence;
			AnimRange rng = pCurSequence->GetTimeRange();

			SAnimTime elapsedTime = GetIEditor()->GetMovieSystem()->GetPlayingTime(pCurSequence) - rng.start;
			if (elapsedTime.GetTicks() > 0 && m_renderContext.expectedTotalTicks > 0)
			{
				i32k totalPercentage = i32(100.0f * (m_renderContext.spentTicks + elapsedTime.GetTicks()) / m_renderContext.expectedTotalTicks);
				m_pProgressBar->setVal(totalPercentage);

				// Progress message
				const QModelIndex selectedEntryIdx = m_batchGroup.pBatchesView->model()->index(m_renderContext.currentItemIndex, 0);
				QString itemText = selectedEntryIdx.data(Qt::DisplayRole).value<QString>();

				stack_string msg;
				msg.Format("Rendering '%s' (%d%%)", itemText.toUtf8().data(), totalPercentage);
				m_pStatusMessage->setText(QObject::tr(msg.c_str()));
			}

			GetIEditor()->GetGameEngine()->Update();
		}
	}
	else
	{
		const QModelIndex selectedEntryIdx = m_batchGroup.pBatchesView->currentIndex();
		if (selectedEntryIdx.isValid())
		{
			// If any of settings changed, then enable the 'update button'.
			// Otherwise, disable it.
			SRenderItem item;
			const bool bSettingChanged = WriteDialogValuesToRenderItem(item) && !(item == m_renderItems[selectedEntryIdx.row()]);
			m_batchGroup.pUpdateButton->setEnabled(bSettingChanged);
		}
	}

	m_pUpdateTimer->stop();
	m_pUpdateTimer->start();
}

void CTrackViewBatchRenderDlg::OnMovieEvent(IMovieListener::EMovieEvent event, IAnimSequence* pSequence)
{
	if (event == IMovieListener::eMovieEvent_CaptureStopped || event == IMovieListener::eMovieEvent_Aborted)
	{
		/// Finalize the current one, if any.
		if (pSequence)
		{
			EndCaptureItem(pSequence);

			const bool bDone = m_renderContext.currentItemIndex == m_renderItems.size() - 1;
			const bool bCancelled = event == IMovieListener::eMovieEvent_Aborted;
			if (bDone || bCancelled)
			{
				// Display the final progress message.
				if (bCancelled)
				{
					m_pProgressBar->setVal(0);
					m_pStatusMessage->setText(QObject::tr("Rendering cancelled"));
				}
				else
				{
					m_pProgressBar->setVal(100);
					m_pStatusMessage->setText(QObject::tr("Rendering finished"));
				}

				/// End the batch.
				GetIEditor()->GetMovieSystem()->EnableBatchRenderMode(false);

				m_pActionButton->setText("Start");
				//m_pActionButton->setIcon();

				m_renderContext.currentItemIndex = -1;
				//m_pluginMessage->setText();
				//GetDlgItem(IDC_BATCH_RENDER_PRESS_ESC_TO_CANCEL)->SetWindowText(m_ffmpegPluginStatusMsg);

				return;
			}

			/// Update the context.
			m_renderContext.spentTicks += m_renderContext.captureOptions.m_duration.GetTicks();
			++m_renderContext.currentItemIndex;
		}

		/// Trigger the next item.
		BegCaptureItem();
	}
}

void CTrackViewBatchRenderDlg::keyPressEvent(QKeyEvent* pEvent)
{
	switch (pEvent->key())
	{
	case Qt::Key_Return:
	case Qt::Key_Enter:
		break;

	case Qt::Key_Escape:
		OnCancelButtonPressed();
		break;

	default:
		QDialog::keyPressEvent(pEvent);
	}
}

u32 CTrackViewBatchRenderDlg::GetCurrentTicksPerFrame() const
{
	u32 ticksPerFrame = 0;
	i32k selectedFpsIdx = m_outputGroup.pFpsField->currentIndex();
	if (m_outputGroup.pFpsField->currentIndex() < arraysize(fixedFpsOptions))
	{
		ticksPerFrame = SAnimTime::numTicksPerSecond / fixedFpsOptions[selectedFpsIdx].fps;
	}
	else
	{
		ticksPerFrame = SAnimTime::numTicksPerSecond / m_customFPS;
	}
	return ticksPerFrame;
}

string CTrackViewBatchRenderDlg::GenerateCaptureItemString(const SRenderItem& item)
{
	stack_string itemText;
	itemText.Format("%s_%s_%d-%d(%dx%d,%dfps,%s)%s",
	                item.pSequence->GetName(),
	                item.pDirectorNode->GetName(),
	                item.frameStart,
	                item.frameEnd,
	                item.resW,
	                item.resH,
	                item.fps,
	                SCaptureFormatInfo::GetCaptureBufferName((SCaptureFormatInfo::ECaptureBuffer)item.bufferIndex),
	                item.bCreateVideo ? "[v]" : ""
	                );

	return itemText;
}

