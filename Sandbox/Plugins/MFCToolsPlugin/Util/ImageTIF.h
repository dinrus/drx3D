// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __imagetif_h__
#define __imagetif_h__

#if _MSC_VER > 1000
	#pragma once
#endif

class PLUGIN_API CImageTIF
{
public:
	bool               Load(const string& fileName, CImageEx& outImage);
	bool               SaveRAW(const string& fileName, ukk pData, i32 width, i32 height, i32 bytesPerChannel, i32 numChannels, bool bFloat, tukk preset);
	static tukk GetPreset(const string& fileName);
};

#endif // __imagetif_h__

