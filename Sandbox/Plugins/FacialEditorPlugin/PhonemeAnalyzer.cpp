// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "PhonemeAnalyzer.h"

#include <drx3D/CoreX/Platform/DrxLibrary.h>

typedef ILipSyncPhonemeRecognizer* (* CreatePhonemeParserFunc)();

//////////////////////////////////////////////////////////////////////////
CPhonemesAnalyzer::CPhonemesAnalyzer()
{
	m_hDLL = 0;
	m_pPhonemeParser = 0;
}

//////////////////////////////////////////////////////////////////////////
CPhonemesAnalyzer::~CPhonemesAnalyzer()
{
	if (m_pPhonemeParser)
		m_pPhonemeParser->Release();
	if (m_hDLL)
		FreeLibrary(m_hDLL);
}

//////////////////////////////////////////////////////////////////////////
bool CPhonemesAnalyzer::Analyze(tukk wavfile, tukk text, ILipSyncPhonemeRecognizer::SSentance** pOutSetence)
{
	if (m_pPhonemeParser)
		m_pPhonemeParser->Release();

	CString strText = text;
	strText.Replace('\r', ' ');
	strText.Replace('\n', ' ');
	strText.Replace('\t', ' ');

	bool bRes = false;
	WCHAR szCurDir[1024];
	GetCurrentDirectoryW(sizeof(szCurDir), szCurDir);
	SetCurrentDirectoryW(L"Editor\\Plugins\\LipSync\\Annosoft");

	m_hDLL = DrxLoadLibrary("LipSync_Annosoft.dll");
	if (m_hDLL)
	{
		CreatePhonemeParserFunc func = (CreatePhonemeParserFunc)::GetProcAddress(m_hDLL, "CreatePhonemeParser");
		if (func)
		{
			m_pPhonemeParser = func();

			// Find the full path.
			CStringW currentDirectory = szCurDir;
			if (!(currentDirectory.GetLength() > 0 && (currentDirectory[currentDirectory.GetLength() - 1] == '\\' || currentDirectory[currentDirectory.GetLength() - 1] == '/')))
				currentDirectory.Append(L"\\");
			CStringW fullPath = currentDirectory + wavfile;
			char fullPathASCIIBuffer[2048] = "";
			WideCharToMultiByte(CP_ACP, 0, fullPath.GetString(), fullPath.GetLength(), fullPathASCIIBuffer, sizeof(fullPathASCIIBuffer) / sizeof(fullPathASCIIBuffer[0]), 0, 0);

			bRes = m_pPhonemeParser->RecognizePhonemes(fullPathASCIIBuffer, strText, pOutSetence);
			m_LastError = m_pPhonemeParser->GetLastError();
		}
	}
	// Restore current directory.
	SetCurrentDirectoryW(szCurDir);
	return bRes;
}

//////////////////////////////////////////////////////////////////////////
tukk CPhonemesAnalyzer::GetLastError()
{
	return m_LastError;
}

