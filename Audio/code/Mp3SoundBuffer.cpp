#include <drx3D/Audio/Mp3/Mp3SoundBuffer.h>

#ifdef DRX3D_BUILD_MACOS
#include <OpenAL/al.h>
#else
#include <AL/al.h>
#endif
#include <X/dr_libs/dr_mp3.h>

#include <drx3D/Files/Files.h>
#include <drx3D/Maths/Time.h>

namespace drx3d {
void Mp3SoundBuffer::Load(SoundBuffer &soundBuffer, const std::filesystem::path &filename) {
#ifdef DRX3D_DEBUG
	auto debugStart = Time::Now();
#endif

	auto fileLoaded = Files::Read(filename);

	if (!fileLoaded) {
		Log::Error("SoundBuffer could not be loaded: ", filename, '\n');
		return;
	}

	drmp3_config config;
	drmp3_uint64 totalPCMFrameCount;
	auto sampleData = drmp3_open_memory_and_read_pcm_frames_s16(fileLoaded->data(), fileLoaded->size(), &config, &totalPCMFrameCount, nullptr);
	if (!sampleData) {
		Log::Error("Error reading OGG ", filename, ", could not load samples\n");
		return;
	}

	uint32_t buffer;
	alGenBuffers(1, &buffer);
	alBufferData(buffer, (config.channels == 2) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, sampleData, totalPCMFrameCount, config.sampleRate);

	Audio::CheckAl(alGetError());

	soundBuffer.SetBuffer(buffer);

	drmp3_free(sampleData, nullptr);

#ifdef DRX3D_DEBUG
	Log::Out("SoundBuffer ", filename, " загружен за ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}

void Mp3SoundBuffer::Write(const SoundBuffer &soundBuffer, const std::filesystem::path &filename) {
#ifdef DRX3D_DEBUG
	auto debugStart = Time::Now();
#endif

	// TODO: Implement

#ifdef DRX3D_DEBUG
	Log::Out("SoundBuffer ", filename, " записан за ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}
}
