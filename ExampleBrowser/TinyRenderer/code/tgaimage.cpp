#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <math.h>
#include "../tgaimage.h"

TGAImage::TGAImage() : data(nullptr), width(0), height(0), bytespp(0) {}

TGAImage::TGAImage(i32 w, i32 h, i32 bpp) : data(nullptr), width(w), height(h), bytespp(bpp)
{
	u64 nbytes = width * height * bytespp;
	data = new u8[nbytes];
	//memset(data, 0, nbytes);
}

TGAImage::TGAImage(const TGAImage &img) : data(nullptr), width(img.width), height(img.height), bytespp(img.bytespp)
{
	u64 nbytes = width * height * bytespp;
	data = new u8[nbytes];
	//memcpy(data, img.data, nbytes);
}

TGAImage::~TGAImage()
{
	if (data) delete[] data;
}

TGAImage &TGAImage::operator=(const TGAImage &img)
{
	if (this != &img)
	{
		if (data) delete[] data;
		width = img.width;
		height = img.height;
		bytespp = img.bytespp;
		u64 nbytes = width * height * bytespp;
		data = new u8[nbytes];
		memcpy(data, img.data, nbytes);
	}
	return *this;
}

bool TGAImage::read_tga_file(tukk filename)
{
	if (data) delete[] data;
	data = nullptr;
	std::ifstream in;
	in.open(filename, std::ios::binary);
	if (!in.is_open())
	{
		std::cerr << "не удалось открыть файл " << filename << "\n";
		in.close();
		return false;
	}
	TGA_Header header;
	in.read((char *)&header, sizeof(header));
	if (!in.good())
	{
		in.close();
		std::cerr << "ошибка при чтении заголовка\n";
		return false;
	}
	width = header.width;
	height = header.height;
	bytespp = header.bitsperpixel >> 3;
	if (width <= 0 || height <= 0 || (bytespp != GRAYSCALE && bytespp != RGB && bytespp != RGBA))
	{
		in.close();
		std::cerr << "неверное значение bpp (или ширина/высота)\n";
		return false;
	}
	u64 nbytes = bytespp * width * height;
	data = new u8[nbytes];
	if (3 == header.datatypecode || 2 == header.datatypecode)
	{
		in.read((char *)data, nbytes);
		if (!in.good())
		{
			in.close();
			std::cerr << "ошибка при чтении данных\n";
			return false;
		}
	}
	else if (10 == header.datatypecode || 11 == header.datatypecode)
	{
		if (!load_rle_data(in))
		{
			in.close();
			std::cerr << "ошибка при чтении данных\n";
			return false;
		}
	}
	else
	{
		in.close();
		std::cerr << "неизвестный формат файла " << (i32)header.datatypecode << "\n";
		return false;
	}
	if (!(header.imagedescriptor & 0x20))
	{
		flip_vertically();
	}
	if (header.imagedescriptor & 0x10)
	{
		flip_horizontally();
	}
	std::cerr << width << "x" << height << "/" << bytespp * 8 << "\n";
	in.close();
	return true;
}

bool TGAImage::load_rle_data(std::ifstream &in)
{
	u64 pixelcount = width * height;
	u64 currentpixel = 0;
	u64 currentbyte = 0;
	TGAColor colorbuffer;
	do
	{
		u8 chunkheader = 0;
		chunkheader = in.get();
		if (!in.good())
		{
			std::cerr << "ошибка при чтении данных\n";
			return false;
		}
		if (chunkheader < 128)
		{
			chunkheader++;
			for (i32 i = 0; i < chunkheader; i++)
			{
				in.read((char *)colorbuffer.bgra, bytespp);
				if (!in.good())
				{
					std::cerr << "ошибка при чтении заголовка\n";
					return false;
				}
				for (i32 t = 0; t < bytespp; t++)
					data[currentbyte++] = colorbuffer.bgra[t];
				currentpixel++;
				if (currentpixel > pixelcount)
				{
					std::cerr << "Считано слишком много пикселей\n";
					return false;
				}
			}
		}
		else
		{
			chunkheader -= 127;
			in.read((char *)colorbuffer.bgra, bytespp);
			if (!in.good())
			{
				std::cerr << "ошибка при чтении заголовка\n";
				return false;
			}
			for (i32 i = 0; i < chunkheader; i++)
			{
				for (i32 t = 0; t < bytespp; t++)
					data[currentbyte++] = colorbuffer.bgra[t];
				currentpixel++;
				if (currentpixel > pixelcount)
				{
					std::cerr << "Считано слишком много пикселей\n";
					return false;
				}
			}
		}
	} while (currentpixel < pixelcount);
	return true;
}

