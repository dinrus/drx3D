// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/TimeValue.h>

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/Delegate.h>
#include <drx3D/Schema/GUID.h>

namespace sxema
{
struct IScriptElement;

typedef std::function<EVisitStatus (IScriptElement&)> ScriptElementVisitor;

struct IScript
{
	virtual ~IScript() {}

	virtual DrxGUID             GetGUID() const = 0;

	virtual tukk       SetFilePath(tukk szFilePath) = 0;
	virtual tukk       GetFilePath() const = 0;

	virtual const CTimeValue& GetTimeStamp() const = 0;

	virtual void              SetRoot(IScriptElement* pRoot) = 0; // #SchematycTODO : Does this really need to be part of the public interface?
	virtual IScriptElement*   GetRoot() = 0;

	virtual EVisitStatus      VisitElements(const ScriptElementVisitor& visitor) = 0;

};
} // sxema
