// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "IPlugin.h"

class CXTPCommandBars;
class CXTPShortcutManager;
struct IVariable;

/*! Collection of Utility MFC functions.
*/
struct PLUGIN_API CMFCUtils
{
	//////////////////////////////////////////////////////////////////////////
	// Load true color image into image list.
	static BOOL LoadTrueColorImageList(CImageList& imageList, UINT nIDResource, i32 nIconWidth, COLORREF colMaskColor);

	//////////////////////////////////////////////////////////////////////////
	static void TransparentBlt(HDC hdcDest, i32 nXDest, i32 nYDest, i32 nWidth, i32 nHeight, HBITMAP hBitmap, i32 nXSrc, i32 nYSrc,
		COLORREF colorTransparent);

	static void LoadShortcuts(CXTPCommandBars* pCommandBars, UINT nMenuIDResource, tukk pSectionNameForLoading);
	static void ShowShortcutsCustomizeDlg(CXTPCommandBars* pCommandBars, UINT nMenuIDResource, tukk pSectionNameForSaving);
	static void ExportShortcuts(CXTPShortcutManager* pShortcutMgr);
	static void ImportShortcuts(CXTPShortcutManager* pShortcutMgr, tukk pSectionNameForSaving);


	static IVariable* GetChildVar(const IVariable* array, tukk name, bool recursive = false);


	//////////////////////////////////////////////////////////////////////////
	// Color utils
	//////////////////////////////////////////////////////////////////////////

	inline static Vec3 Rgb2Vec(COLORREF color)
	{
		return Vec3(GetRValue(color) / 255.0f, GetGValue(color) / 255.0f, GetBValue(color) / 255.0f);
	}

	inline static i32 Vec2Rgb(const Vec3& color)
	{
		return RGB(color.x * 255, color.y * 255, color.z * 255);
	}

	static COLORREF ColorLinearToGamma(ColorF col);
	static ColorF   ColorGammaToLinear(COLORREF col);

	//////////////////////////////////////////////////////////////////////////

	//! Converts a CPoint to a QPoint, using local space of the provided widget.
	static QPoint CPointToQPointLocal(const QWidget* widget, const CPoint& point);

	//////////////////////////////////////////////////////////////////////////

	static bool ExecuteConsoleApp(const string& CommandLine, string& OutputText, bool bNoTimeOut = false, bool bShowWindow = false);
};

//Do not use anymore, only kept for compatibility with old MFC based tools
namespace Deprecated
{
	inline bool CheckVirtualKey(i32 virtualKey)
	{
		GetAsyncKeyState(virtualKey);
		if (GetAsyncKeyState(virtualKey) & (1 << 15))
			return true;
		return false;
	}
}
