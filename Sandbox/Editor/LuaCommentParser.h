// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __LUACOMMENTPARSER_H__
#define __LUACOMMENTPARSER_H__

struct LuaTable
{
	LuaTable(i32 open, i32 close, string name)
	{
		m_Name = name;
		m_OpenBracePos = open;
		m_CloseBracePos = close;
	}

	void AddChild(LuaTable* table)
	{
		m_ChildTables.push_back(table);
	}

	LuaTable* FindChildByName(tukk name)
	{
		for (i32 i = 0; i < m_ChildTables.size(); i++)
		{
			if (m_ChildTables[i]->m_Name == name)
			{
				return m_ChildTables[i];
			}
		}
		return NULL;
	}

	~LuaTable()
	{
		for (i32 i = 0; i < m_ChildTables.size(); i++)
		{
			delete m_ChildTables[i];
		}
		m_ChildTables.resize(0);
	}

	string                 m_Name;
	i32                    m_OpenBracePos;
	i32                    m_CloseBracePos;
	std::vector<LuaTable*> m_ChildTables;
};

class LuaCommentParser
{
public:
	bool                     OpenScriptFile(tukk path);
	bool                     ParseComment(tukk tablePath, tukk varName, float* minVal, float* maxVal, float* stepVal, string* desc);
	static LuaCommentParser* GetInstance();
	~LuaCommentParser(void);
	void                     CloseScriptFile();
protected:
	i32                      FindTables(LuaTable* parentTable = NULL, i32 offset = 0);
	string                   FindTableNameForBracket(i32 bracketPos);
	bool                     IsAlphaNumericalChar(char c);
	i32                      GetVarInTable(tukk tablePath, tukk varName);
	i32                      FindStringInFileSkippingComments(string searchString, i32 offset = 0);
	i32                      FindWordInFileSkippingComments(string searchString, i32 offset = 0);
protected:
	FILE* m_pFile;
	tuk m_AllText;
	tuk m_MovingText;
private:
	LuaCommentParser();
	LuaTable* m_RootTable;
};

#endif __LUACOMMENTPARSER_H__

