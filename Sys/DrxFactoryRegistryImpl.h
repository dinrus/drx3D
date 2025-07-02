// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Extension/IDrxFactoryRegistryImpl.h>
#include <drx3D/CoreX/Extension/IDrxFactory.h>

#include <vector>

class CDrxFactoryRegistryImpl : public IDrxFactoryRegistryImpl
{
public:
	virtual IDrxFactory* GetFactory(tukk cname) const;
	virtual IDrxFactory* GetFactory(const DrxClassID& cid) const;
	virtual void         IterateFactories(const DrxInterfaceID& iid, IDrxFactory** pFactories, size_t& numFactories) const;

	virtual void         RegisterCallback(IDrxFactoryRegistryCallback* pCallback);
	virtual void         UnregisterCallback(IDrxFactoryRegistryCallback* pCallback);

	virtual void         RegisterFactories(const SRegFactoryNode* pFactories);
	virtual void         UnregisterFactories(const SRegFactoryNode* pFactories);

	virtual void         UnregisterFactory(IDrxFactory* const pFactory);

public:
	static CDrxFactoryRegistryImpl& Access();

private:
	struct FactoryByCName
	{
		tukk  m_cname;
		IDrxFactory* m_ptr;

		FactoryByCName(tukk cname) : m_cname(cname), m_ptr(0) { assert(m_cname); }
		FactoryByCName(IDrxFactory* ptr) : m_cname(ptr ? ptr->GetName() : 0), m_ptr(ptr) { assert(m_cname && m_ptr); }
		bool operator<(const FactoryByCName& rhs) const { return strcmp(m_cname, rhs.m_cname) < 0; }
	};
	typedef std::vector<FactoryByCName>      FactoriesByCName;
	typedef FactoriesByCName::iterator       FactoriesByCNameIt;
	typedef FactoriesByCName::const_iterator FactoriesByCNameConstIt;

	struct FactoryByCID
	{
		DrxClassID   m_cid;
		IDrxFactory* m_ptr;

		FactoryByCID(const DrxClassID& cid) : m_cid(cid), m_ptr(0) {}
		FactoryByCID(IDrxFactory* ptr) : m_cid(ptr ? ptr->GetClassID() : DrxGUID::Null()), m_ptr(ptr) { assert(m_ptr); }
		bool operator<(const FactoryByCID& rhs) const { return m_cid < rhs.m_cid; }
	};
	typedef std::vector<FactoryByCID>      FactoriesByCID;
	typedef FactoriesByCID::iterator       FactoriesByCIDIt;
	typedef FactoriesByCID::const_iterator FactoriesByCIDConstIt;

	struct FactoryByIID
	{
		DrxInterfaceID m_iid;
		IDrxFactory*   m_ptr;

		FactoryByIID(DrxInterfaceID iid, IDrxFactory* pFactory) : m_iid(iid), m_ptr(pFactory) {}
		bool operator<(const FactoryByIID& rhs) const { if (m_iid != rhs.m_iid) return m_iid < rhs.m_iid; return m_ptr < rhs.m_ptr; }
	};
	typedef std::vector<FactoryByIID>      FactoriesByIID;
	typedef FactoriesByIID::iterator       FactoriesByIIDIt;
	typedef FactoriesByIID::const_iterator FactoriesByIIDConstIt;
	struct LessPredFactoryByIIDOnly
	{
		bool operator()(const FactoryByIID& lhs, const FactoryByIID& rhs) const { return lhs.m_iid < rhs.m_iid; }
	};

	typedef std::vector<IDrxFactoryRegistryCallback*> Callbacks;
	typedef Callbacks::iterator                       CallbacksIt;
	typedef Callbacks::const_iterator                 CallbacksConstIt;

private:
	CDrxFactoryRegistryImpl();
	~CDrxFactoryRegistryImpl();

	bool GetInsertionPos(IDrxFactory* pFactory, FactoriesByCNameIt& itPosForCName, FactoriesByCIDIt& itPosForCID);
	void UnregisterFactoryInternal(IDrxFactory* const pFactory);

private:
	static CDrxFactoryRegistryImpl s_registry;

private:
	mutable DrxReadModifyLock m_guard;

	FactoriesByCName          m_byCName;
	FactoriesByCID            m_byCID;
	FactoriesByIID            m_byIID;

	Callbacks                 m_callbacks;
};
