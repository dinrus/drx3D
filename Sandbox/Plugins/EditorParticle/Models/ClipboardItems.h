// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/ItemCollection.h>

namespace DrxParticleEditor {

class CNodeItem;

class CParticleGraphModel;

class CClipboardItemCollection : public DrxGraphEditor::CItemCollection
{
private:
	struct SConnection
	{
		i32 sourceNodeIndex;
		i32 sourceFeatureIndex;
		i32 targetNodeIndex;

		SConnection()
			: sourceNodeIndex(-1)
			, sourceFeatureIndex(-1)
			, targetNodeIndex(-1)
		{}

		void Serialize(Serialization::IArchive& archive);
	};

	struct SNode
	{
		float  positionX;
		float  positionY;

		string dataBuffer;

		SNode()
			: positionX(.0f)
			, positionY(.0f)
		{}

		void Serialize(Serialization::IArchive& archive);
	};

	struct SFeature
	{
		string groupName;
		string featureName;
		string dataBuffer;

		void   Serialize(Serialization::IArchive& archive);
	};

	typedef std::map<CNodeItem*, i32 /* index */> NodeIndexByInstance;
	typedef std::vector<CNodeItem*>                 NodesByIndex;

public:
	CClipboardItemCollection(CParticleGraphModel& model);

	virtual void Serialize(Serialization::IArchive& archive) override;

private:
	CParticleGraphModel& m_model;
};

}

