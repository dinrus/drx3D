#include <drx3D/Bitmaps/Jpg/JpgBitmap.h>
#include <cstring>
#include <X/libjpgd/jpgd.h>
#include <drx3D/Files/Files.h>
#include <drx3D/Maths/Time.h>

namespace drx3d {
void JpgBitmap::Load(Bitmap &bitmap, const std::filesystem::path &filename) {
#ifdef DRX3D_DEBUG
	auto debugStart = Time::Now();
#endif

	auto fileLoaded = Files::Read(filename);

	if (!fileLoaded) {
		Log::Error("Bitmap could not be loaded: ", filename, '\n');
		return;
	}

	// TODO

#ifdef DRX3D_DEBUG
	Log::Out("Bitmap ", filename, " loaded in ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}

void JpgBitmap::Write(const Bitmap &bitmap, const std::filesystem::path &filename) {
#ifdef DRX3D_DEBUG
	auto debugStart = Time::Now();
#endif

	// TODO

#ifdef DRX3D_DEBUG
	Log::Out("Bitmap ", filename, " written in ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}
}
