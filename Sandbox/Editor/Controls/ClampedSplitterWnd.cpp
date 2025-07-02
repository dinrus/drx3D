// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"
#include "ClampedSplitterWnd.h"

IMPLEMENT_DYNAMIC(CClampedSplitterWnd, CXTSplitterWnd)

BEGIN_MESSAGE_MAP(CClampedSplitterWnd, CXTSplitterWnd)
ON_WM_SIZE()
END_MESSAGE_MAP()

void CClampedSplitterWnd::TrackRowSize(i32 y, i32 row)
{
	// Overriding base behaviour from CSplitterWnd
	// where if an entry is too small it disappears
	// we want it to instead not get that small

	ClampMovement(y, row, m_nRows, m_pRowInfo);
}

void CClampedSplitterWnd::TrackColumnSize(i32 x, i32 col)
{
	// Overriding base behaviour from CSplitterWnd
	// where if an entry is too small it disappears
	// we want it to instead not get that small

	ClampMovement(x, col, m_nCols, m_pColInfo);
}

void CClampedSplitterWnd::OnSize(UINT nType, i32 cx, i32 cy)
{
	__super::OnSize(nType, cx, cy);

	RequestShowHiddenPanel(m_nCols, m_pColInfo);
	RequestShowHiddenPanel(m_nRows, m_pRowInfo);
}

void CClampedSplitterWnd::ClampMovement(i32k offset, i32k index, i32k numEntries, CRowColInfo* dataSet)
{
	// Asserts to mimic behaviour from CSplitterWnd
	ASSERT_VALID(this);
	ASSERT(numEntries > 1);

	// Determine change in splitter position
	static i32k SPLITTER_WIDTH = 6;
	static i32k HALF_SPLITTER_WIDTH = 3;
	i32 prevPos = -HALF_SPLITTER_WIDTH; // First panel only has one half a splitter in it, so negate half
	for (i32 i = 0; i <= index; i++)
	{
		prevPos += dataSet[i].nCurSize + SPLITTER_WIDTH; // Add the splitter to each
	}
	prevPos -= HALF_SPLITTER_WIDTH; // Last panel only has only half a splitter in it, so negate half
	i32 splitterPosChange = offset - prevPos;

	if (splitterPosChange > 0)
	{
		// Splitter moving right/down
		i32 indexID = numEntries - 2;
		i32 requestedShrink = splitterPosChange;
		i32 accumulatedMovement = 0;

		while (indexID >= index && requestedShrink > 0)
		{
			RequestPanelShrink(indexID + 1, requestedShrink, accumulatedMovement, dataSet);
			indexID--;
		}

		// Add the accumulated movement to the panel to the left of or above our selected splitter
		dataSet[index].nIdealSize = dataSet[index].nCurSize + accumulatedMovement;
	}
	else if (splitterPosChange < 0)
	{
		// Splitter moving left/up
		i32 indexID = 0;
		i32 requestedShrink = splitterPosChange * -1;
		i32 accumulatedMovement = 0;

		while (indexID <= index && requestedShrink > 0)
		{
			RequestPanelShrink(indexID, requestedShrink, accumulatedMovement, dataSet);
			indexID++;
		}

		// Add the accumulated movement to the panel to the right of or below our selected splitter
		dataSet[index + 1].nIdealSize = dataSet[index + 1].nCurSize + accumulatedMovement;
	}
}

// Attempts to shrink a panel, will not shrink smaller than minimum size
// requestedShrink: The amount we want to reduce from the panel
// accumulatedMovement: We add the amount we reduced from this panel to accumulatedMovement
void CClampedSplitterWnd::RequestPanelShrink(i32k index, i32& requestedShrink, i32& accumulatedMovement, CRowColInfo* dataSet)
{
	// Determine how much we can shrink by, but no more than requested
	i32 availableChange = MIN(dataSet[index].nCurSize - dataSet[index].nMinSize, requestedShrink);

	// Deduct from amount we are going to shrink, and add to accumulated movement
	requestedShrink -= availableChange;
	accumulatedMovement += availableChange;

	// Apply the change to this panel
	dataSet[index].nIdealSize = dataSet[index].nCurSize - availableChange;
}

// Attempts to resize all panels that wouldn't be visible anymore after the splitter has been resized
// maxData: The maximum number of rows or cols of dataSet
// dataSet: The dataset(rows / cols)
void CClampedSplitterWnd::RequestShowHiddenPanel(i32k maxData, CRowColInfo* dataSet)
{
	// The amount of pixel when a resizing should occure
	static i32k moveSize = 25;

	if (maxData <= 1 || !dataSet)
	{
		return;
	}

	for (i32 i = 0; i < maxData; ++i)
	{
		if (dataSet[i].nCurSize < moveSize)
		{
			if (i == 0) // Pick right neighbour (panel)
			{
				if (dataSet[i + 1].nCurSize > moveSize)
				{
					ResizePanels(moveSize, dataSet[i + 1], dataSet[i]);
				}
			}
			else // Pick left neighbour (panel)
			{
				if (dataSet[i - 1].nCurSize > moveSize)
				{
					ResizePanels(moveSize, dataSet[i - 1], dataSet[i]);
				}
			}
		}
	}
}

// Resizes two panels, makes one panel smaller and makes the other bigger by the same amount
// resizeAmount: The amount by which the two panels should be sized
// makeSmaller: The panel to become smaller
// makeBigger: The panel to become bigger
void CClampedSplitterWnd::ResizePanels(i32k resizeAmount, CRowColInfo& makeSmaller, CRowColInfo& makeBigger)
{
	makeSmaller.nCurSize -= resizeAmount;
	makeSmaller.nIdealSize = makeSmaller.nCurSize;

	makeBigger.nCurSize += resizeAmount;
	makeBigger.nIdealSize = makeBigger.nCurSize;
}

