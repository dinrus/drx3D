#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <drxtypes.h>
#include <fstream>

#pragma pack(push, 1)
struct TGA_Header
{
	char idlength;
	char colormaptype;
	char datatypecode;
	short colormaporigin;
	short colormaplength;
	char colormapdepth;
	short x_origin;
	short y_origin;
	short width;
	short height;
	char bitsperpixel;
	char imagedescriptor;
};
#pragma pack(pop)

struct TGAColor
{
	u8 bgra[4];
	u8 bytespp;

	TGAColor() : bytespp(1)
	{
		for (int i = 0; i < 4; i++)
			bgra[i] = 0;
	}

	TGAColor(u8 R, u8 G, u8 B, u8 A = 255) : bytespp(4)
	{
		bgra[0] = B;
		bgra[1] = G;
		bgra[2] = R;
		bgra[3] = A;
	}

	TGAColor(u8 v) : bytespp(1)
	{
		for (int i = 0; i < 4; i++) bgra[i] = 0;
		bgra[0] = v;
	}

	TGAColor(u8k *p, u8 bpp) : bytespp(bpp)
	{
		for (int i = 0; i < (int)bpp; i++)
		{
			bgra[i] = p[i];
		}
		for (int i = bpp; i < 4; i++)
		{
			bgra[i] = 0;
		}
	}

	u8 &operator[](const int i) { return bgra[i]; }

	TGAColor operator*(float intensity) const
	{
		TGAColor res = *this;
		intensity = (intensity > 1.f ? 1.f : (intensity < 0.f ? 0.f : intensity));
		for (int i = 0; i < 4; i++) res.bgra[i] = bgra[i] * intensity;
		return res;
	}
};

class TGAImage
{
protected:
	u8 *data;
	int width;
	int height;
	int bytespp;

	bool load_rle_data(std::ifstream &in);
	bool unload_rle_data(std::ofstream &out) const;

public:
	enum Format
	{
		GRAYSCALE = 1,
		RGB = 3,
		RGBA = 4
	};

	TGAImage();
	TGAImage(int w, int h, int bpp);
	TGAImage(const TGAImage &img);
	bool read_tga_file(tukk filename);
	bool write_tga_file(tukk filename, bool rle = true) const;
	bool flip_horizontally();
	bool flip_vertically();
	bool scale(int w, int h);
	TGAColor get(int x, int y) const;

	bool set(int x, int y, TGAColor &c);
	bool set(int x, int y, const TGAColor &c);
	~TGAImage();
	TGAImage &operator=(const TGAImage &img);
	int get_width();
	int get_height();
	int get_bytespp();
	u8 *buffer();
	void clear();
};

#endif  //__IMAGE_H__
