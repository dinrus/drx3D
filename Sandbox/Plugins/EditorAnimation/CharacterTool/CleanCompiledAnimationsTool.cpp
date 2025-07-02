// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"

#include "BatchFileDialog.h"
#include <drx3D/CoreX/Serialization/StringList.h>
#include <drx3D/CoreX/Platform/DrxWindows.h>
#include <Controls/QuestionDialog.h>
namespace CharacterTool
{

void ShowCleanCompiledAnimationsTool(QWidget* parent)
{
	Serialization::StringList filenames;
	SBatchFileSettings settings;
	settings.useDrxPak = false;
	settings.scanExtension = "caf";
	settings.title = "Clean Compiled Animations";
	settings.stateFilename = "cleanAnimations.state";
	settings.listLabel = "Compiled Animation Files";
	settings.descriptionText = "Files marked below will be deleted and recompiled if needed:";

	if (ShowBatchFileDialog(&filenames, settings, parent))
	{
		i32 numFailed = 0;
		for (size_t i = 0; i < filenames.size(); ++i)
		{
			tukk path = filenames[i].c_str();
			DWORD attribs = GetFileAttributesA(path);
			if (attribs == INVALID_FILE_ATTRIBUTES)
			{
				++numFailed;
			}
			else
			{
				if ((attribs & FILE_ATTRIBUTE_READONLY) != 0)
					SetFileAttributesA(path, FILE_ATTRIBUTE_NORMAL);

				if (!DeleteFileA(path))
					++numFailed;
			}
		}
		if (numFailed > 0)
		{
			QString message;
			message.sprintf("Failed to remove %d files.", numFailed);
			CQuestionDialog::SWarning("Error", message);
		}
	}
}

}

