/********************************************************************
Dinrus Source File.
Copyright (C), Dinrus Studios, 2001-2008.
-------------------------------------------------------------------------
Имя файла:   AIBubblesSystemImpl.h
Описание: Central class to create and display speech bubbles over the
AI agents to notify messages to the designers

-------------------------------------------------------------------------
История:
- 09:11:2011 : Created by Francesco Roccucci
*********************************************************************/

#ifndef __AIBubblesSystem_h__
#define __AIBubblesSystem_h__

#include <drx3D/AI/AIBubblesSystem.h>

#ifdef DRXAISYS_DEBUG

class CAIBubblesSystem : public IAIBubblesSystem
{
public:
	// IAIBubblesSystem
	virtual void Init() override;
	virtual void Reset() override;
	virtual SBubbleRequestId QueueMessage(tukk messageName, const SAIBubbleRequest& request) override;
	virtual bool CancelMessageBubble(const EntityId entityId, const SBubbleRequestId& bubbleRequestId) override;
	virtual void Update() override;
	virtual void SetNameFilter(tukk szMessageNameFilter) override;
	// ~IAIBubblesSystem

private:

	// SAIBubbleRequestContainer needs only to be known internally in the
	// BubblesNotifier
	struct SAIBubbleRequestContainer
	{
		SAIBubbleRequestContainer(const SBubbleRequestId& _requestId, u32k _messageNameId, const SAIBubbleRequest& _request)
			:messageNameId(_messageNameId)
			,requestId(_requestId)
			,expiringTime(.0f)
			,request(_request)
			,startExpiringTime(true)
			,neverExpireFromTime(false)
		{
		}

		const SAIBubbleRequest& GetRequest()
		{
			if(startExpiringTime)
			{
				neverExpireFromTime = request.IsDurationInfinite();
				if (!neverExpireFromTime)
				{
					UpdateDuration(request.GetDuration());
				}
				startExpiringTime = false;
			}
			return request;
		}
		u32 GetMessageNameId() const { return messageNameId; }
		const SBubbleRequestId& GetRequestId() const { return requestId; }

		bool IsOld (const CTimeValue currentTimestamp) const
		{
			return !neverExpireFromTime && (currentTimestamp > expiringTime);
		}

	private:
		void UpdateDuration(const float duration)
		{
			expiringTime = gEnv->pTimer->GetFrameStartTime() + (duration ? CTimeValue(duration) : CTimeValue(gAIEnv.CVars.BubblesSystemDecayTime));
		}

		u32           messageNameId;
		SBubbleRequestId requestId;
		CTimeValue       expiringTime;
		bool             startExpiringTime;
		bool             neverExpireFromTime;
		SAIBubbleRequest request;
	};

	struct SRequestByMessageNameFinder
	{
		SRequestByMessageNameFinder(u32 _messageNameId) : messageNameId(_messageNameId) {}
		bool operator()(const SAIBubbleRequestContainer& container) const {return container.GetMessageNameId() == messageNameId;}
		u32 messageNameId;
	};

	struct SRequestByRequestIdFinder
	{
		SRequestByRequestIdFinder(const SBubbleRequestId& _requestId) : requestId(_requestId) {}
		bool operator()(const SAIBubbleRequestContainer& container) const {return container.GetRequestId() == requestId;}
		SBubbleRequestId requestId;
	};

	class CBubbleRender;

	bool DisplaySpeechBubble(SAIBubbleRequestContainer& requestContainer, CBubbleRender& bubbleRender) const;

	// Information logging/popping up
	void LogMessage(tukk const message, const TBubbleRequestOptionFlags flags) const;
	void PopupBlockingAlert(tukk const message, const TBubbleRequestOptionFlags flags) const;

	bool ShouldSuppressMessageVisibility(const SAIBubbleRequest::ERequestType requestType = SAIBubbleRequest::eRT_ErrorMessage) const;
	bool ShouldFilterOutMessage(u32k messsageNameUniqueId) const;

	typedef std::list<SAIBubbleRequestContainer> RequestsList;
	typedef std::unordered_map<EntityId, RequestsList, stl::hash_uint32> EntityRequestsMap;
	EntityRequestsMap	m_entityRequestsMap;

	typedef std::unordered_set<u32> MessageNameFilterSet;
	MessageNameFilterSet m_messageNameFilterSet;

	SBubbleRequestId m_requestIdCounter;
};
#endif // DRXAISYS_DEBUG

#endif // __AIBubblesSystem_h__
