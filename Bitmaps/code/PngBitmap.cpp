#include <drx3D/Bitmaps/Png/PngBitmap.h>
#include <cstring>
#include <X/tiny/spng.h>
#include <drx3D/Files/Files.h>
#include <drx3D/Maths/Time.h>

namespace drx3d {
void PngBitmap::Load(Bitmap &bitmap, const std::filesystem::path &filename) {
#ifdef DRX3D_DEBUG
	auto debugStart = Time::Now();
#endif

	auto fileLoaded = Files::Read(filename);

	if (!fileLoaded) {
		Log::Error("не удалось загрузить битмап: ", filename, '\n');
		return;
	}
	
	/*uint8_t *buffer;
	uint32_t width = 0, height = 0;
	auto error = lodepng_decode_memory(&buffer, &width, &height, reinterpret_cast<const uint8_t *>(fileLoaded->data()), fileLoaded->length(), LCT_RGBA, 8);
	if (buffer && !error) {
		LodePNGColorMode color = {LCT_RGBA, 8};
		auto buffersize = lodepng_get_raw_size(width, height, &color);
		bitmap.SetData(std::make_unique<uint8_t[]>(buffersize));
		std::memcpy(bitmap.GetData().get(), buffer, buffersize);
		bitmap.SetSize({ width, height });
		bitmap.SetBytesPerPixel(buffersize / (width * height));
		free(buffer); // lodepng_free
	}*/

#ifdef DRX3D_DEBUG
	Log::Out("Bitmap ", filename, " loaded in ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}

void PngBitmap::Write(const Bitmap &bitmap, const std::filesystem::path &filename) {
#ifdef DRX3D_DEBUG
	auto debugStart = Time::Now();
#endif

	/*LodePNGColorType colorType = LCT_GREY;
	if (bitmap.GetBytesPerPixel() == 4)
		colorType = LCT_RGBA;
	else if (bitmap.GetBytesPerPixel() == 3)
		colorType = LCT_RGB;
	else
		Log::Error("Cannot write PNG with ", bitmap.GetBytesPerPixel(), " bytes per pixel\n");

	lodepng::encode(filename.string(), bitmap.GetData().get(), bitmap.GetSize().x, bitmap.GetSize().y, colorType);*/

#ifdef DRX3D_DEBUG
	Log::Out("Bitmap ", filename, " written in ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}
}
