// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "FileMonitorBase.h"

namespace ACE
{
class CFileMonitorMiddleware final : public CFileMonitorBase
{
	Q_OBJECT

public:

	explicit CFileMonitorMiddleware(i32 const delay, QObject* const pParent);

	CFileMonitorMiddleware() = delete;

	void Enable();

private:

	std::vector<char const*> m_monitorFolders;
};
} //endns ACE
