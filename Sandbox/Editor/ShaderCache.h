// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ShaderCache_h__
#define __ShaderCache_h__
#pragma once

//////////////////////////////////////////////////////////////////////////
class CLevelShaderCache
{
public:
	CLevelShaderCache()
	{
		m_bModified = false;
	}
	bool Load(tukk filename);
	bool LoadBuffer(const string& textBuffer, bool bClearOld = true);
	bool SaveBuffer(string& textBuffer);
	bool Save();
	bool Reload();
	void Clear();

	void Update();
	void ActivateShaders();

private:
	//////////////////////////////////////////////////////////////////////////
	bool    m_bModified;
	string m_filename;
	typedef std::set<string> Entries;
	Entries m_entries;
};

#endif // __ShaderCache_h__

