// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "Base.h"
#include "EditorSubstanceManager.h"
#include <drx3D/CoreX/Renderer/ITexture.h>

namespace EditorSubstance
{
	namespace Renderers
	{
		struct SGeneratedOutputData
		{
			string path;
			string texturePreset;
			string suffix;
			i32 flags;
		};



		typedef std::map<string, std::shared_ptr<SGeneratedOutputData>> OutputDataMap;
		typedef std::unordered_map<SubstanceAir::UInt, OutputDataMap> PresetOutputsDataMap;


		class CCompressedRenderer : public CInstanceRenderer
		{
		public:

			CCompressedRenderer();
			virtual void FillVirtualOutputRenderData(const ISubstancePreset* preset, const SSubstanceOutput& output, std::vector<SSubstanceRenderData>& renderData) override;
			virtual void FillOriginalOutputRenderData(const ISubstancePreset* preset, SSubstanceOutput& output, std::vector<SSubstanceRenderData>& renderData) override {};
			virtual void ProcessComputedOutputs() override;
			virtual void RemovePresetRenderData(ISubstancePreset* preset) override;
		protected:
			u32 GetOutputCompressedFormat(const EditorSubstance::EPixelFormat& format, bool useSRGB);
			void UpdateTexture(SubstanceAir::RenderResult* result, SGeneratedOutputData* data);
		private:
			void AttachOutputUserData(const ISubstancePreset* preset, const SSubstanceOutput& output, SSubstanceRenderData& renderData, i32k& flags);
			u32 TextureDataSize(u32 nWidth, u32 nHeight, u32 nDepth, u32 nMips, const ETEX_Format eTF) const;


			PresetOutputsDataMap m_PresetOutputDataMap;
		};


	} // END namespace Renderers
} // END namespace EditorSubstance


