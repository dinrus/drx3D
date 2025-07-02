// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>
#include <drx3D/Schema2/IResourceCollectorArchive.h>

#include <drx3D/CoreX/Serialization/Decorators/ResourceFilePath.h>
#include <drx3D/CoreX/Serialization/Decorators/Resources.h>

namespace GameSerialization
{
	// Note: Do not use this directly (use the types below instead)
	struct SGameResource
	{
	private:
		friend struct SGameResourceFile;
		friend struct SGameResourceFileWithType;
		template <typename TString> friend struct SGameResourceSelector;

	private:
		SGameResource(tukk _szValue, tukk _szType)
			: szValue(_szValue)
			, szType(_szType)
		{

		}
		SGameResource() = delete;

	public:

		void Serialize(Serialization::IArchive& archive) {}; // Do nothing

		tukk szValue;
		tukk szType;
	};

	//////////////////////////////////////////////////////////////////////////

	struct SGameResourceFile
	{
		SGameResourceFile(string& path, tukk szFilter = "All files|*.*", tukk szStartFolder = "", i32 flags = 0)
			: filePath(path, szFilter, szStartFolder, flags)
		{

		}

		SGameResource ToGameResource() const
		{
			return SGameResource(filePath.path->c_str(), "Any");
		}

		Serialization::ResourceFilePath filePath;
	};

	inline bool Serialize(Serialization::IArchive& archive, SGameResourceFile& value, tukk szName, tukk szLabel)
	{
		if (archive.caps(sxema2::IResourceCollectorArchive::ArchiveCaps))
			return archive(value.ToGameResource(), szName, szLabel);
		else
			return archive(value.filePath, szName, szLabel);
	}

	//////////////////////////////////////////////////////////////////////////

	struct SGameResourceFileWithType
	{
		SGameResourceFileWithType(string& path, tukk szResourceType, tukk szFilter = "All files|*.*", tukk szStartFolder = "", i32 flags = 0)
			: filePath(path, szFilter, szStartFolder, flags)
			, szResourceType(szResourceType)
		{

		}

		SGameResource ToGameResource() const
		{
			return SGameResource(filePath.path->c_str(), szResourceType);
		}

		Serialization::ResourceFilePath filePath;
		tukk szResourceType;
	};

	inline bool Serialize(Serialization::IArchive& archive, SGameResourceFileWithType& value, tukk szName, tukk szLabel)
	{
		if (archive.caps(sxema2::IResourceCollectorArchive::ArchiveCaps))
			return archive(value.ToGameResource(), szName, szLabel);
		else
			return archive(value.filePath, szName, szLabel);
	}

	//////////////////////////////////////////////////////////////////////////

	template<typename TString>
	struct SGameResourceSelector
	{
		SGameResourceSelector(TString& value, tukk szResourceType)
			: selector(value, szResourceType)
		{

		}

		SGameResourceSelector(Serialization::ResourceSelector<TString>& src)
			: selector(src)
		{

		}

		SGameResource ToGameResource() const
		{
			return SGameResource(selector.GetValue(), selector.resourceType);
		}

		Serialization::ResourceSelector<TString> selector;
	};

	template<typename TString>
	bool Serialize(Serialization::IArchive& archive, SGameResourceSelector<TString>& value, tukk szName, tukk szLabel)
	{
		if (archive.caps(sxema2::IResourceCollectorArchive::ArchiveCaps))
			return archive(value.ToGameResource(), szName, szLabel);
		else
			return archive(value.selector, szName, szLabel);
	}
}
