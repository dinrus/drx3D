// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "ShaderCache.h"
#include "GameEngine.h"

#include <drx3D/CoreX/Renderer/IRenderer.h>

//////////////////////////////////////////////////////////////////////////
bool CLevelShaderCache::Reload()
{
	return Load(m_filename);
}

//////////////////////////////////////////////////////////////////////////
bool CLevelShaderCache::Load(tukk filename)
{
	FILE* f = fopen(filename, "rt");
	if (!f)
		return false;

	i32 nNumLines = 0;
	m_entries.clear();
	m_filename = filename;
	char str[65535];
	while (fgets(str, sizeof(str), f) != NULL)
	{
		if (str[0] == '<')
		{
			m_entries.insert(str);
			nNumLines++;
		}
	}
	fclose(f);
	if (nNumLines == m_entries.size())
		m_bModified = false;
	else
		m_bModified = true;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CLevelShaderCache::LoadBuffer(const string& textBuffer, bool bClearOld)
{
	tukk separators = "\r\n,";
	string resToken;
	i32 curPos = 0;

	i32 nNumLines = 0;
	if (bClearOld)
		m_entries.clear();
	m_filename = "";

	resToken = textBuffer.Tokenize(separators, curPos);
	while (resToken != "")
	{
		if (!resToken.IsEmpty() && resToken[0] == '<')
		{
			m_entries.insert(resToken);
			nNumLines++;
		}

		resToken = textBuffer.Tokenize(separators, curPos);
	}
	if (nNumLines == m_entries.size() && !bClearOld)
		m_bModified = false;
	else
		m_bModified = true;

	i32 numShaders = m_entries.size();
	DrxLog("%d shader combination loaded for level %s", numShaders, (tukk)GetIEditorImpl()->GetGameEngine()->GetLevelPath());

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CLevelShaderCache::Save()
{
	if (m_filename.IsEmpty())
		return false;

	Update();

	FILE* f = fopen(m_filename, "wt");
	if (f)
	{
		for (Entries::iterator it = m_entries.begin(); it != m_entries.end(); ++it)
		{
			tukk str = *it;
			fputs(str, f);
		}
		fclose(f);
	}
	m_bModified = false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CLevelShaderCache::SaveBuffer(string& textBuffer)
{
	Update();

	textBuffer.Preallocate(m_entries.size() * 1024);
	for (Entries::iterator it = m_entries.begin(); it != m_entries.end(); ++it)
	{
		textBuffer += (*it);
		textBuffer += "\n";
	}
	m_bModified = false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CLevelShaderCache::Update()
{
	IRenderer* pRenderer = gEnv->pRenderer;
	{
		string buf;
		tuk str = NULL;
		pRenderer->EF_Query(EFQ_GetShaderCombinations, str);
		if (str)
		{
			buf = str;
			pRenderer->EF_Query(EFQ_DeleteMemoryArrayPtr, str);
		}
		LoadBuffer(buf, true);
	}
}

//////////////////////////////////////////////////////////////////////////
void CLevelShaderCache::Clear()
{
	m_entries.clear();
	m_bModified = true;
}

//////////////////////////////////////////////////////////////////////////
void CLevelShaderCache::ActivateShaders()
{
	bool bPreload = false;
	ICVar* pSysPreload = gEnv->pConsole->GetCVar("sys_preload");
	if (pSysPreload && pSysPreload->GetIVal() != 0)
		bPreload = true;

	if (bPreload)
	{
		string textBuffer;
		textBuffer.Preallocate(m_entries.size() * 1024);
		for (Entries::iterator it = m_entries.begin(); it != m_entries.end(); ++it)
		{
			textBuffer += (*it);
			textBuffer += "\n";
		}
		gEnv->pRenderer->EF_Query(EFQ_SetShaderCombinations, textBuffer);
	}
}

