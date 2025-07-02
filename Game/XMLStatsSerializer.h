// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: Implements statistic serializers
				 This one serializes to a file based on kiev game code

	-------------------------------------------------------------------------
	История:
	- 10:11:2009  : Created by Mark Tully

*************************************************************************/


#ifndef __XMLSTATSSERIALIZER_H__
#define __XMLSTATSSERIALIZER_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <drx3D/CoreX/Game/IGameStatistics.h>
#include <drx3D/Sys/XML/IXml.h>

//////////////////////////////////////////////////////////////////////////

struct SStatNode
{
	typedef std::map<u32, SStatNode*> TNodes;

	SNodeLocator						locator;
	XmlNodeRef							xml;
	SStatNode*							parent;
	TNodes									children;
	IStatsContainer&				container;

	SStatNode(const SNodeLocator& loc, IStatsContainer& cont, SStatNode* prnt = 0);
	SStatNode* addOrFindChild(const SNodeLocator& loc, IStatsContainer& cont);
	void removeChild(const SNodeLocator& loc);
};

//////////////////////////////////////////////////////////////////////////

class CXMLStatsSerializer : public IStatsSerializer
{
public:
	CXMLStatsSerializer(IGameStatistics* pGS, CStatsRecordingMgr* pMissionStats);
	virtual void VisitNode(const SNodeLocator& locator, tukk serializeName, IStatsContainer& container, EStatNodeState state);
	virtual void LeaveNode(const SNodeLocator& locator, tukk serializeName, IStatsContainer& container, EStatNodeState state);

private:
	void OnNodeSaved(const SNodeLocator& locator, XmlNodeRef node);
	void SaveContainerData(const IStatsContainer& container, XmlNodeRef node);
	void SaveEventTrack(XmlNodeRef n, tukk name, const IStatsContainer& container, size_t eventID);
	void SaveStatValToXml(XmlNodeRef node, tukk name, const SStatAnyValue& val);

	SStatNode* m_rootNode;
	SStatNode* m_currentNode;
	IGameStatistics* m_stats;
	CStatsRecordingMgr* m_statsRecorder;
};

//////////////////////////////////////////////////////////////////////////

#endif // __XMLSTATSSERIALIZER_H__
