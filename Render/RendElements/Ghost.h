// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/RendElements/OpticsElement.h>
#include <drx3D/Render/OpticsGroup.h>
#include <drx3D/Render/RendElements/AbstractMeshElement.h>

class CLensGhost : public COpticsElement
{
	struct SShaderParams : COpticsElement::SShaderParamsBase
	{
		Vec4 meshCenterAndBrt;
		Vec4 baseTexSize;
		Vec4 ghostTileInfo;
		Vec4 color;
	};

	_smart_ptr<CTexture> m_pTex;
	Vec4                 m_vTileDefinition;
	CRenderPrimitive     m_primitive;

protected:
#if defined(FLARES_SUPPORT_EDITING)
	void InitEditorParamGroups(DynArray<FuncVariableGroup>& groups);
#endif

public:

	CLensGhost(tukk name, CTexture* externalTex = NULL);

	Matrix34 mx33to34(Matrix33& mx)
	{
		return Matrix34(
		  mx.m00, mx.m01, mx.m02, 0,
		  mx.m10, mx.m11, mx.m12, 0,
		  mx.m20, mx.m21, mx.m22, 0);
	}

	EFlareType GetType() override { return eFT_Ghost; }
	bool       PreparePrimitives(const SPreparePrimitivesContext& context) override;
	void       Load(IXmlNode* pNode) override;

	CTexture*  GetTexture();
	void       SetTexture(CTexture* tex) { m_pTex = tex; }

	i32        GetTileIndex()            { return (i32)m_vTileDefinition.w; }
	i32        GetTotalTileCount()       { return (i32)m_vTileDefinition.z; }
	i32        GetTileCountX()           { return (i32)m_vTileDefinition.x; }
	i32        GetTileCountY()           { return (i32)m_vTileDefinition.y; }

	void       SetTileIndex(i32 idx)
	{
		if (idx >= 0 && idx <= GetTotalTileCount())
			m_vTileDefinition.w = (float)idx;
	}

	void SetTileCountX(i32 n)
	{
		i32 totalCount = GetTotalTileCount();
		if (totalCount > n * GetTileCountY())
			totalCount = n * GetTileCountY();

		SetTileDefinition(n, GetTileCountY(), totalCount, GetTileIndex());
	}

	void SetTileCountY(i32 n)
	{
		i32 totalCount = GetTotalTileCount();
		if (totalCount > GetTileCountX() * n)
			totalCount = GetTileCountX() * n;

		SetTileDefinition(GetTileCountX(), n, totalCount, GetTileIndex());
	}

	void SetTotalTileCount(i32 t)
	{
		SetTileDefinition(GetTileCountX(), GetTileCountY(), t, GetTileIndex());
	}

	void SetTileDefinition(i32 countX, i32 countY, i32 totalCount, i32 index)
	{
		if (totalCount > countX * countY)
			return;

		m_vTileDefinition.x = (float)countX;
		m_vTileDefinition.y = (float)countY;
		m_vTileDefinition.z = (float)totalCount;
		m_vTileDefinition.w = (float)index;
	}

	void SetTileDefinition(Vec4 df)
	{
		SetTileDefinition((i32)df.x, (i32)df.y, (i32)df.z, (i32)df.w);
	}
};

class CMultipleGhost : public COpticsGroup
{
protected:
	_smart_ptr<CTexture> m_pTex;

	Vec2                 m_vRange;
	float                m_fXOffsetNoise;
	float                m_fYOffsetNoise;
	Vec2                 m_vPositionFactor;
	Vec2                 m_vPositionOffset;
	i32                  m_nCount;
	i32                  m_nRandSeed;

	float                m_fSizeNoise;
	float                m_fBrightnessNoise;
	float                m_fColorNoise;

	bool                 m_bContentDirty : 1;

protected:
	void InitEditorParamGroups(DynArray<FuncVariableGroup>& groups);

public:
	CMultipleGhost(tukk name) :
		COpticsGroup(name),
		m_nCount(0),
		m_nRandSeed(0),
		m_bContentDirty(true),
		m_fXOffsetNoise(0),
		m_fYOffsetNoise(0),
		m_fSizeNoise(0.4f),
		m_fColorNoise(0.3f),
		m_fBrightnessNoise(0.3f)
	{
		m_vRange.set(0.1f, 0.7f);
		m_vPositionFactor.set(1, 1);
		m_vPositionOffset.set(0, 0);
		SetSize(0.04f);
		SetBrightness(0.3f);
		SetCount(20);
	}

public:
	void       GenGhosts(const SAuxParams& aux);

	EFlareType GetType() override               { return eFT_MultiGhosts; }
	bool       IsGroup() const override         { return false; }
	i32        GetElementCount() const override { return 0; }
	bool       PreparePrimitives(const SPreparePrimitivesContext& context) override;
	void       Load(IXmlNode* pNode) override;

	CTexture*  GetTexture() const { return m_pTex; }
	void       SetTexture(CTexture* tex)
	{
		m_pTex = tex;
		m_bContentDirty = true;
	}

	float GetXOffsetNoise() const { return m_fXOffsetNoise; }
	void  SetXOffsetNoise(float x)
	{
		m_fXOffsetNoise = x;
		m_bContentDirty = true;
	}

	float GetYOffsetNoise() const { return m_fYOffsetNoise; }
	void  SetYOffsetNoise(float y)
	{
		m_fYOffsetNoise = y;
		m_bContentDirty = true;
	}

	Vec2 GetPositionFactor() const { return m_vPositionFactor; }
	void SetPositionFactor(Vec2 positionFactor)
	{
		m_vPositionFactor = positionFactor;
		m_bContentDirty = true;
	}

	Vec2 GetPositionOffset() const { return m_vPositionOffset; }
	void SetPositionOffset(Vec2 offsets)
	{
		m_vPositionOffset = offsets;
		m_bContentDirty = true;
	}

	Vec2 GetRange() const { return m_vRange; }
	void SetRange(Vec2 range)
	{
		m_vRange = range;
		m_bContentDirty = true;
	}

	i32  GetCount() const { return m_nCount; }
	void SetCount(i32 count)
	{
		if (count < 0)
			return;
		m_nCount = count;
		RemoveAll();
		for (i32 i = 0; i < count; i++)
		{
			CLensGhost* ghost = new CLensGhost("SubGhost");
			ghost->SetAutoRotation(true);
			ghost->SetAspectRatioCorrection(true);
			ghost->SetOccBokehEnabled(true);
			ghost->SetSensorSizeFactor(1);
			ghost->SetSensorBrightnessFactor(1);
			Add(ghost);
		}
		m_bContentDirty = true;
	}

	i32  GetRandSeed() const { return m_nRandSeed; }
	void SetRandSeed(i32 n)
	{
		m_nRandSeed = n;
		m_bContentDirty = true;
	}

	float GetSizeNoise() const { return m_fSizeNoise; }
	void  SetSizeNoise(float n)
	{
		m_fSizeNoise = n;
		m_bContentDirty = true;
	}

	float GetBrightnessNoise() const { return m_fBrightnessNoise; }
	void  SetBrightnessNoise(float n)
	{
		m_fBrightnessNoise = n;
		m_bContentDirty = true;
	}

	float GetColorNoise() const { return m_fColorNoise; }
	void  SetColorNoise(float n)
	{
		m_fColorNoise = n;
		m_bContentDirty = true;
	}
};
