// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/IFacialAnimation.h>
#include <drx3D/CoreX/Containers/VectorSet.h>

class CFacialAnimationContext;

//////////////////////////////////////////////////////////////////////////
struct SPhoneme
{
	wchar_t codeIPA;     // IPA (International Phonetic Alphabet) code.
	char    ASCII[4];    // ASCII name for this phoneme (SAMPA for English).
	string  description; // Phoneme description.
	void    GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(codeIPA);
		pSizer->AddObject(ASCII);
		pSizer->AddObject(description);
	}
};

//////////////////////////////////////////////////////////////////////////
class CPhonemesLibrary : public IPhonemeLibrary
{
public:
	CPhonemesLibrary();

	//////////////////////////////////////////////////////////////////////////
	// IPhonemeLibrary
	//////////////////////////////////////////////////////////////////////////
	virtual i32  GetPhonemeCount() const;
	virtual bool GetPhonemeInfo(i32 nIndex, SPhonemeInfo& phoneme);
	virtual i32  FindPhonemeByName(tukk sPhonemeName);
	//////////////////////////////////////////////////////////////////////////

	SPhoneme& GetPhoneme(i32 nIndex);
	void      LoadPhonemes(tukk filename);

	void      GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_phonemes);
	}

private:
	std::vector<SPhoneme> m_phonemes;
};

//////////////////////////////////////////////////////////////////////////
class CFacialSentence : public IFacialSentence, public _reference_target_t
{
public:
	struct SFaceIdentifierHandleLess : public std::binary_function<CFaceIdentifierHandle, CFaceIdentifierHandle, bool>
	{
		bool operator()(const CFaceIdentifierHandle& left, const CFaceIdentifierHandle& right) const
		{
			return left.GetCRC32() < right.GetCRC32();
		};
	};
	typedef VectorSet<CFaceIdentifierHandle, SFaceIdentifierHandleLess> OverridesMap;

	CFacialSentence();

	//////////////////////////////////////////////////////////////////////////
	// IFacialSentence
	//////////////////////////////////////////////////////////////////////////
	IPhonemeLibrary*    GetPhonemeLib();
	virtual void        SetText(tukk text) { m_text = text; };
	virtual tukk GetText()                 { return m_text; };
	virtual void        ClearAllPhonemes()        { m_phonemes.clear(); m_words.clear(); ++m_nValidateID; };
	virtual i32         GetPhonemeCount()         { return (i32)m_phonemes.size(); };
	virtual bool        GetPhoneme(i32 index, Phoneme& ph);
	virtual i32         AddPhoneme(const Phoneme& ph);

	virtual void        ClearAllWords() { m_words.clear(); ++m_nValidateID; };
	virtual i32         GetWordCount()  { return (i32)m_words.size(); };
	virtual bool        GetWord(i32 index, Word& wrd);
	virtual void        AddWord(const Word& wrd);

	virtual i32         Evaluate(float fTime, float fInputPhonemeStrength, i32 maxSamples, ChannelSample* samples);
	//////////////////////////////////////////////////////////////////////////

	i32      GetPhonemeFromTime(i32 timeMs, i32 nFirst = 0);

	bool     GetPhonemeInfo(i32 phonemeId, SPhonemeInfo& phonemeInfo) const;
	void     Serialize(XmlNodeRef& node, bool bLoading);

	Phoneme& GetPhoneme(i32 index) { return m_phonemes[index]; };

	void     Animate(const QuatTS& rAnimLocationNext, CFacialAnimationContext* pAnimContext, float fTime, float fPhonemeStrength, const OverridesMap& overriddenPhonemes);

	i32      GetValidateID() { return m_nValidateID; }

	void     GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(&m_phonemes, sizeof(m_phonemes) + m_phonemes.capacity() * sizeof(Phoneme));
		pSizer->AddObject(&m_words, sizeof(m_words) + m_words.capacity() * sizeof(Word));
		pSizer->AddObject(m_nValidateID);
	}

private:
	struct WordRec
	{
		string               text;
		i32                  startTime;
		i32                  endTime;
		std::vector<Phoneme> phonemes;
		void                 GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(this, sizeof(*this));
			pSizer->AddObject(text);
			pSizer->AddObject(startTime);
			pSizer->AddObject(endTime);
			pSizer->AddObject(&phonemes, phonemes.capacity() * sizeof(Phoneme));
		};
	};
	string               m_text;
	std::vector<Phoneme> m_phonemes;
	std::vector<WordRec> m_words;

	// If this value has changed, then the sentence has changed.
	i32 m_nValidateID;
};
