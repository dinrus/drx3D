// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PROCEDURAL_CLIP_FACTORY__H__
#define __PROCEDURAL_CLIP_FACTORY__H__

#include "IDrxMannequinProceduralClipFactory.h"

class CProceduralClipFactory
	: public IProceduralClipFactory
{
public:
	CProceduralClipFactory();
	virtual ~CProceduralClipFactory();

	// IProceduralClipFactory
	virtual tukk          FindTypeName(const THash& typeNameHash) const;

	virtual size_t               GetRegisteredCount() const;
	virtual THash                GetTypeNameHashByIndex(const size_t index) const;
	virtual tukk          GetTypeNameByIndex(const size_t index) const;

	virtual bool                 Register(tukk const typeName, const SProceduralClipFactoryRegistrationInfo& registrationInfo);

	virtual IProceduralParamsPtr CreateProceduralClipParams(tukk const typeName) const;
	virtual IProceduralParamsPtr CreateProceduralClipParams(const THash& typeNameHash) const;

	virtual IProceduralClipPtr   CreateProceduralClip(tukk const typeName) const;
	virtual IProceduralClipPtr   CreateProceduralClip(const THash& typeNameHash) const;
	// ~IProceduralClipFactory

private:
	const SProceduralClipFactoryRegistrationInfo* FindRegistrationInfo(const THash& typeNameHash) const;

private:
	typedef VectorMap<THash, string> THashToNameMap;
	THashToNameMap m_typeHashToName;

	typedef VectorMap<THash, SProceduralClipFactoryRegistrationInfo> TTypeHashToRegistrationInfoMap;
	TTypeHashToRegistrationInfoMap m_typeHashToRegistrationInfo;
};

#endif
