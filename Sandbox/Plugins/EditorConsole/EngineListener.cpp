// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "EngineListener.h"

//string compare/ordering functions used
//note: these are mostly non-standard (MSVC only), so I grouped them here in case this file ever needs to be ported
//these ignore the locale settings (if any) that are set for the process or thread, and always use the "C" locale
//for proper unicode support, these need to be changed, however, since ordering is only applied to CVar and CCommand names, it's not currently needed (nor ever, probably)
struct str
{
	//ctor/dtor take care of the locale we use
	const _locale_t locale;
	str() : locale(_create_locale(LC_ALL, "C")) {}
	~str() { _free_locale(locale); }

	//case-insensitive compare
	i32 cmpi(tukk left, tukk right) const { return _stricmp_l(left, right, locale); }

	//case-insensitive compare, fixed max length
	i32 cmpni(tukk left, tukk right, size_t length) const { return _strnicmp_l(left, right, length, locale); }

	//lower case variant, if any
	char tolower(const char c) const { return (char)_tolower_l(c, locale); }

	//case-insensitive find substring
	tukk stri(tukk haystack, tukk needle) const
	{
		size_t haystack_length = strlen(haystack);
		size_t needle_length = strlen(needle);
		if (needle_length > haystack_length)
			return nullptr;

		tukk result = std::search(haystack, haystack + haystack_length, needle, needle + needle_length, [this](char left, char right) { return tolower(left) == tolower(right); });
		return result != haystack + haystack_length ? result : nullptr;
	}
} strfunc;

//map string compare function to the above string functions
i32 CEngineListener::StringCompare(tukk left, tukk right)
{
	return strfunc.cmpi(left, right);
}

//initialize event handler
void CEngineListener::Init(size_t maxHistory)
{
	DrxAutoCriticalSection lock(m_lock);
	IConsole* pConsole = GetConsole();
	if (pConsole)
	{
		//allocate space for history
		m_history.set_capacity(maxHistory);

		//catch up initial history
		i32 lines = pConsole->GetLineCount();
		i32k cMaxLineLength = 1000;
		for (i32 line = 0; line < lines; ++line)
		{
			char buf[cMaxLineLength];
			if (pConsole->GetLineNo(lines - line - 1, buf, cMaxLineLength))
			{
				Print(buf);
			}
		}

		//get CVars and comamnds
		RefreshCVarsAndCommands();

		//add event handlers
		pConsole->AddOutputPrintSink(this);
		pConsole->AddConsoleVarSink(this);
	}
}

//force refresh of all CVar and commands
//they COULD get out-of-sync when someone registers new commands after startup
void CEngineListener::RefreshCVarsAndCommands()
{
	m_cVars.clear();
	m_commands.clear();

	DrxAutoCriticalSection lock(m_lock);
	IConsole* pConsole = GetConsole();
	if (pConsole)
	{
		//register all known CVar
		pConsole->DumpCVars(this);

		//get all the CVar in addition to all the commands
		//since we know all the CVar (just dumped them), we can determine what commands we have
		//there is no way to get the list of commands directly
		i32 count = pConsole->GetNumVars(true);
		if (count)
		{
			std::vector<tukk > haystack(count, NULL);
			count = pConsole->GetSortedVars(&haystack[0], count);
			t_cVars::const_iterator begin = m_cVars.begin();
			char laststart = 0;
			for (i32 i = 0; i < count; ++i)
			{
				tukk needle = haystack[i];
				bool is_cvar = false;
				for (t_cVars::const_iterator j = begin; j != m_cVars.end(); ++j) //engine sorting is slightly different, so we have to
				{
					tukk candidate = (*j)->GetName();
					i32 compare = strfunc.cmpi(candidate, needle);
					if (compare >= 0)
					{
						is_cvar = (compare == 0);
						break;
					}

					//optimization to not use O(N^2)
					//we assume the engine also sorts case-insensitive
					//however, it doesn't use the same sorting algorithm as stricmp does, so we can't assume identical sorting (ie, a_ and ai_ sort differently)
					char currentstart = strfunc.tolower(*candidate);
					if (currentstart != laststart && currentstart != '_')
					{
						begin = j;
						laststart = currentstart;
					}
				}
				if (!is_cvar)
				{
					//this is not a known CVar, so it must be a command
					t_string command = needle;
					m_commands.insert(std::move(command));
				}
			}
		}

		t_string command;
		std::vector<CCommand*> cmds;
		GetIEditor()->GetICommandManager()->GetCommandList(cmds);

		for (size_t cmdno = 0; cmdno < cmds.size(); ++cmdno)
		{
			command = cmds[cmdno]->GetCommandString();
			m_commands.insert(std::move(command));
		}
	}
}

