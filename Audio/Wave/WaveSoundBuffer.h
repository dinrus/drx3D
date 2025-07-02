#pragma once

#include <drx3D/Audio/SoundBuffer.h>

namespace drx3d {
class DRX3D_EXPORT WaveSoundBuffer : public SoundBuffer::Registry<WaveSoundBuffer> {
	inline static const bool Registered = Register(".wav", ".wave");
public:
	static void Load(SoundBuffer &soundBuffer, const std::filesystem::path &filename);
	static void Write(const SoundBuffer &soundBuffer, const std::filesystem::path &filename);
};
}
