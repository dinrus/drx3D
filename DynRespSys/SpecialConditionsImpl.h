// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************

   Some additional base conditions

   /************************************************************************/

#pragma once

#include <drx3D/DynRespSys/IDynamicResponseCondition.h>
#include <drx3D/CoreX/Game/IGameTokens.h>
#include <drx3D/CoreX/String/DrxString.h>
#include "VariableValue.h"
#include "ConditionsCollection.h"
#include "ResponseUpr.h"
#include "Variable.h"

struct IGameToken;

namespace DrxDRS
{
class CPlaceholderCondition final : public DRS::IResponseCondition
{
public:
	CPlaceholderCondition() { m_Label = "Empty"; }

	//////////////////////////////////////////////////////////
	// IResponseCondition implementation
	virtual bool        IsMet(DRS::IResponseInstance* pResponseInstance) override { return true; }
	virtual void        Serialize(Serialization::IArchive& ar) override;
	virtual string      GetVerboseInfo() const override                           { return "\"" + m_Label + "\""; }
	virtual tukk GetType() const override                                  { return "Placeholder"; }
	//////////////////////////////////////////////////////////

private:
	string m_Label;
};

//////////////////////////////////////////////////////////////////////////
class CRandomCondition final : public DRS::IResponseCondition
{
public:
	CRandomCondition() : m_randomFactor(50) {}
	CRandomCondition(i32 factor) : m_randomFactor(factor) {}

	//////////////////////////////////////////////////////////
	// IResponseCondition implementation
	virtual bool        IsMet(DRS::IResponseInstance* pResponseInstance) override;
	virtual void        Serialize(Serialization::IArchive& ar) override;
	virtual string      GetVerboseInfo() const override { return string("Chance: ") + DrxStringUtils::toString(m_randomFactor); }
	virtual tukk GetType() const override        { return "Random"; }
	//////////////////////////////////////////////////////////

private:
	i32 m_randomFactor;   //0 - 100
};

//////////////////////////////////////////////////////////////////////////
class CExecutionLimitCondition final : public DRS::IResponseCondition
{
public:
	CExecutionLimitCondition() : m_minExecutions(1), m_maxExecutions(1) {}
	CExecutionLimitCondition(i32 minExecutions, i32 maxExecutions) : m_minExecutions(minExecutions), m_maxExecutions(maxExecutions) {}

	//////////////////////////////////////////////////////////
	// IResponseCondition implementation
	virtual bool        IsMet(DRS::IResponseInstance* pResponseInstance) override;
	virtual void        Serialize(Serialization::IArchive& ar) override;
	virtual string      GetVerboseInfo() const override;
	virtual tukk GetType() const override { return "Execution Limit"; }
	//////////////////////////////////////////////////////////

private:
	CHashedString m_ResponseToTest;
	i32           m_minExecutions;
	i32           m_maxExecutions;
};

//////////////////////////////////////////////////////////////////////////
class CInheritConditionsCondition final : public DRS::IResponseCondition
{
public:
	CInheritConditionsCondition() {}
	CInheritConditionsCondition(const CHashedString& signalToReuse) : m_responseToReuse(signalToReuse) {}

	//////////////////////////////////////////////////////////
	// IResponseCondition implementation
	virtual bool        IsMet(DRS::IResponseInstance* pResponseInstance) override;
	virtual void        Serialize(Serialization::IArchive& ar) override;
	virtual string      GetVerboseInfo() const override { return string("From ") + m_responseToReuse.GetText(); }
	virtual tukk GetType() const override        { return "Inherit Conditions"; }
	//////////////////////////////////////////////////////////

private:
	CHashedString m_responseToReuse;
	ResponsePtr   m_pCachedResponse;
};

//////////////////////////////////////////////////////////////////////////
// Checks if the time (stored in variable) is in the specified range
class CTimeSinceCondition final : public IVariableUsingBase, public DRS::IResponseCondition
{
public:
	CTimeSinceCondition() { m_minTime = 5.0f; m_maxTime = -1.0f; }
	//////////////////////////////////////////////////////////
	// IResponseCondition implementation
	virtual bool        IsMet(DRS::IResponseInstance* pResponseInstance) override;
	virtual void        Serialize(Serialization::IArchive& ar) override;
	virtual string      GetVerboseInfo() const override;
	virtual tukk GetType() const override { return "TimeSince"; }
	//////////////////////////////////////////////////////////

protected:
	float m_minTime;
	float m_maxTime;
};

//////////////////////////////////////////////////////////////////////////
// Checks if the time since execution of the specified response is in the specified range
class CTimeSinceResponseCondition final : public DRS::IResponseCondition
{
public:
	CTimeSinceResponseCondition() { m_minTime = 5.0f; m_maxTime = -1.0f; }

	//////////////////////////////////////////////////////////
	// IResponseCondition implementation
	virtual bool        IsMet(DRS::IResponseInstance* pResponseInstance) override;
	virtual void        Serialize(Serialization::IArchive& ar) override;
	virtual string      GetVerboseInfo() const override;
	virtual tukk GetType() const override { return "TimeSinceResponse"; }
	//////////////////////////////////////////////////////////

protected:
	CHashedString m_responseId;
	float         m_minTime;
	float         m_maxTime;
};

//////////////////////////////////////////////////////////////////////////
class CGameTokenCondition final : public DRS::IResponseCondition, public IGameTokenEventListener
{
public:
	CGameTokenCondition() : m_pCachedToken(nullptr), m_minValue(CVariableValue::NEG_INFINITE), m_maxValue(CVariableValue::POS_INFINITE) {}
	CGameTokenCondition(const string& tokenName, const CVariableValue& minValue, const CVariableValue& maxValue)
		: m_tokenName(tokenName), m_pCachedToken(nullptr), m_minValue(minValue), m_maxValue(maxValue) {}
	virtual ~CGameTokenCondition() override;

	//////////////////////////////////////////////////////////
	// IResponseCondition implementation
	virtual bool        IsMet(DRS::IResponseInstance* pResponseInstance) override;
	virtual void        Serialize(Serialization::IArchive& ar) override;
	virtual string      GetVerboseInfo() const override;
	virtual tukk GetType() const override { return "GameTokenCheck"; }
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// IGameTokenEventListener implementation
	virtual void OnGameTokenEvent(EGameTokenEvent event, IGameToken* pGameToken) override;
	virtual void GetMemoryUsage(class IDrxSizer* pSizer) const override;
	//////////////////////////////////////////////////////////

private:
	IGameToken*    m_pCachedToken;
	string         m_tokenName;

	CVariableValue m_minValue;
	CVariableValue m_maxValue;
};

}  //end namespace DrxDRS
