// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FLOWITEMANIMATIOn_H__
#define __FLOWITEMANIMATIOn_H__

#include <drx3D/Game/Game.h>
#include <drx3D/Game/Item.h>
#include <drx3D/Game/Actor.h>
#include <drx3D/Game/Nodes/G2FlowBaseNode.h>
#include <drx3D/Game/Nodes/G2FlowBaseNode.h>

class CFlowItemAction : public CFlowBaseNode<eNCT_Instanced>
{
private:

public:

	CFlowItemAction( SActivationInfo * pActInfo );

	IFlowNodePtr Clone( SActivationInfo * pActInfo );
	virtual void GetConfiguration(SFlowNodeConfig& config);
	virtual void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo );
	virtual void GetMemoryUsage(IDrxSizer * s) const;

private:
	void StartDoneCountDown(SActivationInfo* pActInfo, float time);
	IItem* GetItem(SActivationInfo* pActInfo) const;
	void Activate(SActivationInfo* pActInfo);
	void Update(SActivationInfo* pActInfo);

	float m_timerCountDown;
};

#endif //__FLOWITEMANIMATIOn_H__
