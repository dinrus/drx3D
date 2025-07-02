#pragma once

#include <drx3D/Audio/SoundBuffer.h>

namespace drx3d {
class DRX3D_EXPORT OpusSoundBuffer : public SoundBuffer::Registry<OpusSoundBuffer> {
	inline static const bool Registered = Register(".opus");
public:
	static void Load(SoundBuffer &soundBuffer, const std::filesystem::path &filename);
	static void Write(const SoundBuffer &soundBuffer, const std::filesystem::path &filename);
};
}
