// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __sequencersplitter_h__
#define __sequencersplitter_h__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "..\Controls\ClampedSplitterWnd.h"

// CSequencerSplitter

class CSequencerSplitter : public CClampedSplitterWnd
{
	DECLARE_DYNAMIC(CSequencerSplitter)

	virtual CWnd * GetActivePane(i32* pRow = NULL, i32* pCol = NULL)
	{
		return GetFocus();
	}

	void SetPane(i32 row, i32 col, CWnd* pWnd, SIZE sizeInit);
	// Ovveride this for flat look.
	void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rectArg);

public:
	CSequencerSplitter();
	virtual ~CSequencerSplitter();

protected:
	DECLARE_MESSAGE_MAP()
};

#endif // __sequencersplitter_h__

