// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/String/DrxString.h>
#include <drx3D/CoreX/String/DrxFixedString.h>
#include <algorithm>
#include <type_traits>

#if DRX_PLATFORM_POSIX
	#define DRX_NATIVE_PATH_SEPSTR "/"
#else
	#define DRX_NATIVE_PATH_SEPSTR "\\"
#endif

namespace PathUtil
{
namespace detail
{
template<typename>
struct IsValidStringType : std::false_type {};

template<>
struct IsValidStringType<string> : std::true_type {};

template<>
struct IsValidStringType<wstring> : std::true_type {};

template<size_t Size>
struct IsValidStringType<DrxStackStringT<char, Size>> : std::true_type {};

template<size_t Size>
struct IsValidStringType<DrxStackStringT<wchar_t, Size>> : std::true_type {};

template<size_t Size>
struct IsValidStringType<DrxFixedStringT<Size>> : std::true_type {};

template<size_t Size>
struct IsValidStringType<DrxFixedWStringT<Size>> : std::true_type {};

template<typename>
struct SStringConstants 
{
	static constexpr tukk ForwardSlash = "/";
	static constexpr tukk BackSlash = "\\";
};

template<>
struct SStringConstants<wstring>
{
	static constexpr const wchar_t* ForwardSlash = L"/";
	static constexpr const wchar_t* BackSlash = L"\\";
};
}

//! Convert a path to the uniform form.
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, TString>::type
inline /*TString*/ ToUnixPath(const TString& strPath)
{
	if (strPath.find('\\') != TString::npos)
	{
		auto path = strPath;
#ifdef DRX_STRING
		path.replace('\\', '/');
#else
		std::replace(path.begin(), path.end(), '\\', '/');
#endif
		return path;
	}
	return strPath;
}

inline string ToUnixPath(tukk szPath)
{
	return ToUnixPath(string(szPath));
}

//! Convert a path to the DOS form.
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, TString>::type
inline /*TString*/ ToDosPath(const TString& strPath)
{
	if (strPath.find('/') != TString::npos)
	{
		auto path = strPath;
#ifdef DRX_STRING
		path.replace('/', '\\');
#else
		std::replace(path.begin(), path.end(), '/', '\\');
#endif
		return path;
	}
	return strPath;
}

inline string ToDosPath(tukk szPath)
{
	return ToDosPath(string(szPath));
}

//! Split full file name to path and filename.
//! \param[in] filepath Full file name including path.
//! \param[out] path Extracted file path.
//! \param[out] filename Extracted file (without extension).
//! \param[out] extension Extracted file's extension (without .).
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value>::type
inline /*uk / Split(const TString& filepath, TString& path, TString& filename, TString& extension)
{
	path = filename = extension = TString();
	if (filepath.empty())
		return;
	tukk szFilepath = filepath.c_str();
	tukk szExtension = szFilepath + filepath.length() - 1;
	tukk p;
	for (p = szExtension; p >= szFilepath; --p)
	{
		switch (*p)
		{
		case ':':
		case '/':
		case '\\':
			path = TString(szFilepath, p + 1);
			filename = TString(p + 1, szExtension);
			return;
		case '.':
			// there's an extension in this file name
			extension = filepath.substr(p - szFilepath + 1);
			szExtension = p;
			break;
		}
	}
	filename = filepath.substr(p - szFilepath + 1, szExtension - p);
}

template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value>::type
inline /*uk / Split(tukk szFilepath, TString& path, TString& filename, TString& extension)
{
	return Split(TString(szFilepath), path, filename, extension);
}

//! Split full file name to path and filename.
//! \param[in] filepath Full file name including path.
//! \param[out] path Extracted file path.
//! \param[out] file Extracted file (with extension).
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value>::type
inline /*uk / Split(const TString& filepath, TString& path, TString& file)
{
	TString extension;
	Split(filepath, path, file, extension);
	if (!extension.empty())
	{
		file += '.' + extension;
	}
}

template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value>::type
inline /*uk / Split(tukk szFilepath, TString& path, TString& file)
{
	return Split(TString(szFilepath), path, file);
}

//! Extract extension from full specified file path.
//! \return Pointer to the extension (without .) or pointer to an empty 0-terminated string.
inline tukk GetExt(tukk filepath)
{
	tukk szFilepath = filepath;
	const size_t length = strlen(filepath);
	for (tukk p = szFilepath + length - 1; p >= szFilepath; --p)
	{
		switch (*p)
		{
		case ':':
		case '/':
		case '\\':
			// we've reached a path separator - it means there's no extension in this name
			return "";
		case '.':
			// there's an extension in this file name
			return p + 1;
		}
	}
	return "";
}

//! Removes filename from path. Example: "some/directory/file.ext" => "some/directory/"
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, TString>::type
inline /*TString*/ GetPathWithoutFilename(const TString& filepath)
{
	tukk szFilepath = filepath.c_str();
	for (tukk p = szFilepath + filepath.length() - 1; p >= szFilepath; --p)
	{
		switch (*p)
		{
		case ':':
		case '/':
		case '\\':
			return filepath.substr(0, p - szFilepath + 1);
		}
	}
	return "";
}

//! Removes filename from path. Example: "some/directory/file.ext" => "some/directory/"
inline string GetPathWithoutFilename(tukk filepath)
{
	return GetPathWithoutFilename(string(filepath));
}

//! Extract file name with extension from full specified file path.
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, TString>::type
inline /*TString*/ GetFile(const TString& filepath)
{
	tukk szFilepath = filepath.c_str();
	for (tukk p = szFilepath + filepath.length() - 1; p >= szFilepath; --p)
	{
		switch (*p)
		{
		case ':':
		case '/':
		case '\\':
			return filepath.substr(p - szFilepath + 1);
		}
	}
	return filepath;
}

inline tukk GetFile(tukk szFilepath)
{
	const size_t len = strlen(szFilepath);
	for (tukk p = szFilepath + len - 1; p >= szFilepath; --p)
	{
		switch (*p)
		{
		case ':':
		case '/':
		case '\\':
			return p + 1;
		}
	}
	return szFilepath;
}

//! Remove extension for given file.
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, void>::type
inline /*uk / RemoveExtension(TString& filepath)
{
	tukk szFilepath = filepath.c_str();
	for (tukk p = szFilepath + filepath.length() - 1; p >= szFilepath; --p)
	{
		switch (*p)
		{
		case ':':
		case '/':
		case '\\':
			// we've reached a path separator - it means there's no extension in this name
			return;
		case '.':
			// there's an extension in this file name
			filepath = filepath.erase(p - szFilepath);
			return;
		}
	}
	// it seems the file name is a pure name, without extension
}

//! Remove extension for given file.
inline void RemoveExtension(tuk szFilePath)
{
	for (tuk p = szFilePath + strlen(szFilePath) - 1; p >= szFilePath; --p)
	{
		switch (*p)
		{
		case ':':
		case '/':
		case '\\':
			// we've reached a path separator - it means there's no extension in this name
			return;
		case '.':
			// there's an extension in this file name
			*p = '\0';
			return;
		}
	}
	// it seems the file name is a pure name, without extension
}

template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, TString>::type
inline /*TString*/ RemoveExtension(const TString& filepath)
{
	auto result = filepath;
	RemoveExtension(result);
	return result;
}

inline string RemoveExtension(tukk szFilepath)
{
	return RemoveExtension(string(szFilepath));
}

//! Extract file name without extension from full specified file path.
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, TString>::type
inline /*TString*/ GetFileName(const TString& filepath)
{
	return GetFile(RemoveExtension(filepath));
}

inline string GetFileName(tukk szFilepath)
{
	return GetFileName(string(szFilepath));
}

//! Removes the trailing slash or backslash from a given path.
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, TString>::type
inline /*TString*/ RemoveSlash(const TString& path)
{
	if (path.empty() || (path[path.length() - 1] != '/' && path[path.length() - 1] != '\\'))
		return path;
	return path.substr(0, path.length() - 1);
}

inline string RemoveSlash(tukk szPath)
{
	return RemoveSlash(string(szPath));
}

inline string GetEnginePath()
{
	char szEngineRootDir[_MAX_PATH];
	DrxFindEngineRootFolder(DRX_ARRAY_COUNT(szEngineRootDir), szEngineRootDir);
	return RemoveSlash(szEngineRootDir);
}

//! Add a slash if necessary.
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, TString>::type
inline /*TString*/ AddSlash(const TString& path)
{
	if (path.empty() || path[path.length() - 1] == '/')
		return path;
	if (path[path.length() - 1] == '\\')
		return path.substr(0, path.length() - 1) + detail::SStringConstants<TString>::ForwardSlash;
	return path + detail::SStringConstants<TString>::ForwardSlash;
}

inline string AddSlash(tukk szPath)
{
	return AddSlash(string(szPath));
}

//! Add a backslash if necessary.
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, TString>::type
inline /*TString*/ AddBackslash(const TString& path)
{
	if (path.empty() || path[path.length() - 1] == '\\')
		return path;
	if (path[path.length() - 1] == '/')
		return path.substr(0, path.length() - 1) + detail::SStringConstants<TString>::BackSlash;
	return path + detail::SStringConstants<TString>::BackSlash;
}

inline string AddBackslash(tukk szPath)
{
	return AddBackslash(string(szPath));
}

//! Replace extension for given file.
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, TString>::type
inline /*TString*/ ReplaceExtension(const TString& filepath, const TString& extension)
{
	auto str = filepath;
	RemoveExtension(str);
	if (extension[0] != '\0' && extension[0] != '.')
	{
		str += ".";
	}
	str += extension;
	return str;
}

inline string ReplaceExtension(tukk szFilepath, tukk szExtension)
{
	return ReplaceExtension(string(szFilepath), string(szExtension));
}

//! Makes a fully specified file path from path and file name.
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, TString>::type
inline /*TString*/ Make(const TString& directory, const TString& filename)
{
	return AddSlash(directory) + filename;
}

inline string Make(tukk szPath, tukk szFilename)
{
	return Make(string(szPath), string(szFilename));
}

//! Makes a fully specified file path from path and file name.
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, TString>::type
inline /*TString*/ Make(const TString& directory, const TString& filename, const TString& extension)
{
	return AddSlash(directory) + ReplaceExtension(filename, extension);
}

inline string Make(tukk szPath, tukk szFilename, tukk szExtension)
{
	return Make(string(szPath), string(szFilename), string(szExtension));
}

template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, TString>::type
inline /*TString*/ GetParentDirectory(const TString& filePath, i32 generation = 1)
{
	for (tukk p = filePath.c_str() + filePath.length() - 2;   // -2 is for the possible trailing slash: there always must be some trailing symbol which is the file/directory name for which we should get the parent
	     p >= filePath.c_str();
	     --p)
	{
		switch (*p)
		{
		case ':':
			return TString(filePath.c_str(), p);
		case '/':
		case '\\':
			// we've reached a path separator - return everything before it.
			if (!--generation)
				return TString(filePath.c_str(), p);
			break;
		}
	}

	// it seems the file name is a pure name, without path or extension
	return TString();
}

inline string GetParentDirectory(tukk szFilePath, i32 generation = 1)
{
	return GetParentDirectory(string(szFilePath), generation);
}

//! \return True if the string matches the wildcard.
inline bool MatchWildcard(tukk szString, tukk szWildcard)
{
	tukk pString = szString, * pWildcard = szWildcard;
	// skip the obviously the same starting substring
	while (*pWildcard && *pWildcard != '*' && *pWildcard != '?')
	{
		if (*pString != *pWildcard)
			return false;   // must be exact match unless there's a wildcard character in the wildcard string
		else
			++pString, ++pWildcard;
	}

	if (!*pString)
	{
		// this will only match if there are no non-wild characters in the wildcard
		for (; *pWildcard; ++pWildcard)
		{
			if (*pWildcard != '*' && *pWildcard != '?')
				return false;
		}
		return true;
	}

	switch (*pWildcard)
	{
	case '\0':
		return false;   // the only way to match them after the leading non-wildcard characters is !*pString, which was already checked

	// we have a wildcard with wild character at the start.
	case '*':
		{
			// merge consecutive ? and *, since they are equivalent to a single *
			while (*pWildcard == '*' || *pWildcard == '?')
				++pWildcard;

			if (!*pWildcard)
				return true;   // the rest of the string doesn't matter: the wildcard ends with *

			for (; *pString; ++pString)
			{
				if (MatchWildcard(pString, pWildcard))
					return true;
			}

			return false;
		}

	case '?':
		return MatchWildcard(pString + 1, pWildcard + 1) || MatchWildcard(pString, pWildcard + 1);

	default:
		assert(false);
		return false;
	}
}

inline std::vector<string> GetDirectoryStructure(const string& path)
{
	if (path.empty())
	{
		return std::vector<string>();
	}

	string currentDirectoryName;
	tukk pchCurrentPosition = path.c_str();
	tukk pchLastPosition = path.c_str();

	// It removes as many slashes the path has in its start...
	// MAYBE and just maybe we should consider paths starting with
	// more than 2 slashes invalid paths...
	while ((*pchLastPosition == '\\') || (*pchLastPosition == '/'))
	{
		++pchLastPosition;
		++pchCurrentPosition;
	}

	std::vector<string> result;
	do
	{
		pchCurrentPosition = strpbrk(pchLastPosition, "\\/");
		if (pchCurrentPosition == nullptr)
		{
			break;
		}
		currentDirectoryName.assign(pchLastPosition, pchCurrentPosition);
		pchLastPosition = pchCurrentPosition + 1;
		// Again, here we are skipping as many consecutive slashes.
		while ((*pchLastPosition == '\\') || (*pchLastPosition == '/'))
		{
			++pchLastPosition;
		}
		result.push_back(currentDirectoryName);
	}
	while (true);

	return result;
}

enum EPathStyle
{
	//! Allows Posix paths:
	//! - absolute path (/foo/bar)
	//! - relative path (foo, ../bar) paths
	//! Output slashes are '/'
	ePathStyle_Posix,

