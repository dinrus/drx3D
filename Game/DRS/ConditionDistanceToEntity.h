// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/DynRespSys/IDynamicResponseCondition.h>
#include <drx3D/DynRespSys/IDynamicResponseSystem.h>

namespace DRS
{
	struct IResponseActor;
	struct IVariableCollection;
}

class CConditionDistanceToEntity final : public DRS::IResponseCondition
{
public:
	CConditionDistanceToEntity();
	CConditionDistanceToEntity(const string& actorName);
	virtual ~CConditionDistanceToEntity();

	//////////////////////////////////////////////////////////
	// IResponseCondition implementation
	virtual bool IsMet(DRS::IResponseInstance* pResponseInstance) override;
	virtual void Serialize(Serialization::IArchive& ar) override;
	virtual string GetVerboseInfo() const override;
	virtual tukk GetType() const override { return "Distance to Entity"; }
	//////////////////////////////////////////////////////////

private:
	float m_squaredDistance;
	string m_entityName;
};
