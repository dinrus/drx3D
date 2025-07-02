// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/TestExtensions.h>

#ifdef EXTENSION_SYSTEM_INCLUDE_TESTCASES

	#include <drx3D/CoreX/Extension/ClassWeaver.h>
	#include <drx3D/CoreX/Extension/IDrxFactoryRegistryImpl.h>
	#include <drx3D/CoreX/Extension/DrxCreateClassInstance.h>

//////////////////////////////////////////////////////////////////////////

namespace TestComposition
{
struct ITestExt1 : public IDrxUnknown
{
	DRXINTERFACE_DECLARE_GUID(ITestExt1, "9d9e0dcf-a576-4cb0-a737-01595f75bd32"_drx_guid);

	virtual void Call1() const = 0;
};

DECLARE_SHARED_POINTERS(ITestExt1);

class CTestExt1 : public ITestExt1
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(ITestExt1)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CTestExt1, "TestExt1", "43b04e7c-c1be-45ca-9df6-ccb1c0dc1ad8"_drx_guid)

public:
	virtual void Call1() const;

private:
	i32 i;
};

DRXREGISTER_CLASS(CTestExt1)

CTestExt1::CTestExt1()
{
	i = 1;
}

CTestExt1::~CTestExt1()
{
	printf("Inside CTestExt1 dtor\n");
}

void CTestExt1::Call1() const
{
	printf("Inside CTestExt1::Call1()\n");
}

//////////////////////////////////////////////////////////////////////////

struct ITestExt2 : public IDrxUnknown
{
	DRXINTERFACE_DECLARE_GUID(ITestExt2, "8eb7a4b3-9987-4b9c-b96b-d6da7a8c72f9"_drx_guid);

	virtual void Call2() = 0;
};

DECLARE_SHARED_POINTERS(ITestExt2);

class CTestExt2 : public ITestExt2
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(ITestExt2)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CTestExt2, "TestExt2", "25b3ebf8-f175-4b9a-b549-4e3da7cdd80f"_drx_guid);

public:
	virtual void Call2();

private:
	i32 i;
};

DRXREGISTER_CLASS(CTestExt2)

CTestExt2::CTestExt2()
{
	i = 2;
}

CTestExt2::~CTestExt2()
{
	printf("Inside CTestExt2 dtor\n");
}

void CTestExt2::Call2()
{
	printf("Inside CTestExt2::Call2()\n");
}

//////////////////////////////////////////////////////////////////////////

class CComposed : public IDrxUnknown
{
	DRXGENERATE_CLASS_GUID(CComposed, "Composed", "0439d74b-8dcd-4b7f-9287-dcdf7e26a3a5"_drx_guid)

	DRXCOMPOSITE_BEGIN()
	DRXCOMPOSITE_ADD(m_pTestExt1, "Ext1")
	DRXCOMPOSITE_ADD(m_pTestExt2, "Ext2")
	DRXCOMPOSITE_END(CComposed)

	DRXINTERFACE_BEGIN()
	DRXINTERFACE_END()

private:
	ITestExt1Ptr m_pTestExt1;
	ITestExt2Ptr m_pTestExt2;
};

DRXREGISTER_CLASS(CComposed)

CComposed::CComposed()
	: m_pTestExt1()
	, m_pTestExt2()
{
	DrxCreateClassInstance("TestExt1", m_pTestExt1);
	DrxCreateClassInstance("TestExt2", m_pTestExt2);
}

CComposed::~CComposed()
{
}

//////////////////////////////////////////////////////////////////////////

struct ITestExt3 : public IDrxUnknown
{
	DRXINTERFACE_DECLARE_GUID(ITestExt3, "dd017935-a213-4898-bd2f-ffa145551876"_drx_guid);
	virtual void Call3() = 0;
};

DECLARE_SHARED_POINTERS(ITestExt3);

class CTestExt3 : public ITestExt3
{
	DRXGENERATE_CLASS_GUID(CTestExt3, "TestExt3", "eceab40b-c4bb-4988-a9f6-3c1db85a69b1"_drx_guid);

	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(ITestExt3)
	DRXINTERFACE_END()

public:
	virtual void Call3();

private:
	i32 i;
};

DRXREGISTER_CLASS(CTestExt3)

