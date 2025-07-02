// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "FileSystem_Internal_Win32_ActionSequence.h"
#include "FileSystem/Internal/FileSystem_Internal_UpdateSequence.h"

#include <QObject>
#include <memory>

namespace FileSystem
{
namespace Internal
{
namespace Win32
{

/**
 * \brief adapter between Win32_MonitorThread and FileSystemMonitorThread
 * \note instances are moved to the internal thread
 *
 * enriches the Changes with all the necessary details about files and directories
 */
class CMonitorAdapter
	: public QObject
{
	Q_OBJECT

public:
	CMonitorAdapter();
	~CMonitorAdapter();

public slots:
	void Enqueue(const SActionSequence& changes);

signals:
	void SendLostTrack(const QString& fullPath);
	void SendUpdates(const FileSystem::Internal::CUpdateSequence&);

private:
	struct Implementation;
	std::unique_ptr<Implementation> p;
};

} //endns Win32
} //endns Internal
} //endns FileSystem

