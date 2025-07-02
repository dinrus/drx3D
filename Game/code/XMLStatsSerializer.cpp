// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

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

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/StatsRecordingMgr.h>
#include <drx3D/CoreX/Game/IGameStatistics.h>
#include <drx3D/Game/XMLStatsSerializer.h>


//////////////////////////////////////////////////////////////////////////
// CXMLSerializer
//////////////////////////////////////////////////////////////////////////

CXMLStatsSerializer::CXMLStatsSerializer(IGameStatistics *pGS, CStatsRecordingMgr* inRecorder)
: m_stats(pGS)
, m_rootNode(0)
, m_currentNode(0)
, m_statsRecorder(inRecorder)
{
}

//////////////////////////////////////////////////////////////////////////

void CXMLStatsSerializer::VisitNode(const SNodeLocator &locator, tukk serializeName, IStatsContainer& container, EStatNodeState state)
{
	SStatNode* prevNode = m_currentNode;

	if(m_currentNode)
	{
		m_currentNode = m_currentNode->addOrFindChild(locator, container);
	}
	else
	{
		if(!m_rootNode)
		{
			m_rootNode = new SStatNode(locator, container);
			m_currentNode = m_rootNode;
		}
		else
		{
			DRX_ASSERT(m_rootNode->locator == locator);
			m_currentNode = m_rootNode;
		}
	}

	if(!m_currentNode->xml)
	{
		m_currentNode->xml = m_stats->CreateStatXMLNode(serializeName);

		if(prevNode)
			prevNode->xml->addChild(m_currentNode->xml);
	}
}

//////////////////////////////////////////////////////////////////////////

void CXMLStatsSerializer::LeaveNode(const SNodeLocator &locator, tukk serializeName, IStatsContainer& container, EStatNodeState state)
{
	DRX_ASSERT(m_currentNode);
	DRX_ASSERT(locator == m_currentNode->locator);

	SStatNode* curNode = m_currentNode;
	m_currentNode = m_currentNode->parent;

	// Clean up
	if(state == eSNS_Dead)
	{
		SaveContainerData(container, curNode->xml);
		OnNodeSaved(locator, curNode->xml);

		if(m_currentNode)
		{
			m_currentNode->removeChild(locator);
		}
		else
		{
			DRX_ASSERT(locator == m_rootNode->locator);
			delete curNode;
			m_rootNode = 0;
		}
		curNode = 0;
	}
}

//////////////////////////////////////////////////////////////////////////

void CXMLStatsSerializer::OnNodeSaved(const SNodeLocator &locator, XmlNodeRef node)
{
	if(locator.isScope()) // Scope saved
	{
		if(locator.scopeID == eGSC_Session)
			m_statsRecorder->SaveSessionData(node);
	}
}

//////////////////////////////////////////////////////////////////////////

void CXMLStatsSerializer::SaveContainerData(const IStatsContainer& container, XmlNodeRef node)
{
	const size_t numEvents = m_stats->GetEventCount();
	const size_t numStates = m_stats->GetStateCount();
	XmlNodeRef timelines;

	// Save event tracks
	for(size_t e = 0; e != numEvents; ++e)
	{
		if(container.GetEventTrackLength(e))
		{
			if (timelines==NULL)
			{
				timelines = m_stats->CreateStatXMLNode("timelines");
				node->addChild(timelines);
			}
			SaveEventTrack(timelines, m_stats->GetEventDesc(e)->serializeName, container, e);
		}
	}

	// Save states
	for(size_t s = 0; s != numStates; ++s)
	{
		SStatAnyValue val;
		container.GetStateInfo(s, val);

		if(val.IsValidType())
			SaveStatValToXml(node, m_stats->GetStateDesc(s)->serializeName, val);
	}
}

//////////////////////////////////////////////////////////////////////////

void CXMLStatsSerializer::SaveEventTrack(XmlNodeRef n, tukk name, const IStatsContainer& container, size_t eventID)
{
	//////////////////////////////////////////////////////////////////////////
	static tukk SINGLE_STAT_XML_TAG		=	"prm";
	static tukk MULTIPLE_STAT_XML_TAG	=	"param";
	//////////////////////////////////////////////////////////////////////////

	XmlNodeRef c = m_stats->CreateStatXMLNode("timeline");
	c->setAttr("name", name);
	n->addChild(c);

	size_t nEvents = container.GetEventTrackLength(eventID);
	for(size_t i = 0; i != nEvents; ++i)
	{
		CTimeValue time;
		SStatAnyValue param;
		container.GetEventInfo(eventID, i, time, param);

		XmlNodeRef e = m_stats->CreateStatXMLNode("val");
		c->addChild(e);
		e->setAttr("time", time.GetMilliSecondsAsInt64());

		if(!param.IsValidType())
			continue;

		SaveStatValToXml(e, param.type == eSAT_TXML ? MULTIPLE_STAT_XML_TAG : SINGLE_STAT_XML_TAG, param);
	}
}

//////////////////////////////////////////////////////////////////////////

void CXMLStatsSerializer::SaveStatValToXml(XmlNodeRef node, tukk name, const SStatAnyValue& val)
{
	if(val.type == eSAT_TXML && val.pSerializable)
	{
		XmlNodeRef xmlized = val.pSerializable->GetXML(m_stats);
		xmlized->setTag(name);
		node->addChild(xmlized);
	}
	else
	{
		stack_string strValue;
		if(val.ToString(strValue))
			node->setAttr(name, strValue);
	}
}

//////////////////////////////////////////////////////////////////////////
// SStatNode
//////////////////////////////////////////////////////////////////////////

SStatNode::SStatNode(const SNodeLocator& loc, IStatsContainer& cont, SStatNode* prnt)
: locator(loc)
, parent(prnt)
, container(cont)
{
}

//////////////////////////////////////////////////////////////////////////

SStatNode* SStatNode::addOrFindChild(const SNodeLocator& loc, IStatsContainer& cont)
{
	TNodes::const_iterator it = children.find(loc.timeStamp);
	if(it != children.end())
		return it->second;

	SStatNode* newNode = new SStatNode(loc, cont, this);
	children.insert(std::make_pair(loc.timeStamp, newNode));
	return newNode;
}

//////////////////////////////////////////////////////////////////////////

void SStatNode::removeChild(const SNodeLocator &loc)
{
	TNodes::iterator it = children.find(loc.timeStamp);
	if(it != children.end())
	{
		delete it->second;
		children.erase(it);
	}
}

//////////////////////////////////////////////////////////////////////////