bool TGAImage::write_tga_file(tukk filename, bool rle) const
{
	u8 developer_area_ref[4] = {0, 0, 0, 0};
	u8 extension_area_ref[4] = {0, 0, 0, 0};
	u8 footer[18] = {'T', 'R', 'U', 'E', 'V', 'I', 'S', 'I', 'O', 'N', '-', 'X', 'F', 'I', 'L', 'E', '.', '\0'};
	std::ofstream out;
	out.open(filename, std::ios::binary);
	if (!out.is_open())
	{
		std::cerr << "не удаётся открыть файл " << filename << "\n";
		out.close();
		return false;
	}
	TGA_Header header;
	memset((uk )&header, 0, sizeof(header));
	header.bitsperpixel = bytespp << 3;
	header.width = width;
	header.height = height;
	header.datatypecode = (bytespp == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
	header.imagedescriptor = 0x20;  // top-left origin
	out.write((char *)&header, sizeof(header));
	if (!out.good())
	{
		out.close();
		std::cerr << "не удаётся сделать дамп файла tga\n";
		return false;
	}
	if (!rle)
	{
		out.write((char *)data, width * height * bytespp);
		if (!out.good())
		{
			std::cerr << "не удалось выгрузить сырые даные\n";
			out.close();
			return false;
		}
	}
	else
	{
		if (!unload_rle_data(out))
		{
			out.close();
			std::cerr << "не удалось выгрузить данные rle\n";
			return false;
		}
	}
	out.write((char *)developer_area_ref, sizeof(developer_area_ref));
	if (!out.good())
	{
		std::cerr << "не удаётся сделать дамп файла tga\n";
		out.close();
		return false;
	}
	out.write((char *)extension_area_ref, sizeof(extension_area_ref));
	if (!out.good())
	{
		std::cerr << "не удаётся сделать дамп файла tga\n";
		out.close();
		return false;
	}
	out.write((char *)footer, sizeof(footer));
	if (!out.good())
	{
		std::cerr << "не удаётся сделать дамп файла tga\n";
		out.close();
		return false;
	}
	out.close();
	return true;
}

// TODO: it is not necessary to break a raw chunk for two equal pixels (for the matter of the resulting size)
bool TGAImage::unload_rle_data(std::ofstream &out) const
{
	u8k max_chunk_length = 128;
	u64 npixels = width * height;
	u64 curpix = 0;
	while (curpix < npixels)
	{
		u64 chunkstart = curpix * bytespp;
		u64 curbyte = curpix * bytespp;
		u8 run_length = 1;
		bool raw = true;
		while (curpix + run_length < npixels && run_length < max_chunk_length)
		{
			bool succ_eq = true;
			for (i32 t = 0; succ_eq && t < bytespp; t++)
			{
				succ_eq = (data[curbyte + t] == data[curbyte + t + bytespp]);
			}
			curbyte += bytespp;
			if (1 == run_length)
			{
				raw = !succ_eq;
			}
			if (raw && succ_eq)
			{
				run_length--;
				break;
			}
			if (!raw && !succ_eq)
			{
				break;
			}
			run_length++;
		}
		curpix += run_length;
		out.put(raw ? run_length - 1 : run_length + 127);
		if (!out.good())
		{
			std::cerr << "не удаётся сделать дамп файлаe tga\n";
			return false;
		}
		out.write((char *)(data + chunkstart), (raw ? run_length * bytespp : bytespp));
		if (!out.good())
		{
			std::cerr << "не удаётся сделать дамп файла tga\n";
			return false;
		}
	}
	return true;
}

TGAColor TGAImage::get(i32 x, i32 y) const
{
	if (x < 0)
	{
		x = 0;
	}
	if (y < 0)
	{
		y = 0;
	}
	if (x >= width)
	{
		x = width - 1;
	}
	if (y >= height)
	{
		y = height - 1;
	}

	if (!data || x < 0 || y < 0 || x >= width || y >= height)
	{
		return TGAColor(128.f, 128.f, 128.f, 255.f);
	}
	return TGAColor(data + (x + y * width) * bytespp, bytespp);
}

bool TGAImage::set(i32 x, i32 y, TGAColor &c)
{
	if (!data || x < 0 || y < 0 || x >= width || y >= height)
	{
		return false;
	}
	memcpy(data + (x + y * width) * bytespp, c.bgra, bytespp);
	return true;
}

bool TGAImage::set(i32 x, i32 y, const TGAColor &c)
{
	if (!data || x < 0 || y < 0 || x >= width || y >= height)
	{
		return false;
	}
	memcpy(data + (x + y * width) * bytespp, c.bgra, bytespp);
	return true;
}

i32 TGAImage::get_bytespp()
{
	return bytespp;
}

i32 TGAImage::get_width()
{
	return width;
}

i32 TGAImage::get_height()
{
	return height;
}

bool TGAImage::flip_horizontally()
{
	if (!data) return false;
	i32 half = width >> 1;
	for (i32 i = 0; i < half; i++)
	{
		for (i32 j = 0; j < height; j++)
		{
			TGAColor c1 = get(i, j);
			TGAColor c2 = get(width - 1 - i, j);
			set(i, j, c2);
			set(width - 1 - i, j, c1);
		}
	}
	return true;
}

bool TGAImage::flip_vertically()
{
	if (!data) return false;
	u64 bytes_per_line = width * bytespp;
	u8 *line = new u8[bytes_per_line];
	i32 half = height >> 1;
	for (i32 j = 0; j < half; j++)
	{
		u64 l1 = j * bytes_per_line;
		u64 l2 = (height - 1 - j) * bytes_per_line;
		memmove((uk )line, (uk )(data + l1), bytes_per_line);
		memmove((uk )(data + l1), (uk )(data + l2), bytes_per_line);
		memmove((uk )(data + l2), (uk )line, bytes_per_line);
	}
	delete[] line;
	return true;
}

u8 *TGAImage::buffer()
{
	return data;
}

void TGAImage::clear()
{
	memset((uk )data, 0, width * height * bytespp);
}

bool TGAImage::scale(i32 w, i32 h)
{
	if (w <= 0 || h <= 0 || !data) return false;
	u8 *tdata = new u8[w * h * bytespp];
	i32 nscanline = 0;
	i32 oscanline = 0;
	i32 erry = 0;
	u64 nlinebytes = w * bytespp;
	u64 olinebytes = width * bytespp;
	for (i32 j = 0; j < height; j++)
	{
		i32 errx = width - w;
		i32 nx = -bytespp;
		i32 ox = -bytespp;
		for (i32 i = 0; i < width; i++)
		{
			ox += bytespp;
			errx += w;
			while (errx >= (i32)width)
			{
				errx -= width;
				nx += bytespp;
				memcpy(tdata + nscanline + nx, data + oscanline + ox, bytespp);
			}
		}
		erry += h;
		oscanline += olinebytes;
		while (erry >= (i32)height)
		{
			if (erry >= (i32)height << 1)  // it means we jump over a scanline
				memcpy(tdata + nscanline + nlinebytes, tdata + nscanline, nlinebytes);
			erry -= height;
			nscanline += nlinebytes;
		}
	}
	delete[] data;
	data = tdata;
	width = w;
	height = h;
	return true;
}
