// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "LuaCommentParser.h"

//-------------------------------------------------------------------------
LuaCommentParser::LuaCommentParser(void)
{
	m_pFile = NULL;
	m_RootTable = NULL;
	m_AllText = NULL;
}

//-------------------------------------------------------------------------
LuaCommentParser::~LuaCommentParser(void)
{
}

//-------------------------------------------------------------------------
void LuaCommentParser::CloseScriptFile()
{
	if (m_pFile)
	{
		gEnv->pDrxPak->FClose(m_pFile);
		m_pFile = NULL;
	}

	SAFE_DELETE_ARRAY(m_AllText);
	SAFE_DELETE(m_RootTable);
}

//For debugging the table structures
void WriteToFile(FILE* file, LuaTable* table, i32 level = 0)
{
	level++;
	for (i32 i = 0; i < level; i++)
	{
		fwrite("   ", sizeof(char), 1, file);
	}

	fwrite(table->m_Name.c_str(), sizeof(char), table->m_Name.length(), file);
	fwrite("\r", sizeof(char), 1, file);

	for (i32 i = 0; i < table->m_ChildTables.size(); i++)
	{
		LuaTable* child_table = table->m_ChildTables[i];

		WriteToFile(file, child_table, level);
	}
}

//-------------------------------------------------------------------------
bool LuaCommentParser::OpenScriptFile(tukk path)
{
	// Open
	tukk sGameDir = gEnv->pConsole->GetCVar("sys_game_folder")->GetString();
	string full_path = sGameDir;
	full_path.append("/");
	full_path.append(path);

	m_pFile = gEnv->pDrxPak->FOpen(full_path, "rb");

	if (m_pFile)
	{
		// Find file length
		gEnv->pDrxPak->FSeek(m_pFile, 0, SEEK_END);
		i32 nLen = gEnv->pDrxPak->FTell(m_pFile);
		gEnv->pDrxPak->FSeek(m_pFile, 0, SEEK_SET);

		// Read file into char buffer
		m_AllText = new char[nLen + 16];
		gEnv->pDrxPak->FRead(m_AllText, nLen, m_pFile);
		m_AllText[nLen] = 0;

		FindTables();

		//debug
		/*
		   if(m_RootTable && m_RootTable->m_Name.length() < 128)
		   {
		   char buff[256];
		   drx_sprintf(buff, "tableDump%s.txt", m_RootTable->m_Name);
		   FILE* tmpFile = fopen(buff, "wb");
		   WriteToFile(tmpFile, m_RootTable);
		   fclose(tmpFile);
		   }
		 */
	}
	else
	{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------
i32 LuaCommentParser::FindStringInFileSkippingComments(string searchString, i32 offset)
{
	string file = string(m_AllText + offset);
	string fullfile = string(m_AllText);
	i32 posMultiComment, posSingleComment, posSearchString;
	posSearchString = 1;
	i32 posInFile = offset;

	while (posSearchString > 0)
	{
		posMultiComment = file.find("--[[");
		posSingleComment = file.find("--");
		posSearchString = file.find(searchString);

		if (posSearchString < 0)
			return -1;

		if ((posMultiComment >= 0 && posMultiComment < posSearchString) || (posSingleComment >= 0 && posSingleComment < posSearchString))
		{
			//Multi - Jump over comment and search again
			if (posMultiComment >= 0 && posMultiComment <= posSingleComment)
			{
				posInFile = posInFile + posMultiComment + 4;
				file = fullfile.Mid(posInFile, fullfile.length());
				posInFile = posInFile + file.find("]]") + 2;
				file = fullfile.Mid(posInFile, fullfile.length());
			}
			else if (posSingleComment >= 0) //single - go to next line and search again
			{
				posInFile = posInFile + posSingleComment + 2;
				file = fullfile.Mid(posInFile, fullfile.length());
				posInFile = posInFile + file.find('\r') + 1;
				file = fullfile.Mid(posInFile, fullfile.length());
			}
		}
		else
		{
			//found
			return posInFile + posSearchString;
		}
	}

	//nothing found
	return -1;
}

//Recursively finds all tables and creates a tree out of it
//-------------------------------------------------------------------------
i32 LuaCommentParser::FindTables(LuaTable* parentTable, i32 offset)
{
	i32 posInFile = offset;
	string fullfile = string(m_AllText);
	string file = string(m_AllText + offset);
	i32 posOpenBracket;
	posOpenBracket = 1;
	i32 i = 0;

	posOpenBracket = FindStringInFileSkippingComments("{", posInFile);

	//table found
	i32 breakhere = 0;
	file = string(m_AllText + posOpenBracket + 1);
	posInFile = posOpenBracket + 1;
	string tableName = FindTableNameForBracket(posInFile - 1);

	//if called with NULL, make this our root table
	LuaTable* table = new LuaTable(posInFile, posInFile, tableName);
	if (parentTable)
		parentTable->AddChild(table);
	else
		m_RootTable = table;

	//find closing bracket
	i32 nextOpenBrace = FindStringInFileSkippingComments("{", posInFile);
	i32 nextCloseBrace = FindStringInFileSkippingComments("}", posInFile);
	i32 filePointer = posInFile;

	i32 numberOpenBraces = 1;
	i32 numberClosedBraces = 0;
	while (numberOpenBraces != numberClosedBraces)
	{
		//its a child table, skip
		if (nextOpenBrace >= 0 && nextOpenBrace < nextCloseBrace)
		{
			numberOpenBraces++;
			filePointer = nextOpenBrace + 1;
			filePointer = FindTables(table, filePointer - 1);
		}
		else
		{
			//found my closing brace
			numberClosedBraces++;
			filePointer = nextCloseBrace + 1;
			table->m_CloseBracePos = filePointer;
			return filePointer;
		}

		nextOpenBrace = FindStringInFileSkippingComments("{", filePointer);
		nextCloseBrace = FindStringInFileSkippingComments("}", filePointer);
	}

	return -1;
};

//-------------------------------------------------------------------------
string LuaCommentParser::FindTableNameForBracket(i32 bracketPos)
{
	// Find '=' sign and the word before that if not commented
	bool found = false;
	tukk pBracket = m_AllText + bracketPos;
	string tableName;
	i32 i = bracketPos;
	bool inMultiComment = false;
	while (!found && i >= 0)
	{
		if (inMultiComment)
		{
			if (m_AllText[i] == '[' && m_AllText[i - 1] == '[' && m_AllText[i - 2] == '-' && m_AllText[i - 3] == '-')
			{
				inMultiComment = false;
				i -= 4;
				continue;
			}
		}
		else
		{

			if (m_AllText[i] == '\r' || m_AllText[i] == ',')
			{
				if (tableName.length() > 0)
				{
					found = true;
					break;
				}
			}

			//we were in a comment, erase, find next word
			if (m_AllText[i] == '-' && m_AllText[i - 1] == '-')
			{
				tableName = "";
				i -= 2;
				continue;
			}

			//we found the end of a multicomment, clear and wait for opening-multicomment before continuing search
			if (m_AllText[i] == ']' && m_AllText[i - 1] == ']')
			{
				tableName = "";
				i -= 2;
				inMultiComment = true;
				continue;
			}

			if (IsAlphaNumericalChar(m_AllText[i]))
			{
				tableName.insert(0, m_AllText[i]);
			}
			else
			{
				//tableName.empty();
			}
		}
		i--;
	}

	return tableName;
}

//-------------------------------------------------------------------------
i32 LuaCommentParser::GetVarInTable(tukk tablePath, tukk varName)
{
	string sTablePath = tablePath;
	i32 nextTablePos = sTablePath.find(".");
	string remaining = sTablePath;

	LuaTable* table = m_RootTable;

	//Find childtable if needed
	while (nextTablePos > 0)
	{
		string currentTable = remaining;
		remaining = remaining.Mid(nextTablePos + 1, sTablePath.length());
		currentTable.resize(nextTablePos);

		if (!table) break;

		table = table->FindChildByName(currentTable);

		nextTablePos = remaining.find(".");
	}

	if (!table) return -1;

	//Find table
	table = table->FindChildByName(remaining);

	if (!table) return -1;

	//make sure we got the correct work and not a partial result of a different variable
	i32 variablePos = FindWordInFileSkippingComments(varName, table->m_OpenBracePos);

	bool done = false;

	while (!done && variablePos >= 0)
	{
		done = true;
		for (i32 i = 0; i < table->m_ChildTables.size(); i++)
		{
			//if it is inside childtable, find next one
			if (table && table->m_ChildTables[i]->m_CloseBracePos > variablePos && table->m_ChildTables[i]->m_OpenBracePos < variablePos)
			{
				variablePos = FindWordInFileSkippingComments(varName, variablePos + strlen(varName));
				done = false;
				break;
			}
		}
	}

	return variablePos;
}

//-------------------------------------------------------------------------
i32 LuaCommentParser::FindWordInFileSkippingComments(string searchString, i32 offset)
{
	i32 variablePos = FindStringInFileSkippingComments(searchString, offset);
	while (variablePos >= 0 && (IsAlphaNumericalChar(m_AllText[variablePos + searchString.length()]) || IsAlphaNumericalChar(m_AllText[variablePos - 1])))
	{
		variablePos = FindStringInFileSkippingComments(searchString, variablePos + strlen(searchString));
	}
	return variablePos;
}

//-------------------------------------------------------------------------
bool LuaCommentParser::ParseComment(tukk tablePath, tukk varName, float* minVal, float* maxVal, float* stepVal, string* desc)
{
	i32 pos = GetVarInTable(tablePath, varName);

	if (pos > 0)
	{
		string line = m_AllText + pos;

		i32 endLine = line.find('\n');
		i32 posComment = line.find("--");
		posComment += 2;                                 //find comment and skip comment tags
		char c;
		if (posComment < endLine)
		{
			while (c = line[posComment])
			{
				if (c == '\n')
				{
					//end of line, comment was not parseable
					return false;
				}
				if (c == '[')
				{
					//extract all params separated by comma (bail out if not properly parsable)

					string sMinval = line.c_str() + posComment + 1;
					string::size_type div1 = sMinval.find(",");
					if (div1 == string::npos)
						return false;

					string sMaxval = sMinval.c_str() + div1 + 1;
					string::size_type div2 = sMaxval.find(",");
					if (div2 == string::npos)
						return false;

					string sStepVal = sMaxval.c_str() + div2 + 1;
					string::size_type div3 = sStepVal.find(",");
					if (div3 == string::npos)
						return false;

					string sComment = sStepVal.c_str() + div3 + 1;

					string::size_type firstQuote = sComment.find('"');
					if (firstQuote == string::npos)
						return false;

					string::size_type lastQuote = sComment.find('"', firstQuote + 1);
					if (lastQuote == string::npos)
						return false;

					string sDesc = sComment.substr(firstQuote + 1, lastQuote - firstQuote - 1);

					//cut off rest of line
					sMinval.resize(div1);
					sMaxval.resize(div2);
					sStepVal.resize(div3);

					//assign
					*minVal = atof(sMinval);
					*maxVal = atof(sMaxval);
					*stepVal = atof(sStepVal);

					//handle desc
					*desc = sDesc;

					break;
				}

				posComment++;
			}
		}
		else
		{
			return false; // comment not found on same line
		}
	}
	else
	{
		return false; // var not found
	}

	return true;
}

//-------------------------------------------------------------------------
LuaCommentParser* LuaCommentParser::GetInstance()
{
	static LuaCommentParser* m_Instance;
	if (!m_Instance)
	{
		m_Instance = new LuaCommentParser();
	}

	return m_Instance;
}

//-------------------------------------------------------------------------
bool LuaCommentParser::IsAlphaNumericalChar(char c)
{
	bool isNumber = false;
	bool isLowerCase = false;
	bool isUpperCase = false;
	isNumber = (i32)c >= 48 && (i32)c <= 57;
	isLowerCase = (i32)c >= 97 && (i32)c <= 122;
	isUpperCase = (i32)c >= 65 && (i32)c <= 90;

	return isNumber || isLowerCase || isUpperCase || (i32)c == 95;
}

