// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FRAGMENT_EDITOR_NODES__H__
#define __FRAGMENT_EDITOR_NODES__H__

#include "SequencerNode.h"
#include "MannequinBase.h"

class CMannFragmentEditor;

class CFragmentNode
	: public CSequencerNode
{
public:
	CFragmentNode(CSequencerSequence* sequence, SScopeData* pScopeData, CMannFragmentEditor* fragmentEditor);

	virtual void               OnChange();
	virtual u32             GetChangeCount() const;

	virtual ESequencerNodeType GetType() const;

	virtual IEntity*           GetEntity();

	virtual void               InsertMenuOptions(CMenu& menu);
	virtual void               ClearMenuOptions(CMenu& menu);
	virtual void               OnMenuOption(i32 menuOption);

	virtual i32                GetParamCount() const;
	virtual bool               GetParamInfo(i32 nIndex, SParamInfo& info) const;

	virtual bool               CanAddTrackForParameter(ESequencerParamType paramId) const;

	virtual CSequencerTrack*   CreateTrack(ESequencerParamType nParamId);

	virtual void               UpdateMutedLayerMasks(u32 mutedAnimLayerMask, u32 mutedProcLayerMask);

	SScopeData*                GetScopeData();

	void                       SetIsSequenceNode(const bool locked);
	bool                       IsSequenceNode() const;

	void                       SetSelection(const bool hasFragment, const SFragmentSelection& fragSelection);
	bool                       HasFragment() const;
	const SFragmentSelection&  GetSelection() const;

protected:
	u32 GetNumTracksOfParamID(ESequencerParamType paramID) const;

	bool   CanAddParamType(ESequencerParamType paramID) const;

private:
	CMannFragmentEditor* m_fragmentEditor;
	SScopeData*          m_pScopeData;
	SFragmentSelection   m_fragSelection;
	bool                 m_isSequenceNode;
	bool                 m_hasFragment;
};

class CFERootNode
	: public CSequencerNode
{
public:
	CFERootNode(CSequencerSequence* sequence, const SControllerDef& controllerDef);
	~CFERootNode();
	virtual ESequencerNodeType GetType() const;
	virtual i32                GetParamCount() const;
	virtual bool               GetParamInfo(i32 nIndex, SParamInfo& info) const;
	virtual CSequencerTrack*   CreateTrack(ESequencerParamType nParamId);
};

#endif

