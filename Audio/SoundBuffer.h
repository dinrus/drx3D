#pragma once

#include <unordered_map>

#include <drx3D/Maths/Vector3.h>
#include <drx3D/Resources/Resource.h>
#include <drx3D/Audio/Audio.h>

namespace drx3d {
template<typename Base>
class SoundBufferFactory {
public:
	using TLoadMethod = std::function<void(Base &, const std::filesystem::path &)>;
	using TWriteMethod = std::function<void(const Base &, const std::filesystem::path &)>;
	using TRegistryMap = std::unordered_map<STxt, std::pair<TLoadMethod, TWriteMethod>>;

	virtual ~SoundBufferFactory() = default;

	static TRegistryMap &DoRegister() {
		static TRegistryMap impl;
		return impl;
	}

	template<typename T>
	class Registry /*: public Base*/ {
	protected:
		template<typename ...Args>
		static bool Register(Args &&... names) {
			for (STxt &&name : {names...})
				SoundBufferFactory::DoRegister()[name] = std::make_pair(&T::Load, &T::Write);
			return true;
		}
	};
};

/**
 * @brief Resource that represents a sound buffer.
 */
class DRX3D_EXPORT SoundBuffer : public SoundBufferFactory<SoundBuffer>, public Resource {
public:
	/**
	 * Creates a new sound buffer, or finds one with the same values.
	 * @param node The node to decode values from.
	 * @return The sound buffer with the requested values.
	 */
	static std::shared_ptr<SoundBuffer> Create(const Node &node);
	/**
	 * Creates a new sound buffer, or finds one with the same values.
	 * @param filename The file to load the sound buffer from.
	 * @return The sound buffer with the requested values.
	 */
	static std::shared_ptr<SoundBuffer> Create(const std::filesystem::path &filename);

	/**
	 * Creates a new sound buffer.
	 * @param filename The file to load the sound buffer from.
	 * @param load If this resource will be loaded immediately, otherwise {@link SoundBuffer#Load} can be called later.
	 */
	explicit SoundBuffer(std::filesystem::path filename, bool load = true);
	~SoundBuffer();

	std::type_index GetTypeIndex() const override { return typeid(SoundBuffer); }

	const std::filesystem::path &GetFilename() const { return filename; };
	uint32_t GetBuffer() const { return buffer; }
	void SetBuffer(uint32_t buffer);

	friend const Node &operator>>(const Node &node, SoundBuffer &soundBuffer);
	friend Node &operator<<(Node &node, const SoundBuffer &soundBuffer);

private:
	void Load();

	std::filesystem::path filename;
	uint32_t buffer = 0;
};
}
