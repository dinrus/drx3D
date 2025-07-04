#pragma once

#include <drx3D/Engine/Engine.h>

struct PHYSFS_File;

namespace drx3d {
enum class FileMode {
	Read, Write, Append
};

class DRX3D_EXPORT BaseFStream {
public:
	explicit BaseFStream(PHYSFS_File *file);
	virtual ~BaseFStream();

	size_t length();
protected:
	PHYSFS_File *file;
};

class DRX3D_EXPORT IFStream : public BaseFStream, public std::istream {
public:
	explicit IFStream(const std::filesystem::path &filename);
	virtual ~IFStream();
};

class DRX3D_EXPORT OFStream : public BaseFStream, public std::ostream {
public:
	explicit OFStream(const std::filesystem::path &filename, FileMode writeMode = FileMode::Write);
	virtual ~OFStream();
};

class DRX3D_EXPORT FStream : public BaseFStream, public std::iostream {
public:
	explicit FStream(const std::filesystem::path &filename, FileMode openMode = FileMode::Read);
	virtual ~FStream();
};

/**
 * @brief Module used for managing files on engine updates.
 */
class DRX3D_EXPORT Files : public Module::Registry<Files> {
	inline static const bool Registered = Register(Stage::Post);
public:
	Files();
	~Files();

	void Update() override;

	/**
	 * Adds an file search path.
	 * @param path The path to add.
	 */
	void AddSearchPath(const STxt &path);

	/**
	 * Removes a file search path.
	 * @param path The path to remove.
	 */
	void RemoveSearchPath(const STxt &path);

	/**
	 * Clears all file search paths.
	 */
	void ClearSearchPath();

	/**
	 * Gets if the path is found in one of the search paths.
	 * @param path The path to look for.
	 * @return If the path is found in one of the searches.
	 */
	static bool ExistsInPath(const std::filesystem::path &path);

	/**
	 * Reads a file found by real or partial path.
	 * @param path The path to read.
	 * @return The data read from the file.
	 */
	static std::optional<STxt> Read(const std::filesystem::path &path);

	/**
	 * Reads all bytes from file found by real or partial path.
	 * @param path The path to read.
	 * @return The data read from the file.
	 */
	static std::vector<u8> ReadBytes(const std::filesystem::path &path);

	/**
	 * Finds all the files in a path.
	 * @param path The path to search.
	 * @param recursive If paths will be recursively searched.
	 * @return The files found.
	 */
	static std::vector<STxt> FilesInPath(const std::filesystem::path &path, bool recursive = true);

	/**
	 * Gets the next line from a stream.
	 * @param is The input stream.
	 * @param t The next string.
	 * @return The input stream.
	 */
	static std::istream &SafeGetLine(std::istream &is, STxt &t);

private:
	std::vector<STxt> searchPaths;
};
}
