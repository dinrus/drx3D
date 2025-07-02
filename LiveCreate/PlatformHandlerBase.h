// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _LIVECREATE_PLATFORMHANDLERBASE_H_
#define _LIVECREATE_PLATFORMHANDLERBASE_H_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <drx3D/CoreX/BaseTypes.h>
#include <drx3D/LiveCreate/ILiveCreateCommon.h>
#include <drx3D/LiveCreate/ILiveCreatePlatform.h>
#include <vector>
#include <algorithm>

#ifdef LC_USE_STD_STRING
	#include <string>
using namespace std;
#endif

#define LC_API extern "C" __declspec(dllexport)
typedef LiveCreate::IPlatformHandlerFactory* (* TPfnCreatePlatformHandlerFactory)();

#if defined(LIVECREATE_FOR_PC) && !defined(NO_LIVECREATE)

namespace LiveCreate
{

// Base platform implementation
class PlatformHandlerBase : public IPlatformHandler
{
public:
	PlatformHandlerBase(IPlatformHandlerFactory* pFactory, tukk szTargetName);
	virtual ~PlatformHandlerBase();

	virtual bool                     IsOn() const;
	virtual bool                     IsFlagSet(u32 aFlag) const;
	virtual tukk              GetTargetName() const;
	virtual tukk              GetPlatformName() const;
	virtual IPlatformHandlerFactory* GetFactory() const override;
	virtual bool                     ResolveAddress(tuk pOutIpAddress, u32 aMaxOutIpAddressSize) const;
	virtual bool                     Launch(tukk pExeFilename, tukk pWorkingFolder, tukk pArgs) const;
	virtual bool                     Reset(EResetMode aMode) const;
	virtual bool                     ScanFolder(tukk pFolder, IPlatformHandlerFolderScan& outResult) const;
	virtual void                     AddRef();
	virtual void                     Release();

	virtual void                     Delete() = 0;

protected:
	IPlatformHandlerFactory* m_pFactory;
	 i32             m_refCount;
	string                   m_targetName;
	u32                   m_flags;
};

//! add a backslash if needed
inline string AddSlash(const string& path)
{
	if (path.empty() || path[path.length() - 1] == '/')
		return path;

	if (path[path.length() - 1] == '\\')
		return path.substr(0, path.length() - 1) + "/";

	return path + "/";
}

inline void ReplaceAllStrings(string& str, const string& oldStr, const string& newStr)
{
	size_t pos = 0;
	while ((pos = str.find(oldStr, pos)) != string::npos)
	{
		str.replace(pos, oldStr.length(), newStr);
		pos += newStr.length();
	}
}

//! Convert a path to the DOS form.
inline string ToDosPath(const string& strPath)
{
	if (strPath.find('/') != string::npos)
	{
		string path = strPath;

		ReplaceAllStrings(path, "/", "\\");

		return path;
	}

	return strPath;
}

//! Removes the trailing slash or backslash from a given path.
inline string RemoveSlash(const string& path)
{
	if (path.empty() || (path[path.length() - 1] != '/' && path[path.length() - 1] != '\\'))
		return path;

	return path.substr(0, path.length() - 1);
}

//! Convert a path to the uniform form.
inline string ToUnixPath(const string& strPath)
{
	if (strPath.find('\\') != string::npos)
	{
		string path = strPath;
		ReplaceAllStrings(path, "\\", "/");
		return path;
	}
	return strPath;
}

// Extract extension from full specified file path
// Returns
//	pointer to the extension (without .) or pointer to an empty 0-terminated string
inline tukk GetExt(tukk filepath)
{
	tukk str = filepath;
	size_t len = strlen(filepath);

	for (tukk p = str + len - 1; p >= str; --p)
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

inline string TokenizeString(string theString, tukk charSet, i32& nStart)
{
	if (nStart < 0)
	{
		return "";
	}

	if (!charSet)
		return theString;

	tukk sPlace = theString.c_str() + nStart;
	tukk sEnd = theString.c_str() + theString.length();

	if (sPlace < sEnd)
	{
		i32 nIncluding = (i32)strspn(sPlace, charSet);

		if ((sPlace + nIncluding) < sEnd)
		{
			sPlace += nIncluding;
			i32 nExcluding = (i32)strcspn(sPlace, charSet);
			i32 nFrom = nStart + nIncluding;
			nStart = nFrom + nExcluding + 1;

			return theString.substr(nFrom, nExcluding);
		}
	}

	nStart = -1;

	return "";
}

inline string TrimString(const string& t, const string& wspace = " \t\n\r")
{
	string str = t;
	size_t found;

	found = str.find_last_not_of(wspace);

	if (found != string::npos)
		str.erase(found + 1);
	else
		str.clear();

	return str;
}

}

#endif
#endif
