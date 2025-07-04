// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "../stdafx.h"
#include "../Command.h"
#include <drx3D/CoreX/String/DrxString.h>

CCommandArgument::CCommandArgument(CCommand* const pCommand, const size_t index, const string& name, const string& description)
	: m_pCommand(pCommand)
	, m_index(index)
	, m_name(name)
	, m_description(description)
{
}

CCommand* CCommandArgument::GetCommand() const
{
	return m_pCommand;
}

const string& CCommandArgument::GetDescription() const
{
	return m_description;
}

size_t CCommandArgument::GetIndex() const
{
	return m_index;
}

const string& CCommandArgument::GetName() const
{
	return m_name;
}

