// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <vector>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/String/DrxString.h>

class CAutoLodSettings
{
public:
	struct sGlobalParams
	{
		sGlobalParams()
		{
			m_fViewreSolution = 26.6932144;
			m_iViewsAround = 12;
			m_iViewElevations = 3;
			m_fSilhouetteWeight = 5.0;
			m_fVertexWelding = 0.001;
			m_bCheckTopology = true;
			m_bObjectHasBase = false;
		}

		float m_fViewreSolution;
		i32   m_iViewsAround;
		i32   m_iViewElevations;
		float m_fSilhouetteWeight;
		float m_fVertexWelding;
		bool  m_bCheckTopology;
		bool  m_bObjectHasBase;

		void  Serialize(yasli::Archive& ar);
	};
	struct sNodeParam
	{
		sNodeParam()
		{
			m_bAutoGenerate = false;
			m_iLodCount = 3;
			m_fPercent = 0.5;
		}

		sNodeParam(bool bAutoGenerate, i32 iLodCount, float fPercent) :
			m_bAutoGenerate(bAutoGenerate),
			m_iLodCount(iLodCount),
			m_fPercent(fPercent)
		{

		}

		bool  m_bAutoGenerate;
		i32   m_iLodCount;
		float m_fPercent;

		void Serialize(yasli::Archive& ar);

		bool operator==(const sNodeParam& other) const;
	};
	struct sNode
	{
		string     m_nodeName;
		sNodeParam m_nodeParam;

		void       Serialize(yasli::Archive& ar);
	};

	void                Serialize(yasli::Archive& ar);

	sGlobalParams&      getGlobalParams();
	std::vector<sNode>& getNodeList();

	bool                IncludeNode(string name) const
	{
		for each (sNode node in m_nodeParams)
		{
			if (node.m_nodeName == name)
				return true;
		}
		return false;
	}

	sNodeParam GetNodeParam(string name) const
	{
		for each (sNode node in m_nodeParams)
		{
			if (node.m_nodeName == name)
				return node.m_nodeParam;
		}
		return sNodeParam();
	}

private:
	std::vector<sNode> m_nodeParams;
	sGlobalParams      m_globalParams;
};

