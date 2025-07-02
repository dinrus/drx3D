// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#if DRX_PLATFORM_WINDOWS

	#include <drx3D/Sys/XConsole.h>   // CXConsole, struct string_nocase_lt

// extract consoel variable and command help
// in some HTML pages and many small files that can be included in Confluence wiki
// pages (so maintain the documentation only in one place)
//
// Possible improvements/Known issues:
//   - Nicer HTML layout (CSS?)
//   - Searching in the content of the cvars is tricky (main page doesn't have help content)
//   - %TODO% (was wiki image, should look good in confluence and HTML)
//   - many small files should be stored in some extra folder for clearity
//   - before generating the data the older directoy should be cleaned
//   - file should be generated in the user folder
//   - "wb" should be used instead of "w", to get the same result in unix
class CConsoleHelpGen
{
public:
	CConsoleHelpGen(CXConsole& rParent) : m_rParent(rParent), m_eWorkMode(eWM_None)
	{
	}

	void Work();

private: // --------------------------------------------------------

	enum EWorkMode
	{
		eWM_None,
		eWM_HTML,
		eWM_Confluence
	};

	//
	void CreateMainPages();

	// to create one file for for each cvar/command in confluence style
	void CreateFileForEachEntry();

	// insert if the name starts with the with prefix
	void InsertConsoleVars(std::set<tukk , string_nocase_lt>& setCmdAndVars, tukk szPrefix) const;
	// insert if the name starts with the with prefix
	void InsertConsoleCommands(std::set<tukk , string_nocase_lt>& setCmdAndVars, tukk szPrefix) const;
	// insert if the name does not start with any of the prefix in the map
	void InsertConsoleVars(std::set<tukk , string_nocase_lt>& setCmdAndVars, std::map<string, tukk > mapPrefix) const;
	// insert if the name does not start with any of the prefix in the map
	void InsertConsoleCommands(std::set<tukk , string_nocase_lt>& setCmdAndVars, std::map<string, tukk > mapPrefix) const;

	// a single file for the entry is generate
	void               CreateSingleEntryFile(tukk szName) const;

	void               IncludeSingleEntry(FILE* f, tukk szName) const;

	static string      FixAnchorName(tukk szName);
	static string      GetCleanPrefix(tukk p);
	// split before "|" (to get the prefix itself)
	static string      SplitPrefixString_Part1(tukk p);
	// split string after "|" (to get the optional help)
	static tukk SplitPrefixString_Part2(tukk p);

	void               StartPage(FILE* f, tukk szPageName, tukk szPageDescription) const;
	void               EndPage(FILE* f) const;
	void               StartH1(FILE* f, tukk szName) const;
	void               EndH1(FILE* f) const;
	void               StartH3(FILE* f, tukk szName) const;
	void               EndH3(FILE* f) const;
	void               StartCVar(FILE* f, tukk szName) const;
	void               EndCVar(FILE* f) const;
	void               SingleLinePrefix(FILE* f, tukk szPrefix, tukk szPrefixDesc, tukk szLink) const;
	void               StartPrefix(FILE* f, tukk szPrefix, tukk szPrefixDesc, tukk szLink) const;
	void               EndPrefix(FILE* f) const;
	void               SingleLineEntry_InGlobal(FILE* f, tukk szName, tukk szLink) const;
	void               SingleLineEntry_InGroup(FILE* f, tukk szName, tukk szLink) const;
	void               Anchor(FILE* f, tukk szName) const;

	// to log some inital stats like date or application name
	void KeyValue(FILE* f, tukk szKey, tukk szValue) const;

	// prefix explanation and indention for following text
	void Explanation(FILE* f, tukk szText) const;

	void Separator(FILE* f) const;

	void LogVersion(FILE* f) const;

	// case senstive
	// Returns
	//   0 if not found
	const CConsoleCommand* FindConsoleCommand(tukk szName) const;

	// ----------------------------------------------

	//
	tukk GetFolderName() const;
	//
	tukk GetFileExtension() const;

	// ----------------------------------------------

	CXConsole& m_rParent;
	EWorkMode  m_eWorkMode;                     // during Work() thise true:HTML and false:Confluence at some point
};

#endif  // DRX_PLATFORM_WINDOWS
