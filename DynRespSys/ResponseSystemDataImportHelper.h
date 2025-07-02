// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************

   This whole class is just here to allow creating of Drx DRS Elements to be created by hand from the outside.

************************************************************************/

#pragma once

#include <drx3D/DynRespSys/IDynamicResponseSystem.h>
#include "ResponseUpr.h"

namespace DrxDRS
{
class CDataImportHelper final : public DRS::IDataImportHelper
{
public:
	CDataImportHelper();

	//////////////////////////////////////////////////////////
	// IDataImportHelper implementation
	virtual ResponseID                    AddSignalResponse(const string& szName) override;
	virtual bool                          AddResponseCondition(ResponseID responseID, DRS::IConditionSharedPtr pCondition, bool bNegated) override;
	virtual bool                          AddResponseAction(ResponseID segmentID, DRS::IResponseActionSharedPtr pAction) override;
	virtual ResponseSegmentID             AddResponseSegment(ResponseID parentResponse, const string& szName) override;
	virtual bool                          AddResponseSegmentAction(ResponseID parentResponse, ResponseSegmentID segmentID, DRS::IResponseActionSharedPtr pAction) override;
	virtual bool                          AddResponseSegmentCondition(ResponseID parentResponse, ResponseSegmentID segmentID, DRS::IConditionSharedPtr pConditions, bool bNegated) override;

	virtual bool                          HasActionCreatorForType(const CHashedString& type) override;
	virtual bool                          HasConditionCreatorForType(const CHashedString& type) override;
	virtual DRS::IResponseActionSharedPtr CreateActionFromString(const CHashedString& type, const string& data, tukk szFormatName) override;
	virtual DRS::IConditionSharedPtr      CreateConditionFromString(const CHashedString& type, const string& data, tukk szFormatName) override;
	virtual void                          RegisterConditionCreator(const CHashedString& conditionType, CondtionCreatorFct pFunc) override;
	virtual void                          RegisterActionCreator(const CHashedString& actionTyp, ActionCreatorFct pFunc) override;

	virtual void                          Reset() override;
	virtual void                          Serialize(Serialization::IArchive& ar) override;
	//////////////////////////////////////////////////////////

	static bool splitStringList(const string& stringToSplitUp, const char delimeter1, const char delimeter2, std::vector<string>* outResult, bool bTrim = true);

private:

	std::unordered_map<CHashedString, CondtionCreatorFct> m_conditionCreators;
	std::unordered_map<CHashedString, ActionCreatorFct>   m_actionsCreators;

	CResponseUpr::MappedSignals                       m_mappedSignals;
};
}
