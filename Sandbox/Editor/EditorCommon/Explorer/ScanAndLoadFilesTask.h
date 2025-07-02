// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <drx3D/Sys/ISystem.h>
#include <IBackgroundTaskManager.h>
#include <QObject>
#include <vector>

namespace Explorer
{

using std::vector;

struct SLookupRule
{
	vector<string> masks;

	SLookupRule()
	{
	}
};

struct ScanLoadedFile
{
	string       scannedFile;
	string       loadedFile;
	vector<char> content;
	i32          pakState;  //  One or more values from IDrxPak::EFileSearchLocation

	ScanLoadedFile() : pakState(0) {}
};

struct SScanAndLoadFilesTask : QObject, IBackgroundTask
{
	Q_OBJECT
public:
	SLookupRule m_rule;
	vector<ScanLoadedFile> m_loadedFiles;
	string                 m_description;

	SScanAndLoadFilesTask(const SLookupRule& rule, tukk description);
	ETaskResult Work() override;
	void        Finalize() override;
	tukk Description() const { return m_description; }
	void        Delete() override   { delete this; }

signals:
	void SignalFileLoaded(const ScanLoadedFile& loadedFile);
	void SignalLoadingFinished();
};

}

