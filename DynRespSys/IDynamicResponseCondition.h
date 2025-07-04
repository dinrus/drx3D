// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************

   An Interface for Conditions that can be added to Response Segments

************************************************************************/

#ifndef _DYNAMICRESPONSECONDITION_H_
#define _DYNAMICRESPONSECONDITION_H_

#include <drx3D/Sys/IXml.h>
#include <drx3D/CoreX/Serialization/Forward.h>
#include "IDynamicResponseSystem.h"

namespace DRS
{
struct IResponseActor;
struct IVariableCollection;

struct IResponseCondition : public IEditorObject
{
	virtual ~IResponseCondition() = default;

	typedef u32 ConditionIdentifierHash;
	/**
	 * the condition is required to check all needed data as fast as possible and return if the response can therefore be executed
	 *
	 * Note: its called by the DRS system, to evaluate if this condition is fulfilled
	 * @param pResponseInstance - a pointer to the currently running response Instance, this contains the signal name, the contextVariables, the current segment and the active actor
	 */
	virtual bool IsMet(IResponseInstance* pResponseInstance) = 0;

	/**
	 * Serializes all the member variables of this class
	 *
	 * Note: Called by the serialization library to serialize to disk or to the UI
	 */
	virtual void Serialize(Serialization::IArchive& ar) = 0;
};
}  //namespace DRS

#endif  //_DYNAMICRESPONSECONDITION_H_