CTestExt3::CTestExt3()
{
	i = 3;
}

CTestExt3::~CTestExt3()
{
	printf("Inside CTestExt3 dtor\n");
}

void CTestExt3::Call3()
{
	printf("Inside CTestExt3::Call3()\n");
}

//////////////////////////////////////////////////////////////////////////

class CComposed2 : public IDrxUnknown
{
	DRXGENERATE_CLASS_GUID(CComposed2, "Composed2", "0439d74b-8dcd-4b7e-9287-dcdf7e26a3a6"_drx_guid)

	DRXCOMPOSITE_BEGIN()
	DRXCOMPOSITE_ADD(m_pTestExt3, "Ext3")
	DRXCOMPOSITE_END(CComposed2)

	DRXINTERFACE_BEGIN()
	DRXINTERFACE_END()

private:
	ITestExt3Ptr m_pTestExt3;
};

DRXREGISTER_CLASS(CComposed2)

CComposed2::CComposed2()
	: m_pTestExt3()
{
	DrxCreateClassInstance("TestExt3", m_pTestExt3);
}

CComposed2::~CComposed2()
{
}

//////////////////////////////////////////////////////////////////////////

class CTestExt4 : public ITestExt1, public ITestExt2, public ITestExt3
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(ITestExt1)
	DRXINTERFACE_ADD(ITestExt2)
	DRXINTERFACE_ADD(ITestExt3)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CTestExt4, "TestExt4", "43204e7c-c1be-45ca-9df4-ccb1c0dc1ad8"_drx_guid)

public:
	virtual void Call1() const;
	virtual void Call2();
	virtual void Call3();

private:
	i32 i;
};

DRXREGISTER_CLASS(CTestExt4)

CTestExt4::CTestExt4()
{
	i = 4;
}

CTestExt4::~CTestExt4()
{
	printf("Inside CTestExt4 dtor\n");
}

void CTestExt4::Call1() const
{
	printf("Inside CTestExt4::Call1()\n");
}

void CTestExt4::Call2()
{
	printf("Inside CTestExt4::Call2()\n");
}

void CTestExt4::Call3()
{
	printf("Inside CTestExt4::Call3()\n");
}

//////////////////////////////////////////////////////////////////////////

class CMegaComposed : public CComposed, public CComposed2
{
	DRXGENERATE_CLASS_GUID(CMegaComposed, "MegaComposed", "05127875-59f8-4503-421a-c1af66f2fb6f"_drx_guid)

	DRXCOMPOSITE_BEGIN()
	DRXCOMPOSITE_ADD(m_pTestExt4, "Ext4")
	DRXCOMPOSITE_ENDWITHBASE2(CMegaComposed, CComposed, CComposed2)

	DRXINTERFACE_BEGIN()
	DRXINTERFACE_END()

private:
	std::shared_ptr<CTestExt4> m_pTestExt4;
};

DRXREGISTER_CLASS(CMegaComposed)

CMegaComposed::CMegaComposed()
	: m_pTestExt4()
{
	printf("Inside CMegaComposed ctor\n");
	m_pTestExt4 = CTestExt4::CreateClassInstance();
}

CMegaComposed::~CMegaComposed()
{
	printf("Inside CMegaComposed dtor\n");
}

//////////////////////////////////////////////////////////////////////////

static void TestComposition()
{
	printf("\nTest composition:\n");

	IDrxUnknownPtr p;
	if (DrxCreateClassInstance("MegaComposed", p))
	{
		ITestExt1Ptr p1 = drxinterface_cast<ITestExt1>(drxcomposite_query(p, "Ext1"));
		if (p1)
			p1->Call1();     // calls CTestExt1::Call1()
		ITestExt2Ptr p2 = drxinterface_cast<ITestExt2>(drxcomposite_query(p, "Ext2"));
		if (p2)
			p2->Call2();     // calls CTestExt2::Call2()
		ITestExt3Ptr p3 = drxinterface_cast<ITestExt3>(drxcomposite_query(p, "Ext3"));
		if (p3)
			p3->Call3();     // calls CTestExt3::Call3()
		p3 = drxinterface_cast<ITestExt3>(drxcomposite_query(p, "Ext4"));
		if (p3)
			p3->Call3();     // calls CTestExt4::Call3()

		p1 = drxinterface_cast<ITestExt1>(drxcomposite_query(p.get(), "Ext4"));
		p2 = drxinterface_cast<ITestExt2>(drxcomposite_query(p.get(), "Ext4"));

		bool b = DrxIsSameClassInstance(p1, p2);     // true
	}

	{
		IDrxUnknownConstPtr pCUnk = p;
		IDrxUnknownConstPtr pComp1 = drxcomposite_query(pCUnk.get(), "Ext1");
		//IDrxUnknownPtr pComp1 = drxcomposite_query(pCUnk, "Ext1"); // must fail to compile due to const rules

		ITestExt1ConstPtr p1 = drxinterface_cast<const ITestExt1>(pComp1);
		if (p1)
			p1->Call1();
	}
}

} // namespace TestComposition

