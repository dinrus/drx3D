// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PhonemesCtrl_h__
#define __PhonemesCtrl_h__
#pragma once

#include <drx3D/CoreX/Math/ISplines.h>

struct IPhonemeLibrary;
struct SPhonemeInfo;

// Notify event sent when phoneme is being modified.
#define PHONEMECTRLN_CHANGE        (0x0001)
// Notify event sent just before when phoneme is modified.
#define PHONEMECTRLN_BEFORE_CHANGE (0x0002)
// Notify event sent just before when phoneme is modified.
#define PHONEMECTRLN_PREVIEW       (0x0003)
// The user has selected "clear all phonemes" from context menu
#define PHONEMECTRLN_CLEAR         (0x0004)
// The user wants to bake out the phonemes to curves.
#define PHONEMECTRLN_BAKE          (0x0005)

class IPhonemeUndoContext
{
public:
	//////////////////////////////////////////////////////////////////////////
	struct Phoneme
	{
		char sPhoneme[4]; // Phoneme name.

		// Start time and the length of the phoneme.
		i32   time0;
		i32   time1;
		float intensity;

		//////////////////////////////////////////////////////////////////////////
		bool bSelected;
		bool bActive;

		Phoneme() { bSelected = false; bActive = false; * sPhoneme = 0; time0 = time1 = 0; intensity = 1.0f; }
	};
	//////////////////////////////////////////////////////////////////////////

	virtual void SetPhonemes(const std::vector<std::vector<Phoneme>>& phonemes) = 0;
	virtual void GetPhonemes(std::vector<std::vector<Phoneme>>& phonemes) = 0;
	virtual void OnPhonemeChangesUnOrRedone() = 0;
};

//////////////////////////////////////////////////////////////////////////
// Spline control.
//////////////////////////////////////////////////////////////////////////
class CPhonemesCtrl : public CWnd, public IPhonemeUndoContext
{
public:
	DECLARE_DYNAMIC(CPhonemesCtrl)

	//////////////////////////////////////////////////////////////////////////
	struct Word
	{
		CString text; // word itself.
		// Start time and the length of the word.
		i32     time0;
		i32     time1;

		//////////////////////////////////////////////////////////////////////////
		bool bSelected;
		bool bActive;

		Word() { bSelected = false; bActive = false; time0 = time1 = 0; }
	};
	//////////////////////////////////////////////////////////////////////////

	CPhonemesCtrl();
	virtual ~CPhonemesCtrl();

	BOOL Create(DWORD dwStyle, const CRect& rc, CWnd* pParentWnd, UINT nID);

	//Phoneme.
	void        InsertPhoneme(CPoint point, i32 phonemeId);
	void        RenamePhoneme(i32 sentenceIndex, i32 index, i32 phonemeId);
	void        TrackPoint(CPoint point);
	void        RemovePhoneme(i32 sentenceIndex, i32 phonemeId);
	void        ClearAllPhonemes();
	void        BakeLipsynchCurves();
	void        StartTracking();
	void        StopTracking();

	void        SetTimeMarker(float fTime);

	tukk GetMouseOverPhoneme();

	i32         AddSentence();
	void        DeleteSentence(i32 sentenceIndex);
	i32         GetSentenceCount();
	void        SetSentenceStartTime(i32 sentenceIndex, float startTime);
	void        SetSentenceEndTime(i32 sentenceIndex, float endTime);

	i32         GetPhonemeCount(i32 sentenceIndex)   { return (i32)m_sentences[sentenceIndex].phonemes.size(); }
	Phoneme&    GetPhoneme(i32 sentenceIndex, i32 i) { return m_sentences[sentenceIndex].phonemes[i]; }
	void        RemoveAllPhonemes(i32 sentenceIndex) { m_sentences[sentenceIndex].phonemes.clear(); };
	void        AddPhoneme(i32 sentenceIndex, Phoneme& ph);

	void        AddWord(i32 sentenceIndex, const Word& w);
	i32         GetWordCount(i32 sentenceIndex)   { return m_sentences[sentenceIndex].words.size(); }
	Word&       GetWord(i32 sentenceIndex, i32 i) { return m_sentences[sentenceIndex].words[i]; }
	void        RemoveAllWords(i32 sentenceIndex) { m_sentences[sentenceIndex].words.clear(); }

	void        UpdatePhonemeLengths();

	void        UpdateCurrentActivePhoneme();

	void        SetZoom(float fZoom);
	void        SetScrollOffset(float fOrigin);
	float       GetZoom() const         { return m_fZoom; }
	float       GetScrollOffset() const { return m_fOrigin; }

	// IPhonemeUndoContext
	virtual void SetPhonemes(const std::vector<std::vector<Phoneme>>& phonemes);
	virtual void GetPhonemes(std::vector<std::vector<Phoneme>>& phonemes);
	virtual void OnPhonemeChangesUnOrRedone();

protected:
	enum EditMode
	{
		NothingMode = 0,
		SelectMode,
		TrackingMode,
	};
	enum EHitCode
	{
		HIT_NOTHING,
		HIT_PHONEME,
		HIT_EDGE_LEFT,
		HIT_EDGE_RIGHT
	};

	DECLARE_MESSAGE_MAP()

	virtual void PostNcDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, i32 cx, i32 cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);

	// Drawing functions
	void                DrawPhonemes(CDC* pDC);
	void                DrawWords(CDC* pDC);
	void                DrawTimeMarker(CDC* pDC);

	EHitCode            HitTest(CPoint point);

	i32                 TimeToClient(i32 time);
	i32                 ClientToTime(i32 x);

	void                ClearSelection();

	void                SendNotifyEvent(i32 nEvent);

	void                AddPhonemes(CMenu& menu, i32 nBaseId);
	IPhonemeLibrary*    GetPhonemeLib();

	void                SetPhonemeTime(i32 sentenceIndex, i32 index, i32 t0, i32 t1);
	std::pair<i32, i32> PhonemeFromTime(i32 time);
	std::pair<i32, i32> WordFromTime(i32 time);

	void                StoreUndo();

private:
	struct Sentence
	{
		Sentence() : startTime(0.0f), endTime(0.0f) {}
		float                startTime;
		float                endTime;
		std::vector<Phoneme> phonemes;
		std::vector<Word>    words;
	};

	std::vector<Sentence> m_sentences;

	CRect                 m_rcClipRect;
	CRect                 m_rcPhonemes;
	CRect                 m_rcWords;
	CRect                 m_rcClient;
	CRect                 m_TimeUpdateRect;

	CPoint                m_LButtonDown;
	CPoint                m_hitPoint;
	EHitCode              m_hitCode;
	i32                   m_nHitPhoneme;
	i32                   m_nHitSentence;

	string                m_mouseOverPhoneme;

	float                 m_fTimeMarker;

	EditMode              m_editMode;

	float                 m_fZoom;
	float                 m_fOrigin;

	CToolTipCtrl          m_tooltip;
	CBitmap               m_offscreenBitmap;

	std::set<HMENU>       m_phonemePopupMenus;
};

#endif // __PhonemesCtrl_h__

