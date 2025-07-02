// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace AssetLoader
{

struct IFileReader
{
public:
	typedef std::function<void(tukk pBuffer, size_t numberOfBytes, uk pUserData)> CompletionHandler;

public:
	virtual bool ReadFileAsync(tukk szFilePath, const CompletionHandler& completionHandler, uk pUserData) = 0;
	virtual void WaitForCompletion() = 0;
	virtual ~IFileReader() {};
};

std::unique_ptr<IFileReader> CreateFileReader();

}
