// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <DrxMovie/IMovieSystem.h>
#include "HyperGraph/FlowGraphNode.h"

#define TRACKEVENT_CLASS     ("TrackEvent")
#define TRACKEVENTNODE_TITLE ("TrackEvent")
#define TRACKEVENTNODE_CLASS ("TrackEvent")
#define TRACKEVENTNODE_DESC  ("Outputs for Trackview Events")

class CTrackEventNode : public CFlowNode, public ITrackEventListener
{
public:
	static tukk GetClassType()
	{
		return TRACKEVENT_CLASS;
	}
	CTrackEventNode();
	virtual ~CTrackEventNode();

	// CHyperNode
	virtual void           Init() override;
	virtual void           Done() override;
	virtual CHyperNode*    Clone() override;
	virtual void           Serialize(XmlNodeRef& node, bool bLoading, CObjectArchive* ar) override;

	virtual CString        GetTitle() const override;
	virtual tukk    GetClassName() const override;
	virtual tukk    GetDescription() const override;
	virtual Gdiplus::Color GetCategoryColor() const override { return Gdiplus::Color(220, 40, 40); }
	virtual void           OnInputsChanged();
	// ~CHyperNode

	// Description:
	//		Re-populates output ports based on input sequence
	// Arguments:
	//		bLoading - TRUE if called from serialization on loading
	virtual void PopulateOutput(bool bLoading);

	// ~ITrackEventListener
	virtual void OnTrackEvent(IAnimSequence* pSequence, i32 reason, tukk event, uk pUserData);

protected:
	//! Add an output port for an event
	virtual void AddOutputEvent(tukk event);

	//! Remove an output port for an event (including any edges)
	virtual void RemoveOutputEvent(tukk event);

	//! Rename an output port for an event
	virtual void RenameOutputEvent(tukk event, tukk newName);

	//! Move up a specified output port once in the list of output ports
	virtual void MoveUpOutputEvent(tukk event);

	//! Move down a specified output port once in the list of output ports
	virtual void MoveDownOutputEvent(tukk event);

private:
	IAnimSequence* m_pSequence; // Current sequence
};