//update a CVar
void CEngineListener::UpdateCVar(ICVar* pVar, bool changed)
{
	DrxAutoCriticalSection lock(m_lock);
	if (pVar)
	{
		if (m_cVars.insert(pVar).second || changed)
		{
			EmitCVar(pVar);
		}
	}
}

//release a CVar
void CEngineListener::ReleaseCVar(ICVar* pVar)
{
	DrxAutoCriticalSection lock(m_lock);
	if (pVar)
	{
		t_cVars::iterator varIt = m_cVars.find(pVar);
		if (varIt != m_cVars.end())
		{
			DestroyCVar(pVar);
			m_cVars.erase(varIt);
		}
	}
}

//get all known CVars
void CEngineListener::GetAllCVars(const std::function<void (ICVar*)>& function) const
{
	DrxAutoCriticalSection lock(m_lock);
	for (t_cVars::const_iterator i = m_cVars.begin(); i != m_cVars.end(); ++i)
	{
		function(*i);
	}
}

//get filtered CVars
void CEngineListener::GetFilteredCVars(tukk contains, const std::function<void (ICVar*)>& function) const
{
	//in the case where the filter expression is meaningless, just return all CVars
	if (!contains || contains[0] == 0) GetAllCVars(function);

	DrxAutoCriticalSection lock(m_lock);
	for (t_cVars::const_iterator i = m_cVars.begin(); i != m_cVars.end(); ++i)
	{
		ICVar* pVar = *i;
		if (strfunc.stri(pVar->GetName(), contains))
		{
			function(pVar);
		}
	}
}

//get all known lines (that fit in history)
void CEngineListener::GetAllLines(const std::function<void (size_t, const t_string&)>& function) const
{
	DrxAutoCriticalSection lock(m_lock);
	size_t index = m_lines - m_history.size() + 1;
	for (t_history::const_iterator i = m_history.begin(); i != m_history.end(); ++i, ++index)
	{
		function(index, *i);
	}
}

//clean up
CEngineListener::~CEngineListener()
{
	DrxAutoCriticalSection lock(m_lock);
	IConsole* pConsole = GetConsole();
	if (pConsole)
	{
		pConsole->RemoveOutputPrintSink(this);
		pConsole->RemoveConsoleVarSink(this);
	}
}

//notify of new console output
void CEngineListener::Print(tukk inszText)
{
	DrxAutoCriticalSection lock(m_lock);
	if (inszText)
	{
		t_string line = inszText;
		if (!line.empty())
		{
			m_history.push_back(std::move(line));
			EmitLine(++m_lines, *m_history.rbegin());
		}
	}
}

void CEngineListener::AutoComplete(const t_string& query, const std::function<void (const t_string&, ICVar*)>& function) const
{
	tukk pQuery = query.c_str();

	for (ICVar* cvar : m_cVars)
	{
		tukk cvarName = cvar->GetName();
		if (strfunc.stri(cvarName, pQuery) != nullptr)
		{
			function(t_string(cvarName), cvar);
		}
	}

	for (auto& command : m_commands)
	{
		if (strfunc.stri(command.c_str(), pQuery) != nullptr)
		{
			function(command, nullptr);
		}
	}
}

//execute a command in the console
void CEngineListener::Execute(const t_string& command) const
{
	DrxAutoCriticalSection lock(m_lock);
	IConsole* pConsole = GetConsole();
	if (pConsole)
	{
		pConsole->PrintLine(command.c_str());
		pConsole->ExecuteString(command.c_str());
	}
}

