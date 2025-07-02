// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "../StdAfx.h"
#include "../SubstanceCommon.h"

#ifndef DRX_SUBSTANCE_STATIC
#ifdef RESOURCE_COMPILER
#include "../platform_implRC.inl"
#else
#include <drx3D/CoreX/Platform/platform_impl.inl>
#endif

#endif // !DRX_SUBSTANCE_STATIC

#include <drx3D/CoreX/Serialization/IArchive.h>
#include "../SubstancePreset.h"
#include "../SubstanceManager.h"
#include <drx3D/CoreX/Serialization/yasli/JSONIArchive.h>
#include <drx3D/CoreX/Serialization/yasli/JSONOArchive.h>

IFileManipulator* g_fileManipulator = nullptr;

std::vector<std::pair<string, SSubstanceOutput::ESubstanceOutputResolution>> resolutionNamesMap {
	{ "Default", SSubstanceOutput::Original },
	{ "1/2", SSubstanceOutput::Half },
	{ "1/4", SSubstanceOutput::Quarter },
	{ "1/8", SSubstanceOutput::Eighth },
};

void InitDrxSubstanceLib(IFileManipulator* fileManip)
{
	g_fileManipulator = fileManip;
}

namespace SubstanceSerialization {
	template<class T>
	bool LoadJsonFile(T& instance,const string& filename)
	{
		std::vector<char> content;
		Serialization::SStruct obj(instance);
		size_t size;
		if (!g_fileManipulator->ReadFile(filename, content, size, "r"))
			return false;
		yasli::JSONIArchive ia;
		if (!ia.open(content.data(), content.size()))
			return false;
		return ia(obj);
	}

	template<class T>
	bool SaveJsonFile(const string& filename, const T& instance)
	{
		Serialization::SStruct obj(instance);
		string absfilename = g_fileManipulator->GetAbsolutePath(filename);
		yasli::JSONOArchive oa;
		if (!oa(obj))
			return false;
		return oa.save(absfilename);
	}

	template bool LoadJsonFile<ISubstancePresetSerializer>(ISubstancePresetSerializer& instance, const string& filename);
	template bool SaveJsonFile<ISubstancePresetSerializer>(const string& filename, const ISubstancePresetSerializer& instance);

}

SSubstanceOutput::SSubstanceOutput()
	: enabled(true), preset(""), name(""), resolution(SSubstanceOutput::Original), flags(0)
{}

void SSubstanceOutput::SetAllSources(i32 value)
{
	channels[0].sourceId = channels[1].sourceId = channels[2].sourceId = channels[3].sourceId = value;
}

void SSubstanceOutput::SetAllChannels(i32 value)
{
	channels[0].channel = channels[1].channel = channels[2].channel = channels[3].channel = value;
}

void SSubstanceOutput::RemoveSource(i32 sourceID)
{
	sourceOutputs.erase(sourceOutputs.begin() + sourceID);
	for (SSubstanceOutputChannelMapping& info : channels)
	{
		if (info.sourceId == sourceID)
		{
			info.Reset();
		}
		else if (info.sourceId > sourceID)
		{
			info.sourceId -= 1;
		}
	}
}

bool SSubstanceOutput::IsValid()
{
	return !sourceOutputs.empty();
}

void SSubstanceOutput::UpdateState(const std::vector<string>& availableSourceOutputs)
{
	for (i32 i = sourceOutputs.size() - 1; i >= 0; i--)
	{
		std::vector<string>::const_iterator it = std::find_if(availableSourceOutputs.begin(), availableSourceOutputs.end(),
			[=](const string& val)
		{
			return val.CompareNoCase(sourceOutputs[i]) == 0;
		});
		if (it == availableSourceOutputs.end())
		{
			RemoveSource(i);
		}
	}
}

SSubstanceOutputChannelMapping::SSubstanceOutputChannelMapping()
	: sourceId(-1), channel(-1)
{}


void SSubstanceOutput::Serialize(Serialization::IArchive& ar)
{
	ar(enabled, "enabled");
	ar(name, "name");
	ar(preset, "preset");
	ar(resolution, "resolution");
	ar(sourceOutputs, "sourceoutputs");
	ar(channels, "mapping");
}

void SSubstanceOutputChannelMapping::Reset()
{
	channel = -1;
	sourceId = -1;
}

void SSubstanceOutputChannelMapping::Serialize(Serialization::IArchive& ar)
{
	ar(channel, "channel");
	ar(sourceId, "output");
}

#include <drx3D/CoreX/Serialization/Enum.h>
#include "SerializationSubstanceEnums.inl"


ISubstancePreset* ISubstancePreset::Load(const string& filePath)
{
	return CSubstancePreset::Load(filePath);
}

ISubstancePreset* ISubstancePreset::Instantiate(const string& archiveName, const string& graphName)
{
	return CSubstancePreset::Instantiate(archiveName, graphName);
}

