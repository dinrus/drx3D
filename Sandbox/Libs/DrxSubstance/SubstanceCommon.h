// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "DrxSerialization/Serializer.h"
#include "DrxSubstanceAPI.h"
#include "substance/framework/typedefs.h"



struct DRX_SUBSTANCE_API IFileManipulator
{
	virtual bool ReadFile(const string& filePath, std::vector<char>& buffer, size_t& readSize, const string& mode) = 0;
	virtual string GetAbsolutePath(const string& filename) const = 0;
};

extern IFileManipulator* g_fileManipulator;

void DRX_SUBSTANCE_API InitDrxSubstanceLib(IFileManipulator* fileManipulator);




struct DRX_SUBSTANCE_API SSubstanceOutputChannelMapping
{
	SSubstanceOutputChannelMapping();
	char sourceId; // if -1 channel is not used
	char channel;
	void Reset();
	virtual void Serialize(Serialization::IArchive& ar);
};

struct DRX_SUBSTANCE_API SSubstanceOutput
{
	enum ESubstanceOutputResolution {
		Original = 0,
		Half = 1,
		Quarter = 2,
		Eighth = 3,
	};

	SSubstanceOutput();
	void SetAllSources(i32 value = 0);
	void SetAllChannels(i32 value = -1);
	void RemoveSource(i32 sourceID);
	bool IsValid();
	void UpdateState(const std::vector<string>& availableSourceOutputs);
	bool enabled;
	string preset;
	string name;
	i32 flags;
	std::vector<string> sourceOutputs;
	ESubstanceOutputResolution resolution;
	SSubstanceOutputChannelMapping channels[4];

	virtual void Serialize(Serialization::IArchive& ar);
};


struct DRX_SUBSTANCE_API SSubstanceRenderData
{
	SSubstanceRenderData()
		: format(0)
		, useMips(false)
		, skipAlpha(false)
		, swapRG(false)
		, customData(0)
		, name("")
	{}
	SSubstanceOutput output;
	string name;
	u32 format;
	bool useMips;
	bool skipAlpha;
	bool swapRG;
	size_t customData;
};


DRX_SUBSTANCE_API extern std::vector<std::pair<string, SSubstanceOutput::ESubstanceOutputResolution>> resolutionNamesMap;

namespace SubstanceSerialization {
	template<class T> bool LoadJsonFile(T& instance, const string& filename);
	template<class T> bool SaveJsonFile(const string& filename, const T& instance);
	
	template<class T> bool Save(const string& filename, const T& instance)
	{
		return SaveJsonFile<T>(filename, instance);
	}
	
	template<class T> bool Load(T& instance, const string& filename)
	{
		return LoadJsonFile<T>(instance, filename);
	}
}

struct DRX_SUBSTANCE_API ISubstancePresetSerializer
{
	virtual void Serialize(Serialization::IArchive& ar) = 0;
};

class DRX_SUBSTANCE_API ISubstancePreset
{
public:
	virtual void Save() = 0;
	static ISubstancePreset* Load(const string& filePath);
	static ISubstancePreset* Instantiate(const string& archiveName, const string& graphName);
	virtual void Reload() = 0;
	virtual void Reset() = 0;
	virtual const string& GetGraphName() const = 0;
	virtual const string& GetSubstanceArchive() const = 0;
	virtual const string GetFileName() const = 0;
	virtual ISubstancePresetSerializer* GetSerializer() = 0;
	virtual std::vector<SSubstanceOutput>& GetOutputs() = 0;
	virtual const std::vector<string> GetInputImages() const = 0;
	virtual const std::vector<SSubstanceOutput>& GetDefaultOutputs() const = 0;
	virtual void SetGraphResolution(i32k& x, i32k& y) = 0;
	virtual const string GetOutputPath(const SSubstanceOutput* output) const = 0;
	virtual const SubstanceAir::UInt& GetInstanceID() const = 0;
	virtual ISubstancePreset* Clone() const = 0;
	virtual void Destroy() = 0;
};
