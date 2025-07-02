// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "FileReader.h"

namespace FileReader_Private
{

class CFileReader : public AssetLoader::IFileReader
{
public:
	virtual bool ReadFileAsync(tukk szFilePath, const CompletionHandler& completionHandler, uk pUserData) override
	{
		const std::vector<char> buffer = ReadFile(szFilePath);
		if (!buffer.size())
		{
			return false;
		}
		completionHandler(buffer.data(), buffer.size(), pUserData);
		return true;
	}

	virtual void WaitForCompletion() override {}

private:
	std::vector<char> ReadFile(tukk szFilePath)
	{
		typedef std::unique_ptr<FILE, i32(*)(FILE*)> file_ptr;

		file_ptr file(fopen(szFilePath, "rb"), fclose);
		fseek(file.get(), 0, SEEK_END);
		size_t size = ftell(file.get());
		fseek(file.get(), 0, SEEK_SET);
		std::vector<char> buffer(size);
		size_t elementsRead = fread(buffer.data(), sizeof(char), buffer.size(), file.get());
		buffer.resize(elementsRead);
		return buffer;
	}
};

}

namespace AssetLoader
{

std::unique_ptr<IFileReader> CreateFileReader()
{
	using namespace FileReader_Private;
	return std::unique_ptr<IFileReader>(new CFileReader());
}

}

