// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************

-------------------------------------------------------------------------
История:
- 15:05:2009   Created by Federico Rebora

*************************************************************************/

#include <drx3D/Act/StdAfx.h>

#include <drx3D/Act/ColorGradientUpr.h>
#include <drx3D/FlowGraph/IFlowBaseNode.h>

CColorGradientUpr::CColorGradientUpr()
{
}

void CColorGradientUpr::Reset()
{
	stl::free_container(m_colorGradientsToLoad);

	for (std::vector<LoadedColorGradient>::iterator it = m_currentGradients.begin(), itEnd = m_currentGradients.end(); it != itEnd; ++ it)
	{
		LoadedColorGradient& cg = *it;
		if (cg.m_layer.m_texID >= 0)
		{
			GetColorGradingController().UnloadColorChart(cg.m_layer.m_texID);
		}
	}

	stl::free_container(m_currentGradients);
}

void CColorGradientUpr::Serialize(TSerialize serializer)
{
	if(serializer.IsReading())
		Reset();

	serializer.BeginGroup("ColorGradientUpr");
	{
		i32 numToLoad = (i32)m_colorGradientsToLoad.size();
		i32 numLoaded = (i32)m_currentGradients.size();
		i32 numGradients = numToLoad + numLoaded;
		serializer.Value("ColorGradientCount", numGradients);
		if(serializer.IsWriting())
		{
			for(i32 i=0; i<numToLoad; ++i)
			{
				LoadingColorGradient& gradient = m_colorGradientsToLoad[i];
				serializer.BeginGroup("ColorGradient");
				serializer.Value("FilePath", gradient.m_filePath);
				serializer.Value("FadeInTime", gradient.m_fadeInTimeInSeconds);
				serializer.EndGroup();
			}
			for(i32 i=0; i<numLoaded; ++i)
			{
				LoadedColorGradient& gradient = m_currentGradients[i];
				serializer.BeginGroup("ColorGradient");
				serializer.Value("FilePath", gradient.m_filePath);
				serializer.Value("BlendAmount", gradient.m_layer.m_blendAmount);
				serializer.Value("FadeInTime", gradient.m_fadeInTimeInSeconds);
				serializer.Value("ElapsedTime", gradient.m_elapsedTime);
				serializer.Value("MaximumBlendAmount", gradient.m_maximumBlendAmount);
				serializer.EndGroup();
			}
		}
		else
		{
			m_currentGradients.reserve(numGradients);
			for(i32 i=0; i<numGradients; ++i)
			{
				serializer.BeginGroup("ColorGradient");
				string filePath;
				float blendAmount = 1.0f;
				float fadeInTimeInSeconds = 0.0f;
				serializer.Value("FilePath", filePath);
				serializer.Value("BlendAmount", blendAmount);
				serializer.Value("FadeInTime", fadeInTimeInSeconds);
				i32k textureID = GetColorGradingController().LoadColorChart(filePath);
				LoadedColorGradient gradient(filePath, SColorChartLayer(textureID, blendAmount), fadeInTimeInSeconds);

				// Optional
				serializer.ValueWithDefault("ElapsedTime", gradient.m_elapsedTime, 0.0f);
				serializer.ValueWithDefault("MaximumBlendAmount", gradient.m_maximumBlendAmount, 1.0f);

				m_currentGradients.push_back(gradient);
				serializer.EndGroup();
			}
		}
		serializer.EndGroup();
	}
}

void CColorGradientUpr::TriggerFadingColorGradient(const string& filePath, const float fadeInTimeInSeconds)
{
	u32k numGradients = (i32) m_currentGradients.size();
	for (u32 currentGradientIndex = 0; currentGradientIndex < numGradients; ++currentGradientIndex)
	{
		m_currentGradients[currentGradientIndex].FreezeMaximumBlendAmount();
	}

	m_colorGradientsToLoad.push_back(LoadingColorGradient(filePath, fadeInTimeInSeconds));
}

