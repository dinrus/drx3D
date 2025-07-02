#include <drx3D/Audio/Flac/FlacSoundBuffer.h>

#ifdef DRX3D_BUILD_MACOS
#include <OpenAL/al.h>
#else
#include <AL/al.h>
#endif
#include <X/dr_libs/dr_flac.h>

#include <drx3D/Files/Files.h>
#include <drx3D/Maths/Time.h>

namespace drx3d {
void FlacSoundBuffer::Load(SoundBuffer &soundBuffer, const std::filesystem::path &filename) {
#ifdef DRX3D_DEBUG
	auto debugStart = Time::Now();
#endif

	auto fileLoaded = Files::Read(filename);

	if (!fileLoaded) {
		Log::Error("Не удалось загрузить SoundBuffer: ", filename, '\n');
		return;
	}

	//soundBuffer->SetBuffer(buffer);

#ifdef DRX3D_DEBUG
	Log::Out("SoundBuffer ", filename, " загружен за ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}

void FlacSoundBuffer::Write(const SoundBuffer &soundBuffer, const std::filesystem::path &filename) {
#ifdef DRX3D_DEBUG
	auto debugStart = Time::Now();
#endif

	// TODO: Implement

#ifdef DRX3D_DEBUG
	Log::Out("SoundBuffer ", filename, " записан за ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}
}
