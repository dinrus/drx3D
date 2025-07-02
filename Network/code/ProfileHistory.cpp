// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/ProfileHistory.h>
#include  <drx3D/Network/NetContext.h>
#include  <drx3D/Network/ContextView.h>

CProfileHistory::CProfileHistory(CContextView* pView) : CHistory(eHSS_Profile, 1)
{
	if (pView->IsLocal() || pView->IsClient())
		indexMask = 0;
	else
		indexMask = pView->Context()->ServerManagedProfileAspects();
}

bool CProfileHistory::ReadCurrentValue(const SReceiveContext& ctx, bool commit)
{
	u8 profile;
	ctx.ser.Value("profile", profile);
	if (commit)
		ctx.pView->SetAspectProfile(ctx.objId, ctx.index, profile);
	return true;
}

void CProfileHistory::HandleEvent(const SHistoricalEvent& event)
{
	switch (event.event)
	{
	case eHE_UnbindAspect:
		{
			SSyncContext ctx;
			ctx.pViewObjEx = event.pViewObjEx;
			ctx.index = event.index;
			Flush(ctx);
		}
		break;
	}
}

bool CProfileHistory::CanSync(const SSyncContext& ctx, CSyncContext* pSyncContext, const CRegularlySyncedItem& item)
{
	return true;
}

bool CProfileHistory::NeedToSync(const SSyncContext& ctx, CSyncContext* pSyncContext, const CRegularlySyncedItem& item)
{
	switch (item.GetHistoryCount())
	{
	case eHC_Zero:
		return true;
	case eHC_One:
		if (SElem* pElem = pSyncContext->GetPrevElem(ctx))
			return ctx.ctxObj.main->vAspectProfiles[ctx.index] != pElem->data;
		else
			return true;
	case eHC_MoreThanOne:
		CRegularlySyncedItem::ConstValueIter iter = item.GetConstIter();
		u32 oldestSeq = 0;
		while (const SElem* pOther = iter.Current())
		{
			if (pOther->data != ctx.ctxObj.main->vAspectProfiles[ctx.index])
				return true;
			oldestSeq = pOther->seq;
			iter.Next();
		}
		return oldestSeq > ctx.basisSeq;
	}
	NET_ASSERT(false);
	return false;
}

bool CProfileHistory::SendCurrentValue(const SSendContext& ctx, CSyncContext* pSyncContext, u32& newValue)
{
	ctx.pSender->BeginMessage(ctx.pView->GetConfig().pSetAspectProfileMsgs[ctx.index]);
	u8 profile = ctx.ctxObj.main->vAspectProfiles[ctx.index];
	ctx.pSender->ser.Value("profile", profile);
	newValue = profile;
	return true;
}
