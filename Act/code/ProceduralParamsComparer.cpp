// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/Act/IDrxMannequin.h>
#include <drx3D/Act/Serialization.h>
#include <drx3D/Sys/ArchiveHost.h>

class CProceduralParamsComparerDefault
	: public IProceduralParamsComparer
{
public:
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IProceduralParamsComparer)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CProceduralParamsComparerDefault, "ProceduralParamsComparerDefault", "fc53bd92-4853-4faa-ab0f-b42b24e55b3e"_drx_guid)

	virtual ~CProceduralParamsComparerDefault() {}

	virtual bool Equal(const IProceduralParams& lhs, const IProceduralParams& rhs) const override
	{
		return Serialization::CompareBinary(lhs, rhs);
	}
};

DRXREGISTER_CLASS(CProceduralParamsComparerDefault);
