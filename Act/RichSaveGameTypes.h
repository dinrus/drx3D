// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RICH_SAVE_GAME_TYPES_H__
#define __RICH_SAVE_GAME_TYPES_H__

#pragma pack(push)
#pragma pack(1)

struct DrxBitmapFileHeader         /**** BMP file header structure ****/
{
	u16 bfType;           /* Magic number for file */
	u32 bfSize;           /* Size of file */
	u16 bfReserved1;      /* Reserved */
	u16 bfReserved2;      /* ... */
	u32 bfOffBits;        /* Offset to bitmap data */

	AUTO_STRUCT_INFO;
};

struct DrxBitmapInfoHeader         /**** BMP file info structure ****/
{
	u32 biSize;           /* Size of info header */
	i32  biWidth;          /* Width of image */
	i32  biHeight;         /* Height of image */
	u16 biPlanes;         /* Number of color planes */
	u16 biBitCount;       /* Number of bits per pixel */
	u32 biCompression;    /* Type of compression to use */
	u32 biSizeImage;      /* Size of image data */
	i32  biXPelsPerMeter;  /* X pixels per meter */
	i32  biYPelsPerMeter;  /* Y pixels per meter */
	u32 biClrUsed;        /* Number of colors used */
	u32 biClrImportant;   /* Number of important colors */

	AUTO_STRUCT_INFO;
};

#pragma pack(pop)

namespace RichSaveGames
{
static i32k RM_MAXLENGTH = 1024;

struct GUID
{
	u32        Data1;
	u16        Data2;
	u16        Data3;
	u8 Data4[8];

	AUTO_STRUCT_INFO;
};

#pragma pack(push)
#pragma pack(1)
struct RICH_GAME_MEDIA_HEADER
{
	u32  dwMagicNumber;
	u32  dwHeaderVersion;
	u32  dwHeaderSize;
	int64   liThumbnailOffset;
	u32  dwThumbnailSize;
	GUID    guidGameId;
	wchar_t szGameName[RM_MAXLENGTH];
	wchar_t szSaveName[RM_MAXLENGTH];
	wchar_t szLevelName[RM_MAXLENGTH];
	wchar_t szComments[RM_MAXLENGTH];

	AUTO_STRUCT_INFO;
};
#pragma pack(pop)
}
#endif
