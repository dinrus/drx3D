// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "ScanAndLoadFilesTask.h"
#include <drx3D/Sys/File/IDrxPak.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include "ExplorerFileList.h"

namespace Explorer
{

SScanAndLoadFilesTask::SScanAndLoadFilesTask(const SLookupRule& rule, tukk description)
	: m_rule(rule)
	, m_description(description)
{
}

ETaskResult SScanAndLoadFilesTask::Work()
{
	const SLookupRule& rule = m_rule;
	vector<string> masks = rule.masks;

	vector<string> filenames;

	for (size_t j = 0; j < masks.size(); ++j)
	{
		filenames.clear();
		const string& mask = masks[j];
		SDirectoryEnumeratorHelper dirHelper;
		dirHelper.ScanDirectoryRecursive("", "", mask.c_str(), filenames);

		m_loadedFiles.reserve(m_loadedFiles.size() + filenames.size());
		for (i32 k = 0; k < filenames.size(); ++k)
		{
			if (!DrxStringUtils::MatchWildcard(filenames[k].c_str(), mask.c_str()))
				continue;

			ScanLoadedFile file;
			file.scannedFile = filenames[k].c_str();
			file.pakState = ExplorerFileList::GetFilePakState(file.scannedFile.c_str());
			m_loadedFiles.push_back(file);
		}
	}

	return eTaskResult_Completed;
}

void SScanAndLoadFilesTask::Finalize()
{
	for (size_t i = 0; i < m_loadedFiles.size(); ++i)
	{
		const ScanLoadedFile& loadedFile = m_loadedFiles[i];

		SignalFileLoaded(loadedFile);
	}

	SignalLoadingFinished();
}

}