//////////////////////////////////////////////////////////////////////////

namespace TestExtension
{

class CFoobar : public IFoobar
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IFoobar)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CFoobar, "Foobar", "76c8dd6d-1663-4531-95d3-b1cfabcf7ef4"_drx_guid)

public:
	virtual void Foo();
};

DRXREGISTER_CLASS(CFoobar)

CFoobar::CFoobar()
{
}

CFoobar::~CFoobar()
{
}

void CFoobar::Foo()
{
	printf("Inside CFoobar::Foo()\n");
}

static void TestFoobar()
{
	std::shared_ptr<CFoobar> p = CFoobar::CreateClassInstance();
	{
		DrxInterfaceID iid = drxiidof<IFoobar>();
		DrxClassID clsid = p->GetFactory()->GetClassID();
		i32 t = 0;
	}

	{
		IAPtr sp_ = drxinterface_cast<IA>(p);     // sp_ == NULL

		IDrxUnknownPtr sp1 = drxinterface_cast<IDrxUnknown>(p);
		IFoobarPtr sp = drxinterface_cast<IFoobar>(sp1);
		sp->Foo();
	}

	{
		CFoobar* pF = p.get();
		pF->Foo();
		IDrxUnknown* p1 = drxinterface_cast<IDrxUnknown>(pF);
	}

	IFoobar* pFoo = drxinterface_cast<IFoobar>(p.get());
	IDrxFactory* pF1 = pFoo->GetFactory();
	pFoo->Foo();

	i32 t = 0;
}

//////////////////////////////////////////////////////////////////////////

class CRaboof : public IRaboof
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IRaboof)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CRaboof, "Raabof", "ba482ce1-2b2e-4309-8238-ed8b52cb1f1e"_drx_guid)

public:
	virtual void Rab();
};

DRXREGISTER_CLASS(CRaboof)

CRaboof::CRaboof()
{
}

CRaboof::~CRaboof()
{
}

void CRaboof::Rab()
{
	printf("Inside CRaboof::Rab()\n");
}

static void TestRaboof()
{
	std::shared_ptr<CRaboof> pFoo0_ = CRaboof::CreateClassInstance();
	IRaboofPtr pFoo0 = drxinterface_cast<IRaboof>(pFoo0_);
	IDrxUnknownPtr p0 = drxinterface_cast<IDrxUnknown>(pFoo0);

	DrxInterfaceID iid = drxiidof<IRaboof>();
	DrxClassID clsid = p0->GetFactory()->GetClassID();

	std::shared_ptr<CRaboof> pFoo1 = CRaboof::CreateClassInstance();

	pFoo0->Rab();
	pFoo1->Rab();
}

//////////////////////////////////////////////////////////////////////////

class CAB : public IA, public IB
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IA)
	DRXINTERFACE_ADD(IB)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CAB, "AB", "b9e54711-a644-48c0-a481-9b4ed3024d04"_drx_guid)

public:
	virtual void A();
	virtual void B();

private:
	i32 i;
};

DRXREGISTER_CLASS(CAB)

CAB::CAB()
{
	i = 0x12345678;
}

CAB::~CAB()
{
}

void CAB::A()
{
	printf("Inside CAB::A()\n");
}

void CAB::B()
{
	printf("Inside CAB::B()\n");
}

//////////////////////////////////////////////////////////////////////////