void CColorGradientUpr::UpdateForThisFrame(const float frameTimeInSeconds)
{
	RemoveZeroWeightedLayers();
	
	LoadGradients();

	FadeInLastLayer(frameTimeInSeconds);
	FadeOutCurrentLayers();

	SetLayersForThisFrame();
}

void CColorGradientUpr::FadeInLastLayer(const float frameTimeInSeconds)
{
	if (m_currentGradients.empty())
	{
		return;
	}

	m_currentGradients.back().FadeIn(frameTimeInSeconds);
}

void CColorGradientUpr::FadeOutCurrentLayers()
{
	if (m_currentGradients.size() <= 1u)
	{
		return;
	}

	u32k numberofFadingOutGradients = (i32) m_currentGradients.size() - 1;
	const float fadingInLayerBlendAmount = m_currentGradients[numberofFadingOutGradients].m_layer.m_blendAmount;
	for (u32 index = 0; index < numberofFadingOutGradients; ++index)
	{
		m_currentGradients[index].FadeOut(fadingInLayerBlendAmount);
	}
}

void CColorGradientUpr::RemoveZeroWeightedLayers()
{
	std::vector<LoadedColorGradient>::iterator currentGradient = m_currentGradients.begin();

	while (currentGradient != m_currentGradients.end())
	{
		if (currentGradient->m_layer.m_blendAmount == 0.0f)
		{
			GetColorGradingController().UnloadColorChart(currentGradient->m_layer.m_texID);

			currentGradient = m_currentGradients.erase(currentGradient);
		}
     else
     {
       ++currentGradient;
     }
	}
}

void CColorGradientUpr::SetLayersForThisFrame()
{
	std::vector<SColorChartLayer> thisFrameLayers;

	u32k numberOfFadingInGradients = (i32) m_currentGradients.size();
	thisFrameLayers.reserve(numberOfFadingInGradients + thisFrameLayers.size());
	for (u32 index = 0; index < numberOfFadingInGradients; ++index)
	{
		thisFrameLayers.push_back(m_currentGradients[index].m_layer);
	}

	u32k numLayers = (u32) thisFrameLayers.size();
	const SColorChartLayer* pLayers = numLayers ? &thisFrameLayers.front() : 0;
	GetColorGradingController().SetLayers(pLayers, numLayers);
}

void CColorGradientUpr::LoadGradients()
{
	u32k numGradientsToLoad = (i32) m_colorGradientsToLoad.size();
	m_currentGradients.reserve(numGradientsToLoad +  m_currentGradients.size());
	for (u32 index = 0; index < numGradientsToLoad; ++index)
	{
		LoadedColorGradient loadedGradient = m_colorGradientsToLoad[index].Load(GetColorGradingController());
		
		m_currentGradients.push_back(loadedGradient);
	}

	m_colorGradientsToLoad.clear();
}

IColorGradingController& CColorGradientUpr::GetColorGradingController()
{
	return *gEnv->pRenderer->GetIColorGradingController();
}

CColorGradientUpr::LoadedColorGradient::LoadedColorGradient(const string& filePath, const SColorChartLayer& layer, const float fadeInTimeInSeconds)
: m_filePath(filePath)
, m_layer(layer)
, m_fadeInTimeInSeconds(fadeInTimeInSeconds)
, m_elapsedTime(0.0f)
, m_maximumBlendAmount(1.0f)
{

}

void CColorGradientUpr::LoadedColorGradient::FadeIn(const float frameTimeInSeconds)
{
	if (m_fadeInTimeInSeconds == 0.0f)
	{
		m_layer.m_blendAmount = 1.0f;

		return;
	}

	m_elapsedTime += frameTimeInSeconds;

	const float blendAmount = m_elapsedTime / m_fadeInTimeInSeconds;

	m_layer.m_blendAmount = min(blendAmount, 1.0f);
}

void CColorGradientUpr::LoadedColorGradient::FadeOut( const float blendAmountOfFadingInGradient )
{
	m_layer.m_blendAmount = m_maximumBlendAmount * (1.0f - blendAmountOfFadingInGradient);
}

