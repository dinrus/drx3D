// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PhonemeAnalyzer_h__
#define __PhonemeAnalyzer_h__
#pragma once

//////////////////////////////////////////////////////////////////////////
struct ILipSyncPhonemeRecognizer
{
public:
	struct SPhoneme
	{
		enum { MAX_PHONEME_LENGTH = 8 };

		i32   startTime;
		i32   endTime;
		i32   nPhonemeCode;
		char  sPhoneme[MAX_PHONEME_LENGTH];
		float intensity;
	};

	struct SWord
	{
		i32   startTime;
		i32   endTime;
		tuk sWord;
	};

	struct SSentance
	{
		tuk     sSentence;

		i32       nWordCount;
		SWord*    pWords; // Array of words.

		i32       nPhonemeCount;
		SPhoneme* pPhonemes; // Array of phonemes.
	};

	virtual void        Release() = 0;
	virtual bool        RecognizePhonemes(tukk wavfile, tukk text, SSentance** pOutSetence) = 0;
	virtual tukk GetLastError() = 0;
};

//////////////////////////////////////////////////////////////////////////
class CPhonemesAnalyzer
{
public:
	CPhonemesAnalyzer();
	~CPhonemesAnalyzer();

	// Analyze wav file and extract phonemes out of it.
	bool        Analyze(tukk wavfile, tukk text, ILipSyncPhonemeRecognizer::SSentance** pOutSetence);
	tukk GetLastError();

private:
	HMODULE                    m_hDLL;
	ILipSyncPhonemeRecognizer* m_pPhonemeParser;
	string                     m_LastError;
};

#endif // __PhonemeAnalyzer_h__