class CABC : public CAB, public IC
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IC)
	DRXINTERFACE_ENDWITHBASE(CAB)

	DRXGENERATE_CLASS_GUID(CABC, "ABC", "4e61feae-1185-4be7-a161-57c5f8baadd9"_drx_guid)

public:
	virtual void C();

private:
	i32 a;
};

DRXREGISTER_CLASS(CABC)

CABC::CABC()
//: CAB()
{
	a = 0x87654321;
}

CABC::~CABC()
{
}

void CABC::C()
{
	printf("Inside CABC::C()\n");
}

//////////////////////////////////////////////////////////////////////////

class CCustomC : public ICustomC
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IC)
	DRXINTERFACE_ADD(ICustomC)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CCustomC, "CustomC", "ee61760b-98a4-4b71-a05e-7372b44bd0fd"_drx_guid)

public:
	virtual void C();
	virtual void C1();

private:
	i32 a;
};

DRXREGISTER_CLASS(CCustomC)

CCustomC::CCustomC()
{
	a = 0x87654321;
}

CCustomC::~CCustomC()
{
}

void CCustomC::C()
{
	printf("Inside CCustomC::C()\n");
}

void CCustomC::C1()
{
	printf("Inside CCustomC::C1()\n");
}

//////////////////////////////////////////////////////////////////////////

class CMultiBase : public CAB, public CCustomC
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ENDWITHBASE2(CAB, CCustomC)

	DRXGENERATE_CLASS_GUID(CMultiBase, "MultiBase", "75966b8f-9864-4d42-8fbd-d489e94cc29e"_drx_guid)

public:
	virtual void A();
	virtual void C1();

	i32 i;
};

DRXREGISTER_CLASS(CMultiBase)

CMultiBase::CMultiBase()
{
	i = 0x87654321;
}

CMultiBase::~CMultiBase()
{
}

void CMultiBase::C1()
{
	printf("Inside CMultiBase::C1()\n");
}

void CMultiBase::A()
{
	printf("Inside CMultiBase::A()\n");
}

//////////////////////////////////////////////////////////////////////////

