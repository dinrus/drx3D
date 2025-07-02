// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

//! Interface for game effect render elements.
//! Designed to be instantiated in game code, and called from the CREGameEffect within the engine.
//! This then allows render elements to be created in game code as well as in the engine.
struct IREGameEffect
{
	virtual ~IREGameEffect(){}
};

//! Render element that uses the IREGameEffect interface for its functionality.
class CREGameEffect : public CRenderElement
{
public:

	CREGameEffect();
	~CREGameEffect();

	// CREGameEffect interface
	inline void           SetPrivateImplementation(IREGameEffect* pImpl) { m_pImpl = pImpl; }
	inline IREGameEffect* GetPrivateImplementation() const               { return m_pImpl; }

	virtual void          GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}
private:

	IREGameEffect* m_pImpl; //!< Implementation of of render element.

};