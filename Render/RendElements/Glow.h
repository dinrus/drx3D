// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/RendElements/OpticsElement.h>
#include <drx3D/Render/RendElements/AbstractMeshElement.h>
#include <drx3D/Render/RendElements/MeshUtil.h>

class Glow : public COpticsElement, public AbstractMeshElement
{
private:
	struct SShaderParams : COpticsElement::SShaderParamsBase
	{
		Vec4 meshCenterAndBrt;
		Vec4 lumaParams;
	};

	static float     compositionBufRatio;
	CRenderPrimitive m_primitive;

protected:
	float            m_fFocusFactor;
	float            m_fPolyonFactor;
	float            m_fGamma;

protected:
	void         GenMesh() override;
	void         Invalidate() override
	{
		m_meshDirty = true;
	}

public:
	Glow(tukk name);

#if defined(FLARES_SUPPORT_EDITING)
	void InitEditorParamGroups(DynArray<FuncVariableGroup>& groups);
#endif

	EFlareType GetType() override { return eFT_Glow; }
	void       Render(CShader* shader, Vec3 vSrcWorldPos, Vec3 vSrcProjPos, SAuxParams& aux);
	bool       PreparePrimitives(const SPreparePrimitivesContext& context) override;
	void       Load(IXmlNode* pNode) override;

	void       SetColor(ColorF t)
	{
		COpticsElement::SetColor(t);
		m_meshDirty = true;
	}
	float GetFocusFactor() const { return m_fFocusFactor; }
	void  SetFocusFactor(float f)
	{
		m_fFocusFactor = f;
	}
	i32  GetPolygonFactor() const { return (i32)m_fPolyonFactor; }
	void SetPolygonFactor(i32 f)
	{
		if (f < 0)
			f = 0;
		else if (f > 128)
			f = 128;
		if (m_fPolyonFactor != f)
		{
			m_fPolyonFactor = (float)f;
			m_meshDirty = true;
		}
	}
	float GetGamma() const { return m_fGamma; }
	void  SetGamma(float gamma)
	{
		m_fGamma = (float)gamma;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this) + GetMeshDataSize());
	}
};