static void TestComplex()
{
	{
		ICPtr p;
		if (DrxCreateClassInstance("75966b8f-9864-4d42-8fbd-d489e94cc29e"_drx_guid, p))
		{
			p->C();
		}
	}

	{
		ICustomCPtr p;
		if (DrxCreateClassInstance("MultiBase", p))
		{
			p->C();
		}
	}

	{
		IFoobarPtr p;
		if (DrxCreateClassInstance("75966b8f-9864-4d42-8fbd-d489e94cc29e"_drx_guid, p))
		{
			p->Foo();
		}
	}

	{
		std::shared_ptr<CMultiBase> p = CMultiBase::CreateClassInstance();
		std::shared_ptr<const CMultiBase> pc = p;

		{
			IDrxUnknownPtr pUnk = drxinterface_cast<IDrxUnknown>(p);
			IDrxUnknownConstPtr pCUnk0 = drxinterface_cast<const IDrxUnknown>(p);
			IDrxUnknownConstPtr pCUnk1 = drxinterface_cast<const IDrxUnknown>(pc);
			//IDrxUnknownPtr pUnkF = drxinterface_cast<IDrxUnknown>(pc); // must fail to compile due to const rules

			IDrxFactory* pF = pUnk->GetFactory();

			i32 t = 0;
		}

		ICPtr pC = drxinterface_cast<IC>(p);
		ICustomCPtr pCC = drxinterface_cast<ICustomC>(pC);

		p->C();
		p->C1();

		pC->C();
		pCC->C1();

		IAPtr pA = drxinterface_cast<IA>(p);
		pA->A();
		p->A();
	}

	{
		std::shared_ptr<CCustomC> p = CCustomC::CreateClassInstance();

		ICPtr pC = drxinterface_cast<IC>(p);
		ICustomCPtr pCC = drxinterface_cast<ICustomC>(pC);

		p->C();
		p->C1();

		pC->C();
		pCC->C1();
	}
	{
		DrxInterfaceID ia = drxiidof<IA>();
		DrxInterfaceID ib = drxiidof<IB>();
		DrxInterfaceID ic = drxiidof<IC>();
		DrxInterfaceID ico = drxiidof<IDrxUnknown>();
	}

	{
		std::shared_ptr<CAB> p = CAB::CreateClassInstance();
		DrxClassID clsid = p->GetFactory()->GetClassID();

		IAPtr pA = drxinterface_cast<IA>(p);
		IBPtr pB = drxinterface_cast<IB>(p);

		IBPtr pB1 = drxinterface_cast<IB>(pA);
		IAPtr pA1 = drxinterface_cast<IA>(pB);

		pA->A();
		pB->B();

		IDrxUnknownPtr p1 = drxinterface_cast<IDrxUnknown>(pA);
		IDrxUnknownPtr p2 = drxinterface_cast<IDrxUnknown>(pB);
		const IDrxUnknown* p3 = drxinterface_cast<const IDrxUnknown>(pB.get());

		i32 t = 0;
	}

	{
		std::shared_ptr<CABC> pABC = CABC::CreateClassInstance();
		DrxClassID clsid = pABC->GetFactory()->GetClassID();

		IDrxFactory* pFac = pABC->GetFactory();
		pFac->ClassSupports(drxiidof<IA>());
		pFac->ClassSupports(drxiidof<IRaboof>());

		IAPtr pABC0 = drxinterface_cast<IA>(pABC);
		IBPtr pABC1 = drxinterface_cast<IB>(pABC0);
		ICPtr pABC2 = drxinterface_cast<IC>(pABC1);

		pABC2->C();
		pABC1->B();

		pABC2->GetFactory();

		const IC* pCconst = pABC2.get();
		const IDrxUnknown* pOconst = drxinterface_cast<const IDrxUnknown>(pCconst);
		const IA* pAconst = drxinterface_cast<const IA>(pOconst);
		const IB* pBconst = drxinterface_cast<const IB>(pAconst);

		//const IA* pA11 = drxinterface_cast<IA>(pOconst);

		pCconst = drxinterface_cast<const IC>(pBconst);

		IC* pC = static_cast<IC*>(static_cast<uk>(pABC1.get()));
		pC->C();     // calls IB::B()

		i32 t = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
// use of extension system without any of the helper macros/templates

class CDontLikeMacrosFactory : public IDrxFactory
{
	// IDrxFactory
public:
	virtual tukk GetClassName() const
	{
		return "DontLikeMacros";
	}
	virtual const DrxClassID& GetClassID() const
	{
		static constexpr DrxGUID cid = "73c3ab00-42e6-488a-89ca-1a3763365565"_drx_guid;
		return cid;
	}
	virtual bool ClassSupports(const DrxInterfaceID& iid) const
	{
		return iid == drxiidof<IDrxUnknown>() || iid == drxiidof<IDontLikeMacros>();
	}
	virtual void ClassSupports(const DrxInterfaceID*& pIIDs, size_t& numIIDs) const
	{
		static const DrxInterfaceID iids[2] = { drxiidof<IDrxUnknown>(), drxiidof<IDontLikeMacros>() };
		pIIDs = iids;
		numIIDs = 2;
	}
	virtual IDrxUnknownPtr CreateClassInstance() const;

public:
	static CDontLikeMacrosFactory& Access()
	{
		return s_factory;
	}

private:
	CDontLikeMacrosFactory() {}
	~CDontLikeMacrosFactory() {}

private:
	static CDontLikeMacrosFactory s_factory;
};

CDontLikeMacrosFactory CDontLikeMacrosFactory::s_factory;

class CDontLikeMacros : public IDontLikeMacros
{
	// IDrxUnknown
public:
	virtual IDrxFactory* GetFactory() const
	{
		return &CDontLikeMacrosFactory::Access();
	};

protected:
	virtual uk QueryInterface(const DrxInterfaceID& iid) const
	{
		if (iid == drxiidof<IDrxUnknown>())
			return (uk )(IDrxUnknown*) this;
		else if (iid == drxiidof<IDontLikeMacros>())
			return (uk )(IDontLikeMacros*) this;
		else
			return 0;
	}

	virtual uk QueryComposite(tukk) const
	{
		return 0;
	}

	// IDontLikeMacros
public:
	virtual void CallMe()
	{
		printf("Yey, no macros...\n");
	}

	CDontLikeMacros() {}

protected:
	virtual ~CDontLikeMacros() {}
};

IDrxUnknownPtr CDontLikeMacrosFactory::CreateClassInstance() const
{
	std::shared_ptr<CDontLikeMacros> p(new CDontLikeMacros);
	return IDrxUnknownPtr(*static_cast<std::shared_ptr<IDrxUnknown>*>(static_cast<uk>(&p)));
}

static SRegFactoryNode g_dontLikeMacrosFactory(&CDontLikeMacrosFactory::Access());

//////////////////////////////////////////////////////////////////////////

static void TestDontLikeMacros()
{
	IDrxFactory* f = &CDontLikeMacrosFactory::Access();

	f->ClassSupports(drxiidof<IDrxUnknown>());
	f->ClassSupports(drxiidof<IDontLikeMacros>());

	const DrxInterfaceID* pIIDs = 0;
	size_t numIIDs = 0;
	f->ClassSupports(pIIDs, numIIDs);

	IDrxUnknownPtr p = f->CreateClassInstance();
	IDontLikeMacrosPtr pp = drxinterface_cast<IDontLikeMacros>(p);

	IDrxUnknownPtr pq = drxcomposite_query(p, "blah");

	pp->CallMe();
}

} // namespace TestExtension

//////////////////////////////////////////////////////////////////////////

void TestExtensions(IDrxFactoryRegistryImpl* pReg)
{
	printf("Test extensions:\n");

	struct MyCallback : public IDrxFactoryRegistryCallback
	{
		virtual void OnNotifyFactoryRegistered(IDrxFactory* pFactory)
		{
			i32 test = 0;
		}
		virtual void OnNotifyFactoryUnregistered(IDrxFactory* pFactory)
		{
			i32 test = 0;
		}
	};

	//pReg->RegisterCallback((IDrxFactoryRegistryCallback*) 0x4);
	//pReg->RegisterCallback((IDrxFactoryRegistryCallback*) 0x1);
	//pReg->RegisterCallback((IDrxFactoryRegistryCallback*) 0x3);
	//pReg->RegisterCallback((IDrxFactoryRegistryCallback*) 0x3);
	//pReg->RegisterCallback((IDrxFactoryRegistryCallback*) 0x2);

	//pReg->UnregisterCallback((IDrxFactoryRegistryCallback*) 0x2);
	//pReg->UnregisterCallback((IDrxFactoryRegistryCallback*) 0x2);
	//pReg->UnregisterCallback((IDrxFactoryRegistryCallback*) 0x4);
	//pReg->UnregisterCallback((IDrxFactoryRegistryCallback*) 0x3);
	//pReg->UnregisterCallback((IDrxFactoryRegistryCallback*) 0x1);

	//MyCallback callback0;
	//pReg->RegisterCallback(&callback0);
	//pReg->RegisterFactories(g_pHeadToRegFactories);

	//pReg->RegisterFactories(g_pHeadToRegFactories);
	//pReg->UnregisterFactories(g_pHeadToRegFactories);

	IDrxFactory* pF[4];
	size_t numFactories = 4;
	pReg->IterateFactories(drxiidof<IA>(), pF, numFactories);
	pReg->IterateFactories("ffffffff-ffff-ffff-ffff-ffffffffffff"_drx_guid, pF, numFactories);

	numFactories = (size_t)-1;
	pReg->IterateFactories(drxiidof<IDrxUnknown>(), 0, numFactories);

	MyCallback callback1;
	pReg->RegisterCallback(&callback1);
	pReg->UnregisterCallback(&callback1);

	pReg->GetFactory("ee61760b-98a4-4b71-a05e-7372b44bd0fd"_drx_guid);
	pReg->GetFactory("CustomC");
	pReg->GetFactory("ABC");
	pReg->GetFactory(nullptr);

	pReg->GetFactory("DontLikeMacros");
	pReg->GetFactory("73c3ab00-42e6-488a-89ca-1a3763365565"_drx_guid);

	TestExtension::TestFoobar();
	TestExtension::TestRaboof();
	TestExtension::TestComplex();
	TestExtension::TestDontLikeMacros();

	TestComposition::TestComposition();
}

#endif // #ifdef EXTENSION_SYSTEM_INCLUDE_TESTCASES