	//! Allows Posix paths as well as these Windows specific paths:
	//! - UNC path (\\server\foo\bar)
	//! - drive-absolute path (c:\foo\bar)
	//! - drive-relative path (c:foo, c:..\bar)
	//! Output slashes are '\'
	ePathStyle_Windows,

#if DRX_PLATFORM_WINAPI
	ePathStyle_Native = ePathStyle_Windows,
#elif DRX_PLATFORM_POSIX
	ePathStyle_Native = ePathStyle_Posix,
#else
	#error Native path style is not supported
#endif
};

//! Simplifies a given file path, such that . and .. are removed (if possible).
//! Slashes are unified according to the path style and redundant and trailing slashes are removed.
//! The path-style also determines what inputs are accepted by the function. See EPathStyle enumeration values for more information.
//! If an error occurs or if the buffer is too small, the buffer will contain an empty string and the function returns false.
//! \note The output will always be shorter or equal length to the input, so a buffer of the same size as the input will always work.
inline bool SimplifyFilePath(tukk const szInput, tuk const szBuf, const size_t bufLength, const EPathStyle pathStyle)
{
#define DRX_SIMPLIFY_REJECT { *szBuf = 0; return false; }
#define DRX_SIMPLIFY_EMIT(expr) { if (--pOut < szBuf) { *szBuf = 0; return false; } *pOut = expr; }

	if (szBuf == nullptr || bufLength == 0)
	{
		return false;
	}
	if (szInput == nullptr || (*szInput == 0))
	{
		DRX_SIMPLIFY_REJECT;
	}

	const bool bWinApi = pathStyle == ePathStyle_Windows;
	const char kSlash = bWinApi ? '\\' : '/';
	tukk pIn = szInput + strlen(szInput);
	tukk pLast = pIn - 1;
	tukk pLastSlash = pIn;
	tuk pOut = szBuf + bufLength;
	size_t skipElements = 0;
	bool bDots = true;
	char driveRelative = 0;

	DRX_SIMPLIFY_EMIT(0); // null-terminator
	while (pIn != szInput)
	{
		assert(pIn >= szInput);
		const char c = *--pIn;
		switch (c)
		{
		case '\\':
		case '/':
			if ((pIn == szInput + 1) && ((szInput[0] == '\\') || (szInput[0] == '/'))) // UNC path
			{
				if (!bWinApi || bDots || skipElements != 0)
				{
					DRX_SIMPLIFY_REJECT;
				}
				DRX_SIMPLIFY_EMIT(kSlash);
				DRX_SIMPLIFY_EMIT(kSlash);
				pIn = szInput;
				continue;
			}
			else if (bDots) // handle redundant slashes and . and .. elements
			{
				const size_t numDots = pLastSlash - pIn - 1;
				if (numDots == 2)
				{
					++skipElements;
				}
				else if ((numDots != 0) && (numDots != 1) && (pIn != pLast))
				{
					DRX_SIMPLIFY_REJECT;
				}
			}
			else if (skipElements != 0) // mark eaten element
			{
				--skipElements;
			}
			if ((*pOut != kSlash) && (skipElements == 0))
			{
				if (*pOut != '\0') // don't emit trailing slashes
				{
					DRX_SIMPLIFY_EMIT(kSlash);
				}
				else if (pIn == szInput || (bWinApi && pIn[-1] == ':')) // exception for single slash input '/' and 'c:\'
				{
					DRX_SIMPLIFY_EMIT(kSlash);
				}
			}
			pLastSlash = pIn;
			bDots = true;
			continue;

		case '.':
			if (bDots) // count dots
			{
				continue;
			}
			break;

		case ':':
			if (bWinApi) // ':' should belong to a drive, otherwise it is not an allowed char in win
			{
				if ((pIn != szInput + 1) || ((pLastSlash == szInput + 2) && (skipElements != 0)))
				{
					DRX_SIMPLIFY_REJECT;
				}
				else // handle drive identifier
				{
					const char driveLetter = pIn[-1];
					if (!(driveLetter >= 'a' && driveLetter <= 'z') && !(driveLetter >= 'A' && driveLetter <= 'Z'))
					{
						DRX_SIMPLIFY_REJECT;
					}
					if (pLastSlash == szInput + 2)
					{
						if (skipElements != 0)
						{
							DRX_SIMPLIFY_REJECT;
						}
					}
					else if (bDots)
					{
						const size_t numDots = pLastSlash - pIn - 1;
						if (numDots == 2)
						{
							DRX_SIMPLIFY_EMIT('.');
							DRX_SIMPLIFY_EMIT('.');
						}
						else if (numDots != 1)
						{
							DRX_SIMPLIFY_REJECT;
						}
					}
					else if (skipElements != 0)
					{
						--skipElements;
					}
					driveRelative = driveLetter;
					pIn = szInput;
					bDots = false;
					continue;
				}
			}
		// fall-through
		default:
			if (bDots)
			{
				if (skipElements == 0)
				{
					const size_t numDots = pLastSlash - pIn - 1;
					for (size_t i = 0; i < numDots; ++i)
					{
						DRX_SIMPLIFY_EMIT('.');
					}
				}
				bDots = false;
			}
			break;
		}
		if (!skipElements)
		{
			DRX_SIMPLIFY_EMIT(c);
		}
	}

	if (bDots) // record remaining dots
	{
		const size_t numDots = pLastSlash - szInput;
		if (numDots == 2)
		{
			++skipElements;
		}
		else if (numDots == 1)
		{
			if ((*pOut == kSlash) && (skipElements == 0))
			{
				++pOut; // leading dot should eat a slash
			}
			else if (*pOut == 0)
			{
				DRX_SIMPLIFY_EMIT('.'); // special case, the input is only a dot, keep it
			}
		}
		else if (skipElements != 0)
		{
			DRX_SIMPLIFY_REJECT;
		}
	}
	else if (skipElements && !driveRelative) // if not bDots, then we read a relative element that needs to be discounted, e.g. a/..
	{
		--skipElements;
	}

	for (size_t i = 0; i < skipElements; ++i) // flush all pending dots
	{
		DRX_SIMPLIFY_EMIT('.');
		DRX_SIMPLIFY_EMIT('.');
		if (i != skipElements - 1)
		{
			DRX_SIMPLIFY_EMIT(kSlash);
		}
	}

	if (driveRelative != 0) // Fix up non-absolute but drive-relative paths.
	{
		DRX_SIMPLIFY_EMIT(':');
		DRX_SIMPLIFY_EMIT(driveRelative);
	}

	if (pOut != szBuf) // left-align in the buffer
	{
		const size_t resultLength = szBuf + bufLength - pOut;
		assert(resultLength > 0);
		memmove(szBuf, pOut, resultLength);
	}
	return true;

#undef DRX_SIMPLIFY_REJECT
#undef DRX_SIMPLIFY_EMIT
}

//! Converts \ to / and replaces ASCII characters to lower-case (A-Z only).
//! This function is ASCII-only and Unicode agnostic.
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value>::type
inline /*uk / UnifyFilePath(TString& path)
{
	path.replace('\\', '/');
	path.MakeLower();
}
}

#ifndef DRX_COMMON_HELPERS_ONLY
#include <drx3D/Sys/IDrxPak.h>

namespace PathUtil
{
inline string GetGameFolder()
{
	DRX_ASSERT(gEnv && gEnv->pDrxPak);
	return gEnv->pDrxPak->GetGameFolder();
}

inline string GetLocalizationFolder()
{
	DRX_ASSERT(gEnv && gEnv->pDrxPak);
	return gEnv->pDrxPak->GetLocalizationFolder();
}

//! Make a game correct path out of any input path.
template<typename TString>
typename std::enable_if<detail::IsValidStringType<TString>::value, TString>::type
inline /*TString*/ MakeGamePath(const TString& path)
{
	const auto fullpath = ToUnixPath(path);
	const auto rootDataFolder = ToUnixPath(AddSlash(PathUtil::GetGameFolder()));
	if (fullpath.length() > rootDataFolder.length() && strnicmp(fullpath.c_str(), rootDataFolder.c_str(), rootDataFolder.length()) == 0)
	{
		return fullpath.substr(rootDataFolder.length(), fullpath.length() - rootDataFolder.length());
	}
	return fullpath;
}

inline string MakeGamePath(tukk szPath)
{
	return MakeGamePath(string(szPath));
}
}
#endif