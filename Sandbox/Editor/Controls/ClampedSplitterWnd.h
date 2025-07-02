// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __clampedsplitterwnd_h__
#define __clampedsplitterwnd_h__

#if _MSC_VER > 1000
	#pragma once
#endif

// CClampedSplitterWnd

class CClampedSplitterWnd : public CXTSplitterWnd
{
	DECLARE_DYNAMIC(CClampedSplitterWnd)

public:
	CClampedSplitterWnd() {};
	virtual ~CClampedSplitterWnd() {};

protected:
	DECLARE_MESSAGE_MAP()

	// Overrides for standard CSplitterWnd behaviour
	virtual void TrackRowSize(i32 y, i32 row);
	virtual void TrackColumnSize(i32 x, i32 col);
	virtual void OnSize(UINT nType, i32 cx, i32 cy);

private:
	// Attempts to resize panels but will not allow resizing smaller than min. Will bunch up multiple panels to try and meet resize requirements
	// offset: The position of the splitter relative to the entry at index
	// index: The index of the dataset which is to the left of the splitter
	// numEntries: The number of entries in this dataset
	// dataSet: The dataset (rows / cols)
	void ClampMovement(i32k offset, i32k index, i32k numEntries, CRowColInfo* dataSet);

	// Attempts to shrink a panel, will not shrink smaller than minimum size
	// requestedShrink: The amount we want to reduce from the panel
	// accumulatedMovement: We add the amount we reduced from this panel to accumulatedMovement
	void RequestPanelShrink(i32k index, i32& requestedShrink, i32& accumulatedMovement, CRowColInfo* dataSet);

	// Attempts to resize all panels that wouldn't be visible anymore after the splitter has been resized
	// maxData: The maximum number of rows or cols of dataSet
	// dataSet: The dataset(rows / cols)
	void RequestShowHiddenPanel(i32k maxRowsCols, CRowColInfo* dataSet);

	// Resizes two panels, makes one panel smaller and makes the other bigger by the same amount
	// resizeAmount: The amount by which the two panels should be sized
	// makeSmaller: The panel to become smaller
	// makeBigger: The panel to become bigger
	void ResizePanels(i32k resizeAmount, CRowColInfo& makeSmaller, CRowColInfo& makeBigger);
};

#endif // __clampedsplitterwnd_h__

