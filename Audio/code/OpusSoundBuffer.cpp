#include <drx3D/Audio/Opus/OpusSoundBuffer.h>

#ifdef DRX3D_BUILD_MACOS
#include <OpenAL/al.h>
#else
#include <AL/al.h>
#endif
#include <X/dr_libs/dr_opus.h>

#include <drx3D/Files/Files.h>
#include <drx3D/Maths/Time.h>

namespace drx3d {
void OpusSoundBuffer::Load(SoundBuffer &soundBuffer, const std::filesystem::path &filename) {
#ifdef DRX3D_DEBUG
	auto debugStart = Time::Now();
#endif

	auto fileLoaded = Files::Read(filename);

	if (!fileLoaded) {
		Log::Error("SoundBuffer could not be loaded: ", filename, '\n');
		return;
	}

	//soundBuffer.SetBuffer(buffer);
	
#ifdef DRX3D_DEBUG
	Log::Out("SoundBuffer ", filename, " loaded in ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}

void OpusSoundBuffer::Write(const SoundBuffer &soundBuffer, const std::filesystem::path &filename) {
#ifdef DRX3D_DEBUG
	auto debugStart = Time::Now();
#endif

	// TODO: Implement

#ifdef DRX3D_DEBUG
	Log::Out("SoundBuffer ", filename, " written in ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}
}
