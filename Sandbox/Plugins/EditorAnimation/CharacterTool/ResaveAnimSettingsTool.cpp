// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"

#include "BatchFileDialog.h"
#include <drx3D/CoreX/Serialization/StringList.h>
#include <QApplication>
#include "AnimationList.h"
#include <drx3D/CoreX/Platform/DrxWindows.h>
#include "Controls/QuestionDialog.h"

namespace CharacterTool
{

void ShowResaveAnimSettingsTool(AnimationList* animationList, QWidget* parent)
{
	Serialization::StringList filenames;
	SBatchFileSettings settings;
	settings.useDrxPak = true;
	settings.scanExtension = "animsettings";
	settings.title = "Resave AnimSettings";
	settings.stateFilename = "resaveAnimSettings.state";
	settings.listLabel = "AnimSettings files";
	settings.descriptionText = "Files marked below will be automatically resaved (converted to new format if needed).";

	if (ShowBatchFileDialog(&filenames, settings, parent))
	{
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		QApplication::processEvents();

		i32 numFailed = 0;
		for (size_t i = 0; i < filenames.size(); ++i)
		{
			if (!animationList->ResaveAnimSettings(filenames[i].c_str()))
			{
				DrxLogAlways("Failed to resave animsettings: \"%s\"", filenames[i].c_str());
				++numFailed;
			}
		}
		if (numFailed > 0)
		{
			QString message;
			message.sprintf("Failed to resave %d files. See Sandbox log for details.", numFailed);
			CQuestionDialog::SWarning("Error", message);
		}
		QApplication::restoreOverrideCursor();
	}
}

}

