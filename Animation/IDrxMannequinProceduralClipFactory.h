// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __I_PROCEDURAL_CLIP_FACTORY__H__
#define __I_PROCEDURAL_CLIP_FACTORY__H__

#include <drx3D/Animation/IDrxMannequinDefs.h> // Needed for SCRCRef

class IProceduralClip;
DECLARE_SHARED_POINTERS(IProceduralClip);

struct IProceduralParams;
DECLARE_SHARED_POINTERS(IProceduralParams);

struct SProceduralClipFactoryRegistrationInfo
{
	typedef IProceduralClipPtr (*   CreateProceduralClipFunction)();
	typedef IProceduralParamsPtr (* CreateProceduralParamsFunction)();

	SProceduralClipFactoryRegistrationInfo()
		: pProceduralClipCreator(NULL)
		, pProceduralParamsCreator(NULL)
	{
	}

	SProceduralClipFactoryRegistrationInfo(const CreateProceduralClipFunction& pClipCreator, const CreateProceduralParamsFunction pParamsCreator)
		: pProceduralClipCreator(pClipCreator)
		, pProceduralParamsCreator(pParamsCreator)
	{
	}

	CreateProceduralClipFunction   pProceduralClipCreator;
	CreateProceduralParamsFunction pProceduralParamsCreator;
};

template<typename T>
inline IProceduralClipPtr CreateProceduralClip()
{
	return std::make_shared<T>();
}

template<typename T>
inline IProceduralParamsPtr CreateProceduralParams()
{
	return std::make_shared<T>();
}

struct IProceduralClipFactory
{
	typedef SCRCRef<0, SCRCRefHash_CRC32> THash;

	virtual ~IProceduralClipFactory() {}

	virtual tukk FindTypeName(const THash& typeNameHash) const = 0;

	virtual size_t      GetRegisteredCount() const = 0;
	virtual THash       GetTypeNameHashByIndex(const size_t index) const = 0;
	virtual tukk GetTypeNameByIndex(const size_t index) const = 0;

	template<typename TProceduralClip, typename TProceduralParams>
	inline bool Register(tukk const typeName)
	{
		SProceduralClipFactoryRegistrationInfo registrationInfo;
		registrationInfo.pProceduralClipCreator = &::CreateProceduralClip<TProceduralClip>;
		registrationInfo.pProceduralParamsCreator = &::CreateProceduralParams<TProceduralParams>;

		const bool registerSuccess = Register(typeName, registrationInfo);
		return registerSuccess;
	}

	virtual bool                 Register(tukk const typeName, const SProceduralClipFactoryRegistrationInfo& registrationInfo) = 0;

	virtual IProceduralParamsPtr CreateProceduralClipParams(tukk const typeName) const = 0;
	virtual IProceduralParamsPtr CreateProceduralClipParams(const THash& typeNameHash) const = 0;

	virtual IProceduralClipPtr   CreateProceduralClip(tukk const typeName) const = 0;
	virtual IProceduralClipPtr   CreateProceduralClip(const THash& typeNameHash) const = 0;
};

struct SProceduralClipRegistrationHelperBase
{
	SProceduralClipRegistrationHelperBase(SProceduralClipRegistrationHelperBase** pFirst)
		: m_pNext(NULL)
	{
		m_pNext = *pFirst;
		* pFirst = this;
	}

	virtual ~SProceduralClipRegistrationHelperBase() {}

	virtual void Register(IProceduralClipFactory& proceduralClipFactory) const = 0;

	SProceduralClipRegistrationHelperBase* m_pNext;
};

template<typename TClipType>
struct SProceduralClipRegistrationHelper
	: public SProceduralClipRegistrationHelperBase
{
	SProceduralClipRegistrationHelper(SProceduralClipRegistrationHelperBase** pFirst, tukk const name)
		: SProceduralClipRegistrationHelperBase(pFirst)
		, m_name(name)
	{
	}

	virtual void Register(IProceduralClipFactory& proceduralClipFactory) const
	{
		typedef typename TClipType::TParamsType TParamsType;
		proceduralClipFactory.Register<TClipType, TParamsType>(m_name);
	}

	tukk m_name;
};

inline SProceduralClipRegistrationHelperBase** GetModuleMannequinProceduralClipAutoRegistrationListPtr()
{
	static SProceduralClipRegistrationHelperBase* s_pModuleMannequinProceduralClipAutoRegistrationList = NULL;
	return &s_pModuleMannequinProceduralClipAutoRegistrationList;
}

#define REGISTER_PROCEDURAL_CLIP(clipType, name) SProceduralClipRegistrationHelper<clipType> clipType ## _RegistrationHelper(GetModuleMannequinProceduralClipAutoRegistrationListPtr(), name);

namespace mannequin
{
inline void RegisterProceduralClips(IProceduralClipFactory& proceduralClipFactory, const SProceduralClipRegistrationHelperBase* const pFirst)
{
	const SProceduralClipRegistrationHelperBase* pCurrent = pFirst;
	while (pCurrent)
	{
		pCurrent->Register(proceduralClipFactory);
		pCurrent = pCurrent->m_pNext;
	}
}

inline void RegisterProceduralClipsForModule(IProceduralClipFactory& proceduralClipFactory)
{
	SProceduralClipRegistrationHelperBase** ppModuleMannequinProceduralClipAutoRegistrationList = GetModuleMannequinProceduralClipAutoRegistrationListPtr();
	DRX_ASSERT(ppModuleMannequinProceduralClipAutoRegistrationList != NULL);

	RegisterProceduralClips(proceduralClipFactory, *ppModuleMannequinProceduralClipAutoRegistrationList);

	*ppModuleMannequinProceduralClipAutoRegistrationList = NULL;
}
}

#endif
