// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"  // TSmartPtr

class CMaterial;

class CMaterialSettings
{
public:
	typedef std::function<void ()> MaterialCallback;

	CMaterialSettings();
	virtual ~CMaterialSettings();

	void             SetMaterialChangedCallback(const MaterialCallback& callback);

	CMaterial*       GetMaterial();
	const CMaterial* GetMaterial() const;

	string           GetMaterialName() const;
	void             SetMaterial(const string& materialName);

	void             Serialize(yasli::Archive& ar);
private:
	void             SetMaterial(CMaterial* pMaterial);

	MaterialCallback     m_materialChangedCallback;
	TSmartPtr<CMaterial> m_pMaterial;
};

