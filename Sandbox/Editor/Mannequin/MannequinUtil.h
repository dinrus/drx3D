// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once


namespace MannequinDragDropHelpers
{
	inline UINT GetAnimationNameClipboardFormat()
	{
		static UINT nAnimationNameClipboardFormat = RegisterClipboardFormat("AnimationBrowserCopy");
		return nAnimationNameClipboardFormat;
	}

	inline UINT GetFragmentClipboardFormat()
	{
		static UINT nFragmentClipboardFormat = RegisterClipboardFormat("PreviewFragmentProperties");
		return nFragmentClipboardFormat;
	}
}
