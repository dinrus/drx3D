// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "FileSystem_Internal_FilterUtils.h"
#include "FileSystem_Internal_SnapshotUpdate.h"

#include "FileSystem/FileSystem_SubTreeMonitor.h"
#include "FileSystem/FileSystem_FileFilter.h"

namespace FileSystem
{
namespace Internal
{

/// \brief contains all active sub tree monitors
class CSubTreeMonitors
{
public:
	void Add(i32 handle, const SFileFilter& filter, const SubTreeMonitorPtr& monitor);
	void Remove(i32 handle);

	/// filter update for all monitors and run callback
	template<typename Callback>
	void UpdateAll(const SSnapshotUpdate& update, Callback callback)
	{
		foreach(const auto & entry, m_fileMonitors)
		{
			auto monitor = entry.monitor.lock();
			if (!monitor)
			{
				Remove(entry.handle);
				continue; // unused monitor
			}
			auto filteredUpdate = FilterUpdate(entry, update);
			callback(monitor, filteredUpdate);
		}
	}

private:
	typedef std::weak_ptr<ISubTreeMonitor> MonitorWeakPtr;

	struct SEntry
	{
		i32                             handle;
		SFileFilter                     filter;
		FilterUtils::SDirectoryIncluded root;
		MonitorWeakPtr                  monitor;
	};

private:
	static SDirectory     BuildDirectoryTree(const QVector<QString>& includedKeyPathes);
	SSubTreeMonitorUpdate FilterUpdate(const SEntry& entry, const SSnapshotUpdate& update);

private:
	QHash<i32, SEntry> m_fileMonitors;
};

} //endns Internal
} //endns FileSystem

