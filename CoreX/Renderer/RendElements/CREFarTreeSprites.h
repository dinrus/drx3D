// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CREFarTreeSprites_H__
#define __CREFarTreeSprites_H__

class CREFarTreeSprites : public CRenderElement
{
public:
	PodArray<struct SVegetationSpriteInfo>* m_arrVegetationSprites[2][2];

	CREFarTreeSprites()
	{
		mfSetType(eDATA_FarTreeSprites);
		mfUpdateFlags(FCEF_TRANSFORM);
		for (i32 i = 0; i < 2; i++)
		{
			m_arrVegetationSprites[i][0] = NULL;
			m_arrVegetationSprites[i][1] = NULL;
		}
	}
	virtual bool mfDraw(CShader* ef, SShaderPass* sfm);
	virtual void mfPrepare(bool bCheckOverflow);

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}
};

#endif  // __CREFarTreeSprites_H__
