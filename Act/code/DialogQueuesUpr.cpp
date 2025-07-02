// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/DialogQueuesUpr.h>
#include <drx3D/Act/DrxActionCVars.h>

////////////////////////////////////////////////////////////////////////////
CDialogQueuesUpr::CDialogQueuesUpr()
	: m_numBuffers(0), m_uniqueDialogID(0)
{
	static tukk BUFFERS_FILENAME = "Libs/FlowNodes/DialogFlowNodeBuffers.xml";

	XmlNodeRef xmlNodeRoot = GetISystem()->LoadXmlFromFile(BUFFERS_FILENAME);
	if (xmlNodeRoot == (IXmlNode*)NULL)
	{
		if (!gEnv->IsEditor())
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CDialogFlowNodeMgr::Init() - Failed to load '%s'. dialog flownode buffers disabled.", BUFFERS_FILENAME);
		return;
	}

	m_numBuffers = xmlNodeRoot->getChildCount() - 1; // first one in the file is assumed to be "nobuffer"
	m_buffersList.resize(m_numBuffers);
	u32k DEFAULT_BUFFER_SIZE = 4; // just an small size that should be big enough for almost all situations, to avoid extra memory allocations.
	for (u32 i = 0; i < m_numBuffers; ++i)
	{
		m_buffersList[i].reserve(DEFAULT_BUFFER_SIZE);
	}

#ifdef DEBUGINFO_DIALOGBUFFER
	m_bufferNames.resize(m_numBuffers);
	for (u32 i = 0; i < m_numBuffers; ++i)
	{
		XmlNodeRef xmlNodeBuffer = xmlNodeRoot->getChild(i + 1);
		m_bufferNames[i] = xmlNodeBuffer->getAttr("name");
	}
#endif
}

////////////////////////////////////////////////////////////////////////////
void CDialogQueuesUpr::Reset()
{
	m_uniqueDialogID = 0;
	for (u32 i = 0; i < m_numBuffers; ++i)
	{
		m_buffersList[i].clear();
	}
#ifdef DEBUGINFO_DIALOGBUFFER
	m_dialogNames.clear();
#endif
}

////////////////////////////////////////////////////////////////////////////
bool CDialogQueuesUpr::IsBufferFree(u32 queueID)
{
	if (IsQueueIDValid(queueID))
		return m_buffersList[queueID].size() == 0;
	else
		return true;
}

////////////////////////////////////////////////////////////////////////////
CDialogQueuesUpr::TDialogId CDialogQueuesUpr::Play(u32 queueID, const string& name)
{
	TDialogId dialogId = CreateNewDialogId();
	if (IsQueueIDValid(queueID))
	{
		m_buffersList[queueID].push_back(dialogId);
#ifdef DEBUGINFO_DIALOGBUFFER
		if (CDrxActionCVars::Get().g_debugDialogBuffers == 1)
			m_dialogNames.insert(std::make_pair(dialogId, name));
#endif
	}
	return dialogId;
}

////////////////////////////////////////////////////////////////////////////
bool CDialogQueuesUpr::IsDialogWaiting(u32 queueID, TDialogId dialogId)
{
	if (IsQueueIDValid(queueID))
	{
		TBuffer& buffer = m_buffersList[queueID];
		assert(!buffer.empty());
		return buffer[0] != dialogId;
	}
	else
		return false;
}

////////////////////////////////////////////////////////////////////////////
void CDialogQueuesUpr::NotifyDialogDone(u32 queueID, TDialogId dialogId)
{
	if (IsQueueIDValid(queueID))
	{
		TBuffer& buffer = m_buffersList[queueID];
		// this is called at most once per dialog, and the vectors are very small (usually a couple elements or 3 at most),
		// and the element we want to delete should almost always be the first one, so we dont care about performance hit
		stl::find_and_erase(buffer, dialogId);
#ifdef DEBUGINFO_DIALOGBUFFER
		if (CDrxActionCVars::Get().g_debugDialogBuffers == 1)
			m_dialogNames.erase(dialogId);
#endif
	}
}

////////////////////////////////////////////////////////////////////////////
u32 CDialogQueuesUpr::BufferEnumToId(u32 bufferEnum) // buffer enum means the values stored in the .xml definition file
{
	u32 queueID = bufferEnum - 1; // this assumes that the "nobuffer" enum value is the 0
	assert(IsQueueIDValid(queueID) || queueID == NO_QUEUE);
	return queueID;
}

////////////////////////////////////////////////////////////////////////////
void CDialogQueuesUpr::Serialize(TSerialize ser)
{
	ser.Value("m_numBuffers", m_numBuffers);
	ser.Value("m_dialogIdCount", m_uniqueDialogID);

	ser.BeginGroup("buffers");
	for (i32 i = 0; i < m_numBuffers; ++i)
	{
		ser.Value("buffer", m_buffersList[i]);
	}
	ser.EndGroup();
}

////////////////////////////////////////////////////////////////////////////
void CDialogQueuesUpr::Update()
{
#ifdef DEBUGINFO_DIALOGBUFFER
	if (CDrxActionCVars::Get().g_debugDialogBuffers == 1)
	{
		for (u32 i = 0; i < m_numBuffers; ++i)
		{
			TBuffer& buffer = m_buffersList[i];

			float white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			float green[] = { 0.0f, 1.0f, 1.0f, 1.0f };
			float red[] = { 1.0f, 0.0f, 0.0f, 1.0f };
			float x = 300.f * i;
			float y = 100;
			IRenderAuxText::Draw2dLabel(x, y, 1.2f, white, false, "%s", m_bufferNames[i].c_str());
			IRenderAuxText::Draw2dLabel(x, y + 20, 1.2f, white, false, "------------------------------------");

			for (u32 j = 0; j < buffer.size(); ++j)
			{
				tukk pName = "<UNKNOWN>";
				TDialogNames::const_iterator iter = m_dialogNames.find(buffer[j]);
				if (iter != m_dialogNames.end())
					pName = iter->second.c_str();
				IRenderAuxText::Draw2dLabel(x, y + 40 + 20 * j, 1.2f, j == 0 ? green : red, false, "%s", pName);
			}
		}
	}
#endif
}
