// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Sys/DrxUnitTest.h>
#include <drx3D/Game/GameTypeInfo.h>

#include <cstring>


bool CGameTypeInfo::IsCastable(const CGameTypeInfo* type) const
{
	for (const CGameTypeInfo* ptr = this; ptr; ptr = ptr->GetParent())
	{
		if (ptr == type)
			return true;
	}
	return false;
}




class CBase
{
public:
	virtual ~CBase() {}
	DRX_DECLARE_GTI_BASE(CBase);
};
DRX_IMPLEMENT_GTI_BASE(CBase);



class CType1 : public CBase
{
public:
	DRX_DECLARE_GTI(CType1);
};
DRX_IMPLEMENT_GTI(CType1, CBase);



class CType2 : public CBase
{
public:
	DRX_DECLARE_GTI(CType2);
};
DRX_IMPLEMENT_GTI(CType2, CBase);



DRX_UNIT_TEST_SUITE(DrxGameTypeInfoTest)
{
	DRX_UNIT_TEST(GoodDownCast)
	{
		CBase* obj = new CType2();
		CType2* casted = crygti_cast<CType2*>(obj);
		DRX_UNIT_TEST_ASSERT(casted==obj);
		delete obj;
	}

	DRX_UNIT_TEST(BadDownCast)
	{
		CBase* obj = new CType2();
		CType1* casted = crygti_cast<CType1*>(obj);
		DRX_UNIT_TEST_ASSERT(casted == 0);
		delete obj;
	}

	DRX_UNIT_TEST(UpCast)
	{
		CType2* obj = new CType2();
		CBase* casted = crygti_cast<CBase*>(obj);
		DRX_UNIT_TEST_ASSERT(casted == obj);
		delete obj;
	}

	DRX_UNIT_TEST(ConstGoodDownCast)
	{
		const CBase* obj = new CType2();
		const CType2* casted = crygti_cast<const CType2*>(obj);
		DRX_UNIT_TEST_ASSERT(casted==obj);
		delete obj;
	}

	DRX_UNIT_TEST(ConstBadDownCast)
	{
		const CBase* obj = new CType2();
		const CType1* casted = crygti_cast<const CType1*>(obj);
		DRX_UNIT_TEST_ASSERT(casted == 0);
		delete obj;
	}

	DRX_UNIT_TEST(ConstUpCast)
	{
		const CType2* obj = new CType2();
		const CBase* casted = crygti_cast<const CBase*>(obj);
		DRX_UNIT_TEST_ASSERT(casted == obj);
		delete obj;
	}

	DRX_UNIT_TEST(SameCast)
	{
		CType2* obj = new CType2();
		CType2* casted = crygti_cast<CType2*>(obj);
		DRX_UNIT_TEST_ASSERT(casted == obj);
		delete obj;
	}

	DRX_UNIT_TEST(NullCast)
	{
		CType2* obj = 0;
		CType2* casted = crygti_cast<CType2*>(obj);
		DRX_UNIT_TEST_ASSERT(casted == 0);
	}

	DRX_UNIT_TEST(BaseTypeNotCastableToSubType_BugFix)
	{
		CBase* obj = new CBase();
		CType1* casted = crygti_cast<CType1*>(obj);
		DRX_UNIT_TEST_ASSERT(casted == 0);
		delete obj;
	}

	DRX_UNIT_TEST(TrueDownCastIsOf)
	{
		CBase* obj = new CType2();
		DRX_UNIT_TEST_ASSERT(crygti_isof<CType2>(obj) == true);
		delete obj;
	}

	DRX_UNIT_TEST(FalseDownCastIsOf)
	{
		CBase* obj = new CType2();
		DRX_UNIT_TEST_ASSERT(crygti_isof<CType1>(obj) == false);
		delete obj;
	}

	DRX_UNIT_TEST(TrueUpCastIsOf)
	{
		CBase* obj = new CType2();
		DRX_UNIT_TEST_ASSERT(crygti_isof<CBase>(obj) == true);
		delete obj;
	}

	DRX_UNIT_TEST(NullIsOf)
	{
		CBase* obj = 0;
		DRX_UNIT_TEST_ASSERT(crygti_isof<CBase>(obj) == false);
		delete obj;
	}

	DRX_UNIT_TEST(GetName)
	{
		CBase* obj = new CType2();
		DRX_UNIT_TEST_ASSERT(std::strcmp(obj->GetRunTimeType()->GetName(), "CType2") == 0);
		delete obj;
	}

}
