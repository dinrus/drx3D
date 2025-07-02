// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScript.h>
#include <drx3D/Schema/StackString.h>

namespace sxema
{
class CScript : public IScript
{
	typedef std::vector<IScriptElement*> Elements;

public:
	CScript(const DrxGUID& guid, tukk szFilePath);
	CScript(tukk szFilePath);

	// IScript
	virtual DrxGUID           GetGUID() const override;

	virtual tukk       SetFilePath(tukk szFilePath) override;
	virtual tukk       GetFilePath() const override { return m_filePath.c_str(); }

	virtual const CTimeValue& GetTimeStamp() const override;

	virtual void              SetRoot(IScriptElement* pRoot) override;
	virtual IScriptElement*   GetRoot() override;

	virtual EVisitStatus      VisitElements(const ScriptElementVisitor& visitor) override;
	// ~IScript

private:
	EVisitStatus VisitElementsRecursive(const ScriptElementVisitor& visitor, IScriptElement& element);
	void         SetNameFromRootRecursive(CStackString& name, IScriptElement& element);

	DrxGUID         m_guid;
	string          m_filePath;
	CTimeValue      m_timeStamp;
	IScriptElement* m_pRoot;
};

DECLARE_SHARED_POINTERS(CScript)
}