void CColorGradientUpr::LoadedColorGradient::FreezeMaximumBlendAmount()
{
	m_maximumBlendAmount = m_layer.m_blendAmount;
}

CColorGradientUpr::LoadingColorGradient::LoadingColorGradient(const string& filePath, const float fadeInTimeInSeconds)
: m_filePath(filePath)
, m_fadeInTimeInSeconds(fadeInTimeInSeconds)
{

}

CColorGradientUpr::LoadedColorGradient CColorGradientUpr::LoadingColorGradient::Load(IColorGradingController& colorGradingController) const
{
	i32k textureID = colorGradingController.LoadColorChart(m_filePath);

	return LoadedColorGradient(m_filePath, SColorChartLayer(textureID, 1.0f), m_fadeInTimeInSeconds);
}

class CFlowNode_ColorGradient : public CFlowBaseNode<eNCT_Instanced>
{
public:
	enum InputPorts
	{
		eIP_Trigger,
	};
	static const SInputPortConfig inputPorts[];

	CFlowNode_ColorGradient(SActivationInfo* pActivationInformation);
	~CFlowNode_ColorGradient();

	virtual void         GetConfiguration(SFlowNodeConfig& config);
	virtual void         ProcessEvent(EFlowEvent event, SActivationInfo* pActivationInformation);
	virtual void         GetMemoryUsage(IDrxSizer* sizer) const;

	virtual IFlowNodePtr Clone(SActivationInfo* pActInfo) { return new CFlowNode_ColorGradient(pActInfo); }

	enum EInputPorts
	{
		eInputPorts_Trigger,
		eInputPorts_TexturePath,
		eInputPorts_TransitionTime,
		eInputPorts_Count,
	};

private:
	//	IGameEnvironment& m_environment;
	ITexture* m_pTexture;
};

const SInputPortConfig CFlowNode_ColorGradient::inputPorts[] =
{
	InputPortConfig_Void("Trigger",            _HELP("")),
	InputPortConfig<string>("tex_TexturePath", _HELP("Path to the Color Chart texture.")),
	InputPortConfig<float>("TransitionTime",   _HELP("Fade in time (Seconds).")),
	{ 0 },
};

CFlowNode_ColorGradient::CFlowNode_ColorGradient(SActivationInfo* pActivationInformation)
	: m_pTexture(NULL)
{
}

CFlowNode_ColorGradient::~CFlowNode_ColorGradient()
{
	SAFE_RELEASE(m_pTexture);
}

void CFlowNode_ColorGradient::GetConfiguration(SFlowNodeConfig& config)
{
	config.pInputPorts = inputPorts;
	config.SetCategory(EFLN_ADVANCED);
}

void CFlowNode_ColorGradient::ProcessEvent(EFlowEvent event, SActivationInfo* pActivationInformation)
{
	if (!gEnv->pRenderer)
	{
		return;
	}

	//Preload texture
	if (event == IFlowNode::eFE_PrecacheResources && m_pTexture == nullptr)
	{
		const string texturePath = GetPortString(pActivationInformation, eInputPorts_TexturePath);
		u32k COLORCHART_TEXFLAGS = FT_NOMIPS | FT_DONT_STREAM | FT_STATE_CLAMP;
		m_pTexture = gEnv->pRenderer->EF_LoadTexture(texturePath.c_str(), COLORCHART_TEXFLAGS);
	}

	if (event == IFlowNode::eFE_Activate && IsPortActive(pActivationInformation, eIP_Trigger))
	{
		const string texturePath = GetPortString(pActivationInformation, eInputPorts_TexturePath);
		const float timeToFade = GetPortFloat(pActivationInformation, eInputPorts_TransitionTime);
		CDrxAction::GetDrxAction()->GetColorGradientUpr()->TriggerFadingColorGradient(texturePath, timeToFade);
	}
}

void CFlowNode_ColorGradient::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->Add(*this);
}

REGISTER_FLOW_NODE("Image:ColorGradient", CFlowNode_ColorGradient);
