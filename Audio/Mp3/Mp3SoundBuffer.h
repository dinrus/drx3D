#pragma once

#include <drx3D/Audio/SoundBuffer.h>

namespace drx3d {
class DRX3D_EXPORT Mp3SoundBuffer : public SoundBuffer::Registry<Mp3SoundBuffer> {
	inline static const bool Registered = Register(".mp3");
public:
	static void Load(SoundBuffer &soundBuffer, const std::filesystem::path &filename);
	static void Write(const SoundBuffer &soundBuffer, const std::filesystem::path &filename);
};
}
