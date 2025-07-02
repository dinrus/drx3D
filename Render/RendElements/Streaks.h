// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/RendElements/OpticsElement.h>
#include <drx3D/Render/RendElements/AbstractMeshElement.h>
#include <drx3D/Render/RendElements/MeshUtil.h>

class CTexture;

class Streaks : public COpticsElement
{
	struct SShaderParams : COpticsElement::SShaderParamsBase
	{
		Vec4 meshCenterAndBrt = Vec4(ZERO);
	};

private:
	_smart_ptr<CTexture> m_pSpectrumTex;
	bool                 m_bUseSpectrumTex;

	i32                  m_nStreakCount;
	i32                  m_nColorComplexity;

	// noise strengths
	float m_fSizeNoiseStrength;
	float m_fThicknessNoiseStrength;
	float m_fSpacingNoiseStrength;

	float m_fThickness;
	i32   m_nNoiseSeed;
	bool  m_meshDirty;

protected:

	struct SSeparatedMesh : public AbstractMeshElement
	{
		SSeparatedMesh() 
		{}

		SSeparatedMesh(SSeparatedMesh&& other)
			: primitive(std::move(other.primitive))
		{}

		void GenMesh() override {};

		CRenderPrimitive             primitive;
	};

	std::vector<SSeparatedMesh>      m_separatedMeshList;
	std::vector<u16>              m_meshIndices;
	buffer_handle_t                  m_indexBuffer;
	CConstantBufferPtr               m_constantBuffer;

	CTexture*  GetTexture();

	void UpdateMeshes();
	void Invalidate() override { m_meshDirty = true; }

public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Streaks(tukk name);
	virtual ~Streaks();

#if defined(FLARES_SUPPORT_EDITING)
	void       InitEditorParamGroups(DynArray<FuncVariableGroup>& groups);
#endif
	EFlareType GetType() override            { return eFT_Streaks; }
	bool       PreparePrimitives(const SPreparePrimitivesContext& context) override;
	void       Load(IXmlNode* pNode) override;

	bool       GetEnableSpectrumTex() const  { return m_bUseSpectrumTex; }
	void       SetEnableSpectrumTex(bool b)  { m_bUseSpectrumTex = b; }

	CTexture*  GetSpectrumTex() const        { return m_pSpectrumTex; }
	void       SetSpectrumTex(CTexture* tex) { m_pSpectrumTex = tex; }

	i32        GetNoiseSeed() const          { return m_nNoiseSeed; }
	void       SetNoiseSeed(i32 seed)
	{
		m_nNoiseSeed = seed;
		m_meshDirty = true;
	}

	i32  GetStreakCount() const { return m_nStreakCount; }
	void SetStreakCount(i32 n)
	{
		m_nStreakCount = n;

		// ideally: resize(n) but due to some issue in orbis stl resize tries to call the (deleted) copy constructor.
		while (m_separatedMeshList.size() < n)
			m_separatedMeshList.push_back(SSeparatedMesh());

		m_meshDirty = true;
	}

	i32  GetColorComplexity() const { return m_nColorComplexity; }
	void SetColorComplexity(i32 n)
	{
		m_nColorComplexity = n;
		m_meshDirty = true;
	}

	float GetThickness() const { return m_fThickness; }
	void  SetThickness(float f)
	{
		m_fThickness = f;
		m_meshDirty = true;
	}

	float GetThicknessNoise() const { return m_fThicknessNoiseStrength; }
	void  SetThicknessNoise(float noise)
	{
		m_fThicknessNoiseStrength = noise;
		m_meshDirty = true;
	}

	float GetSizeNoise() const { return m_fSizeNoiseStrength; }
	void  SetSizeNoise(float noise)
	{
		m_fSizeNoiseStrength = noise;
		m_meshDirty = true;
	}

	float GetSpacingNoise() const { return m_fSpacingNoiseStrength; }
	void  SetSpacingNoise(float noise)
	{
		m_fSpacingNoiseStrength = noise;
		m_meshDirty = true;
	}
};
