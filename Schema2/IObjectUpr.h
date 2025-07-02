// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Think about how we can filter signals for broadcast rather then using the brute force approach.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/TemplateUtils_Delegate.h>

#include <drx3D/Schema2/ILib.h>

namespace sxema2
{
	struct IObject;
	struct SObjectParams;

	typedef TemplateUtils::CDelegate<EVisitStatus (const IObject&)> IObjectVisitor;
	typedef TemplateUtils::CDelegate<EVisitStatus (const IObject&)> IObjectConstVisitor;

	struct IObjectUpr
	{
		virtual ~IObjectUpr() {}

		virtual IObject* CreateObject(const SObjectParams& params) = 0;
		virtual void DestroyObject(IObject* pObject) = 0;
		virtual IObject* GetObjectById(const ObjectId& objectId) = 0;
		//virtual IObject* GetObjectByEntityId(const ExplicitEntityId& entityId) = 0;
		virtual void VisitObjects(const IObjectVisitor& visitor) = 0;
		virtual void VisitObjects(const IObjectConstVisitor& visitor) const = 0;
		virtual void SendSignal(const ObjectId& objectId, const SGUID& signalGUID, const TVariantConstArray& inputs = TVariantConstArray()) = 0;
		virtual void SendSignal(const ExplicitEntityId& entityId, const SGUID& signalGUID, const TVariantConstArray& inputs = TVariantConstArray()) = 0;
		virtual void BroadcastSignal(const SGUID& signalGUID, const TVariantConstArray& inputs = TVariantConstArray()) = 0;
	};
}
