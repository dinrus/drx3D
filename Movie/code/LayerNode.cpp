// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Movie/StdAfx.h>
#include <drx3D/Movie/LayerNode.h>

namespace
{
bool s_nodeParamsInitialized = false;
std::vector<CAnimNode::SParamInfo> s_nodeParams;

void AddSupportedParam(tukk sName, i32 paramId, EAnimValue valueType)
{
	CAnimNode::SParamInfo param;
	param.name = sName;
	param.paramType = paramId;
	param.valueType = valueType;
	s_nodeParams.push_back(param);
}
};

CLayerNode::CLayerNode(i32k id) : CAnimNode(id), m_bInit(false), m_bPreVisibility(true)
{
	CLayerNode::Initialize();
}

void CLayerNode::Initialize()
{
	if (!s_nodeParamsInitialized)
	{
		s_nodeParamsInitialized = true;
		s_nodeParams.reserve(1);
		AddSupportedParam("Visibility", eAnimParamType_Visibility, eAnimValue_Bool);
	}
}

void CLayerNode::Animate(SAnimContext& animContext)
{
	bool bVisibilityModified = false;

	i32 trackCount = NumTracks();

	for (i32 paramIndex = 0; paramIndex < trackCount; paramIndex++)
	{
		CAnimParamType paramType = m_tracks[paramIndex]->GetParameterType();
		IAnimTrack* pTrack = m_tracks[paramIndex];

		if (pTrack->GetNumKeys() == 0)
		{
			continue;
		}

		if (pTrack->GetFlags() & IAnimTrack::eAnimTrackFlags_Disabled)
		{
			continue;
		}

		if (pTrack->IsMasked(animContext.trackMask))
		{
			continue;
		}

		switch (paramType.GetType())
		{
		case eAnimParamType_Visibility:
			if (!animContext.bResetting)
			{
				IAnimTrack* visTrack = pTrack;
				TMovieSystemValue value = visTrack->GetValue(animContext.time);

				bool bVisible = stl::get<bool>(value);
				if (m_bInit)
				{
					if (bVisible != m_bPreVisibility)
					{
						m_bPreVisibility = bVisible;
						bVisibilityModified = true;
					}
				}
				else
				{
					m_bInit = true;
					m_bPreVisibility = bVisible;
					bVisibilityModified = true;
				}

			}

			break;
		}

		// Layer entities visibility control
		if (bVisibilityModified)
		{
			// This is for game mode, in case of the layer data being exported.
			if (gEnv->pEntitySystem)
			{
				gEnv->pEntitySystem->EnableLayer(GetName(), m_bPreVisibility);
			}
		}
	}
}

void CLayerNode::CreateDefaultTracks()
{
	CreateTrack(eAnimParamType_Visibility);
}

void CLayerNode::OnReset()
{
	m_bInit = false;
}

void CLayerNode::Activate(bool bActivate)
{
	CAnimNode::Activate(bActivate);
}

void CLayerNode::Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks)
{
	CAnimNode::Serialize(xmlNode, bLoading, bLoadEmptyTracks);
}

u32 CLayerNode::GetParamCount() const
{
	return s_nodeParams.size();
}

CAnimParamType CLayerNode::GetParamType(u32 nIndex) const
{
	if (nIndex < s_nodeParams.size())
	{
		return s_nodeParams[nIndex].paramType;
	}

	return eAnimParamType_Invalid;
}

bool CLayerNode::GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const
{
	for (u32 i = 0; i < s_nodeParams.size(); i++)
	{
		if (s_nodeParams[i].paramType == paramId)
		{
			info = s_nodeParams[i];
			return true;
		}
	}

	return false;
}
