// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "DrxSerialization/Serializer.h"
#include "DrxSerialization/ClassFactory.h"
#include "SubstanceCommon.h"
#include "DrxSubstanceAPI.h"
#include "ISubstanceManager.h"
#include <typeindex>
#include "substance/framework/callbacks.h"

namespace SubstanceAir
{
	class PackageDesc;
	class GraphInstance;
	class Renderer;
	class OutputInstanceBase;
}

class CSubstancePreset;
class ISubstanceInstanceRenderer;

typedef  std::unordered_map < u32 /*crc*/, SubstanceAir::PackageDesc*> PackageMap;


class CSubstanceManager: public ISubstanceManager
{

	public:
		static CSubstanceManager* Instance();

		void CreateInstance(const string& archiveName, const string& instanceName, const string& instanceGraph, const std::vector<SSubstanceOutput>& outputs, const Vec2i& resolution) override;
		bool GetArchiveContents(const string& archiveName, std::map<string, std::vector<string>>& contents) override;
		bool IsRenderPending(const SubstanceAir::UInt id) const override;
		void RegisterInstanceRenderer(ISubstanceInstanceRenderer* renderer) override;		
		void GenerateOutputs(ISubstancePreset* preset, ISubstanceInstanceRenderer* renderer) override;
		SubstanceAir::UInt GenerateOutputsAsync(ISubstancePreset* preset, ISubstanceInstanceRenderer* renderer) override;
		SubstanceAir::UInt GenerateOutputsAsync(const std::vector<ISubstancePreset*>& preset, ISubstanceInstanceRenderer* renderer) override;

		SubstanceAir::GraphInstance* InstantiateGraph(const string& archiveName, const string& graphName);
		void RemovePresetInstance(CSubstancePreset* preset);
	protected:

		enum GenerateType
		{
			Sync,
			Async
		};

		SubstanceAir::PackageDesc* GetPackage( const string& archiveName);
		SubstanceAir::UInt GenerateOutputs(const std::vector<ISubstancePreset*>& preset, ISubstanceInstanceRenderer* renderer, GenerateType jobType);

		struct DrxSubstanceCallbacks : public SubstanceAir::RenderCallbacks
		{
			void outputComputed(SubstanceAir::UInt runUid, size_t userData, const SubstanceAir::GraphInstance* graphInstance, SubstanceAir::OutputInstanceBase* outputInstance);
			void outputComputed(SubstanceAir::UInt runUid, const SubstanceAir::GraphInstance* graphInstance, SubstanceAir::OutputInstanceBase* outputInstance);
		};

		CSubstanceManager();
		SubstanceAir::PackageDesc* LoadPackage(const string& archiveName);
		void PushPreset(CSubstancePreset* preset, ISubstanceInstanceRenderer* renderer);
	private:
		static CSubstanceManager* _instance;
		PackageMap m_loadedPackages;
		std::unordered_map<u32 /*crc*/, i32> m_refCount;
		SubstanceAir::Renderer* m_pRenderer;
		DrxSubstanceCallbacks* m_pCallbacks;
		std::map<std::type_index, ISubstanceInstanceRenderer* > m_renderers;
		friend void DRX_SUBSTANCE_API InitDrxSubstanceLib(IFileManipulator* fileManipulator);
};


