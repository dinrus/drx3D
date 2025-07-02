// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SEQUENCE_ANALAYZER_NODES__H__
#define __SEQUENCE_ANALAYZER_NODES__H__

#include "SequencerNode.h"
#include "MannequinBase.h"

class CRootNode
	: public CSequencerNode
{
public:
	CRootNode(CSequencerSequence* sequence, const SControllerDef& controllerDef);
	~CRootNode();

	virtual i32              GetParamCount() const;
	virtual bool             GetParamInfo(i32 nIndex, SParamInfo& info) const;

	virtual CSequencerTrack* CreateTrack(ESequencerParamType nParamId);
};

class CScopeNode
	: public CSequencerNode
{
public:
	CScopeNode(CSequencerSequence* sequence, SScopeData* pScopeData, EMannequinEditorMode mode);

	CMenu menuSetContext;

	virtual void             InsertMenuOptions(CMenu& menu);

	virtual void             ClearMenuOptions(CMenu& menu);

	virtual void             OnMenuOption(i32 menuOption);
	virtual IEntity*         GetEntity();
	virtual i32              GetParamCount() const;
	virtual bool             GetParamInfo(i32 nIndex, SParamInfo& info) const;

	virtual CSequencerTrack* CreateTrack(ESequencerParamType nParamId);

	virtual void             UpdateMutedLayerMasks(u32 mutedAnimLayerMask, u32 mutedProcLayerMask);

private:
	SScopeData*          m_pScopeData;
	EMannequinEditorMode m_mode;
};

#endif

