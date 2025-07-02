// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/IFacialAnimation.h>
#include <drx3D/Animation/FaceEffector.h>

class CFacialModel;

//////////////////////////////////////////////////////////////////////////
// FacialState represents full state of all end effectors.
//////////////////////////////////////////////////////////////////////////
class CFaceState : public IFaceState, public _i_reference_target_t
{
public:
	CFaceState(CFacialModel* pFacialModel) { m_pFacialModel = pFacialModel; };

	//////////////////////////////////////////////////////////////////////////
	// IFaceState
	//////////////////////////////////////////////////////////////////////////
	virtual float GetEffectorWeight(i32 nIndex)
	{
		if (nIndex >= 0 && nIndex < (i32)m_weights.size())
			return GetWeight(nIndex);
		return 0;
	}
	virtual void SetEffectorWeight(i32 nIndex, float fWeight)
	{
		if (nIndex >= 0 && nIndex < (i32)m_weights.size())
			SetWeight(nIndex, fWeight);
	}
	//////////////////////////////////////////////////////////////////////////

	CFacialModel* GetModel() const { return m_pFacialModel; };

	float         GetWeight(i32 nIndex) const
	{
		assert(nIndex >= 0 && nIndex < (i32)m_weights.size());
		return m_weights[nIndex];
	}
	void SetWeight(i32 nIndex, float fWeight)
	{
		assert(nIndex >= 0 && nIndex < (i32)m_weights.size());
		m_weights[nIndex] = fWeight;
	}
	void SetWeight(IFacialEffector* pEffector, float fWeight)
	{
		i32 nIndex = ((CFacialEffector*)pEffector)->m_nIndexInState;
		if (nIndex >= 0 && nIndex < (i32)m_weights.size())
			m_weights[nIndex] = fWeight;
	}
	float GetBalance(i32 nIndex) const
	{
		assert(nIndex >= 0 && nIndex < (i32)m_balance.size());
		return m_balance[nIndex];
	}
	void SetBalance(i32 nIndex, float fBalance)
	{
		assert(nIndex >= 0 && nIndex < (i32)m_balance.size());
		m_balance[nIndex] = fBalance;
	}
	void   Reset();

	void   SetNumWeights(i32 n);
	i32    GetNumWeights() { return m_weights.size(); };

	size_t SizeOfThis()
	{
		return sizeofVector(m_weights) * sizeof(float) + sizeofVector(m_balance);
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_weights);
		pSizer->AddObject(m_balance);
	}

public:
	// Weights of the effectors in state.
	std::vector<float> m_weights;
	// Balances of the effectors in state.
	std::vector<float> m_balance;
	CFacialModel*      m_pFacialModel;
};
