// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <AssetSystem/AssetType.h>
#include <AssetSystem/EditableAsset.h>
#include "SubstanceCommon.h"
class QMenu;
namespace EditorSubstance
{
	namespace AssetTypes
	{

		class CSubstanceArchiveType : public CAssetType
		{
		public:
			DECLARE_ASSET_TYPE_DESC(CSubstanceArchiveType);

			class SSubstanceArchiveCache
			{
				friend class CSubstanceArchiveType;
			public:
				class Graph
				{
					friend class CSubstanceArchiveType;
				public:
					Graph();
					const std::vector<string>& GetOutputNames() const { return m_outputs; }
					const std::vector<SSubstanceOutput>& GetOutputsConfiguration() const { return m_outputsConfiguration; }
					const Vec2i GetResolution() const;
					void Serialize(Serialization::IArchive& ar);
				protected:
					string m_name;
					std::vector<string> m_outputs;
					std::vector<SSubstanceOutput> m_outputsConfiguration;
					i32 m_resolutionX;
					i32 m_resolutionY;
				};
				const Graph& GetGraph(const string& name) const;
				const bool HasGraph(const string& name) const;
				const std::vector<string> GetGraphNames();
				const std::vector<const Graph*> GetGraphs();
				void Serialize(Serialization::IArchive& ar);
			protected:
				std::map<string /*name*/, Graph> m_graphs;

			};

			virtual tukk GetTypeName() const override { return "SubstanceDefinition"; }
			virtual tukk GetUiTypeName() const override { return QT_TR_NOOP("Substance Archive"); }
			virtual bool IsImported() const override { return true; }
			virtual bool CanBeEdited() const override { return true; }
			virtual CAssetEditor* Edit(CAsset* asset) const override;
			virtual DrxIcon GetIcon() const override;
			virtual bool HasThumbnail() const override { return false; }
			virtual void AppendContextMenuActions(const std::vector<CAsset*>& assets, CAbstractMenu* menu) const override;
			virtual tukk GetFileExtension() const override { return "sbsar"; }

			//////////////////////////////////////////////////////////////////////////
			void SetArchiveCache(CEditableAsset* editableAsset, const std::map<string, std::vector<string>>& archiveData) const;
			SSubstanceArchiveCache GetArchiveCache(CAsset* asset) const;
			void SetGraphOutputsConfiguration(CEditableAsset* editableAsset, const string& name, const std::vector<SSubstanceOutput>& outputs, const Vec2i& resolution) const;
		private:
			void SaveGraphCache(CEditableAsset* editableAsset, const CSubstanceArchiveType::SSubstanceArchiveCache& cache) const;
		};
	}
}
