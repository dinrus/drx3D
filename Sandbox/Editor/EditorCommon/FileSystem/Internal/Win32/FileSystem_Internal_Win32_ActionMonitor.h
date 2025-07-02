// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "FileSystem_Internal_Win32_ActionSequence.h"

#include <QObject>
#include <QString>
#include <QVector>

#include <memory>

namespace FileSystem
{
namespace Internal
{
namespace Win32
{

/**
 * \brief monitor multiple pathes for changes on Win32-API
 *
 * this thread communicates with the windows kernel and works as fast as possible
 * to keep the buffer sizes low without loosing track too often
 * (this means you need a buffering adapter to get useful results)
 *
 * OPTION: actively manage buffer sizes - increase it if we loose track and did not hang
 * OPTION: switch to an event-stream output and move buffering to InfoThread
 */
class CActionMonitor
	: public QObject
{
	Q_OBJECT

public:
	CActionMonitor();
	~CActionMonitor();

	/// \param key uniquely identifies the monitor
	/// \param path is an absolute filesystem path
	void AddPath(u64 key, const QString& fullAbsolutePath);

	/// \brief top monitoring the path represented by the key
	void Remove(u64 key);

signals:
	/// \note: called through internal thread - always use a Qt::QueuedConnection!
	void SendChanges(const FileSystem::Internal::Win32::SActionSequence&);

	/// \note: called through internal thread - always use a Qt::QueuedConnection!
	void SendLostTrack(const QString&);

private:
	struct Implementation;
	std::unique_ptr<Implementation> p;
};

} //endns Win32
} //endns Internal
} //endns FileSystem

