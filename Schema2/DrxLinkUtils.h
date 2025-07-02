// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Unify uri format used by CreateUri and Execute.

#pragma once

#include <drx3D/Sys/IConsole.h>

#include <drx3D/Schema2/GUID.h>
#include <drx3D/Schema2/IString.h>
#include <drx3D/Schema2/StackString.h>

namespace sxema2
{
namespace DrxLinkUtils
{
enum class ECommand
{
	None = 0,     // Do nothing.
	Show          // Show element in editor.
};

inline bool CreateUri(IString& output, ECommand command, const sxema2::SGUID& elementGUID, const sxema2::SGUID& detailGUID = sxema2::SGUID())
{
	CStackString commandLine;
	switch (command)
	{
	case ECommand::Show:
		{
			CStackString temp;
			commandLine = "sc_rpcShowLegacy ";
			sxema2::StringUtils::SysGUIDToString(elementGUID.sysGUID, temp);
			commandLine.append(temp);
			commandLine.append(" ");
			sxema2::StringUtils::SysGUIDToString(detailGUID.sysGUID, temp);
			commandLine.append(temp);
			break;
		}
	}
	if (!commandLine.empty())
	{
		DrxLinkService::CDrxLinkUriFactory drxLinkFactory("editor", DrxLinkService::ELinkType::Commands);
		drxLinkFactory.AddCommand(commandLine.c_str(), commandLine.length());
		output.assign(drxLinkFactory.GetUri());
		return true;
	}
	return false;
}

inline void ExecuteUri(tukk szUri)
{
	DrxLinkService::CDrxLink drxLink(szUri);
	tukk szCmd = drxLink.GetQuery("cmd1");
	if (szCmd && szCmd[0])
	{
		gEnv->pConsole->ExecuteString(szCmd);
	}
}

inline void ExecuteCommand(ECommand command, const sxema2::SGUID& elementGUID, const sxema2::SGUID& detailGUID = sxema2::SGUID())
{
	CStackString uri;
	CreateUri(uri, command, elementGUID, detailGUID);
	ExecuteUri(uri.c_str());
}
} // DrxLinkUtils
} // sxema
