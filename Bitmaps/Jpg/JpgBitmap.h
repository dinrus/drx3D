#pragma once

#include <drx3D/Bitmaps/Bitmap.h>

namespace drx3d {
class DRX3D_EXPORT JpgBitmap : public Bitmap::Registry<JpgBitmap> {
	inline static const bool Registered = Register(".jpg", ".jpeg");
public:
	static void Load(Bitmap &bitmap, const std::filesystem::path &filename);
	static void Write(const Bitmap &bitmap, const std::filesystem::path &filename);
};
}
