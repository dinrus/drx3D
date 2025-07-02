// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//////////////////////////////////////////////////////////////////////////

#include <drx3D/LiveCreate/StdAfx.h>
#include <drx3D/LiveCreate/LiveCreateHost.h>

#ifndef NO_LIVECREATE
	#include <drx3D/Network/IServiceNetwork.h>
	#include <drx3D/Network/IRemoteCommand.h>
	#include <drx3D/LiveCreate/LiveCreateUpr.h>
	#include <drx3D/LiveCreate/LiveCreateNativeFile.h>
	#include <drx3D/LiveCreate/LiveCreateCommands.h>
	#include <drx3D/Sys/ICmdLine.h>
	#include <drx3D/CoreX/Platform/IPlatformOS.h>
	#include <drx3D/Network/DrxSocks.h>
	#include <drx3D/LiveCreate/PlatformHandler_GamePC.h>

namespace LiveCreate
{

//-----------------------------------------------------------------------------

CPlatformService::CPlatformService(CHost* pHost, u16k listeningPort /*= kDefaultDiscoverySerivceListenPost*/)
	: m_socket(0)
	, m_pHost(pHost)
	, m_port(listeningPort)
	, m_bQuiet(false)
{
	Bind();

	if (!gEnv->pThreadUpr->SpawnThread(this, "LiveCreatePlatformService"))
	{
		DrxFatalError("Error spawning \"LiveCreatePlatformService\" thread.");
	}
}

CPlatformService::~CPlatformService()
{
	if (m_socket > 0)
	{
		DrxSock::shutdown(m_socket, 2);
		DrxSock::closesocket(m_socket);
		m_socket = 0;
	}
}

bool CPlatformService::Bind()
{
	// close current socket
	if (m_socket > 0)
	{
		DrxSock::shutdown(m_socket, 2);
		DrxSock::closesocket(m_socket);
		m_socket = 0;
	}

	// create new listening socket
	DRXSOCKET s = DrxSock::socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	{
		gEnv->pLog->LogWarning("LiveCreate discovery service NOT STARTED: socket error %d", s);
	}
	else
	{
		// bind socket to specified port
		DRXSOCKADDR_IN servaddr;
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port = htons(m_port);
		i32k ret = DrxSock::bind(s, (struct DRXSOCKADDR*)&servaddr, sizeof(servaddr));
		if (ret < 0)
		{
			gEnv->pLog->LogWarning("LiveCreate discovery service NOT STARTED: bind error %d", ret);
		}
		else
		{
			gEnv->pLog->Log("LiveCreate discovery service started at port %d", m_port);
			m_socket = s;
		}

		// set socket timeout
		DrxSock::SetRecvTimeout(s, 1, 0);
	}

	return m_socket != 0;
}

tukk GetPlatformName()
{
	#if DRX_PLATFORM_ORBIS
	return "PS4";
	#elif DRX_PLATFORM_DURANGO
	return "XBOXONE";
	#elif defined(_WINDOWS_)
	return "PC";
	#else
	return "UNKNOWN";
	#endif
}

bool CPlatformService::Process(IDataReadStream& reader, IDataWriteStream& writer)
{
	const string cmd(reader.ReadString());

	if (cmd == "Cmd")
	{
		const string consoleCmd(reader.ReadString());
		gEnv->pConsole->ExecuteString(consoleCmd.c_str(), false, true);
		return true;
	}
	else if (cmd == "SuppressCommands")
	{
		m_pHost->SuppressCommandExecution();
		return true;
	}
	else if (cmd == "GetInfo")
	{
		CHostInfoPacket info;
		m_pHost->FillHostInfo(info);

		writer.WriteString("Info");
		info.Serialize(writer);
		return true;
	}

	return false;
}

void CPlatformService::ThreadEntry()
{
	while (!m_bQuiet)
	{
		DRXSOCKADDR_IN addr;
		DRXSOCKLEN addrLen = sizeof(addr);
		char buffer[1000];

		// get data from connection
		i32 length = DrxSock::recvfrom(m_socket, buffer, sizeof(buffer), 0, (DRXSOCKADDR*)&addr, &addrLen);
		if (length < 0)
		{
			const DrxSock::eDrxSockError errCode = DrxSock::TranslateSocketError(length);
			if (errCode == DrxSock::eCSE_EWOULDBLOCK || errCode == DrxSock::eCSE_ETIMEDOUT)
			{
				// just a timeout
				DrxSleep(500);
				continue;
			}

			gEnv->pLog->LogWarning("LiveCreate discovery service socket error: %d", length);

			// try to restart
			if (!Bind())
			{
				gEnv->pLog->LogWarning("LiveCreate: Failed to rebind platform service socket. Closing service.");
				break;
			}

			continue;
		}

		// process data
		TAutoDelete<IDataReadStream> reader(gEnv->pServiceNetwork->CreateMessageReader(buffer, length));
		if (reader)
		{
			TAutoDelete<IDataWriteStream> writer(gEnv->pServiceNetwork->CreateMessageWriter());
			if (writer)
			{
				if (Process(reader, writer))
				{
					// send the response data - size of the data is limited
					char responseBuffer[1024];
					DRX_ASSERT(writer->GetSize() < sizeof(responseBuffer));
					if (writer->GetSize() < sizeof(responseBuffer))
					{
						writer->CopyToBuffer(responseBuffer);

						// send the message back
						i32 ret = DrxSock::sendto(m_socket, responseBuffer, writer->GetSize(), 0, (DRXSOCKADDR*)&addr, addrLen);
						if (ret < 0)
						{
							gEnv->pLog->LogWarning("LiveCreate: sendto() error: %d", ret);
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------

CUtilityService::FileTransferInProgress::FileTransferInProgress(IServiceNetworkConnection* pConnection, const string& destFilePath, FILE* destFile, u32k fileSize)
	: m_pConnection(pConnection)
	, m_destFilePath(destFilePath)
	, m_destFile(destFile)
	, m_fileSize(fileSize)
	, m_fileDataLeft(fileSize)
	, m_startTime(gEnv->pTimer->GetAsyncTime())
{
	if (!gEnv->pThreadUpr->SpawnThread(this, "LiveCreateUtilityService"))
	{
		DrxFatalError("Error spawning \"LiveCreateUtilityService\" thread.");
	}
}

CUtilityService::FileTransferInProgress::~FileTransferInProgress()
{
	// close output file
	gEnv->pDrxPak->FClose(m_destFile);

	// delete file if not transfered completely
	if (m_fileDataLeft > 0)
	{
		gEnv->pDrxPak->RemoveFile(m_destFilePath.c_str());
	}

	// close connection
	if (NULL != m_pConnection)
	{
		m_pConnection->Close();
		m_pConnection->Release();
		m_pConnection = NULL;
	}
}

bool CUtilityService::FileTransferInProgress::Update()
{
	// connection is dead :(
	if (!m_pConnection->IsAlive())
	{
		gEnv->pLog->LogWarning("FileTransfer for '%s': connection interrupted (data left: %d)", m_destFilePath.c_str(), m_fileDataLeft);
		return true;
	}

	// handle new data
	IServiceNetworkMessage* pMessage = m_pConnection->ReceiveMsg();
	while (NULL != pMessage)
	{
		u8k* pData = (u8k*)pMessage->GetPointer();
		u32k dataSize = pMessage->GetSize();
		if (dataSize > m_fileDataLeft)
		{
			gEnv->pLog->LogWarning("FileTransfer for '%s': data overflow (%d>%d)", m_destFilePath.c_str(), dataSize, m_fileDataLeft);
			pMessage->Release();
			return true;
		}

		// write data to file
		u32k writtenSize = gEnv->pDrxPak->FWrite(pData, 1, dataSize, m_destFile);
		if (writtenSize != dataSize)
		{
			gEnv->pLog->LogWarning("FileTransfer for '%s': write error (%d requested, %d written)", m_destFilePath.c_str(), dataSize, writtenSize);
			pMessage->Release();
			return true;
		}

		pMessage->Release();
		pMessage = m_pConnection->ReceiveMsg();
		m_fileDataLeft -= dataSize;
	}

	// finished
	if (m_fileDataLeft == 0)
	{
		const float time = gEnv->pTimer->GetAsyncTime().GetDifferenceInSeconds(m_startTime);
		gEnv->pLog->LogAlways("FileTransfer for '%s' completed in %1.2fs (%1.2f MB/s)",
		                      m_destFilePath.c_str(), time, ((float)m_fileSize / time) / (1024.0f * 1024.0f));
	}

	// close when completed
	return m_fileDataLeft == 0;
}

CUtilityService::CUtilityService(class CHost* pHost, u16k listeningPort)
	: m_pHost(pHost)
	, m_bQuit(false)
{
	m_pListener = gEnv->pServiceNetwork->CreateListener(listeningPort);

	if (!gEnv->pThreadUpr->SpawnThread(this, "LiveCreateUtilityService"))
	{
		DrxFatalError("Error spawning \"LiveCreateUtilityService\" thread.");
	}
}

CUtilityService::~CUtilityService()
{
	// close all files
	for (TFileTransferList::const_iterator it = m_fileTransfers.begin();
	     it != m_fileTransfers.end(); ++it)
	{
		delete (*it);
	}

	m_fileTransfers.clear();
}

void CUtilityService::ThreadEntry()
{
	typedef std::vector<IServiceNetworkConnection*> TConnections;
	TConnections createdConnections;

	while (!m_bQuit)
	{
		// any new connections ?
		IServiceNetworkConnection* pNewConnection = m_pListener->Accept();
		while (NULL != pNewConnection)
		{
			createdConnections.push_back(pNewConnection);
			pNewConnection = m_pListener->Accept();
		}

		// wait for initialization packet
		for (TConnections::iterator it = createdConnections.begin();
		     it != createdConnections.end(); )
		{
			IServiceNetworkConnection* pConnection = (*it);

			if (!pConnection->IsAlive())
			{
				pConnection->Close();
				pConnection->Release();
				it = createdConnections.erase(it);
			}
			else
			{
				IServiceNetworkMessage* pMessages = pConnection->ReceiveMsg();
				if (NULL != pMessages)
				{
					TAutoDelete<IDataReadStream> reader(pMessages->CreateReader());
					TAutoDelete<IDataWriteStream> writer(gEnv->pServiceNetwork->CreateMessageWriter());

					const string destPath = reader->ReadString();
					u32k id = reader->ReadUint32();
					u32k fileSize = reader->ReadUint32();

					gEnv->pLog->LogAlways("FileTransfer ID%d request received for file '%s', size %d.",
					                      id, destPath.c_str(), fileSize);

					// do we already have a file transfer request for this file ? if so - delete it
					for (TFileTransferList::iterator jt = m_fileTransfers.begin();
					     jt != m_fileTransfers.end(); ++jt)
					{
						FileTransferInProgress* pCurrentTransfer = (*jt);
						if (pCurrentTransfer->GetFileName() == destPath)
						{
							gEnv->pLog->LogAlways("FileTransfer for file '%s' already pending: canceling it.",
							                      destPath.c_str());

							delete pCurrentTransfer;
							m_fileTransfers.erase(jt);
							break;
						}
					}

					// create the access for output file
					FILE* pDestFile = gEnv->pDrxPak->FOpen(destPath.c_str(), "wb");
					if (NULL == pDestFile)
					{
						gEnv->pLog->LogAlways("FileTransfer ID%d, file '%s' error: cannot open file to writing.",
						                      id, destPath.c_str());

						// response
						writer->WriteUint8(0);
					}
					else
					{
						FileTransferInProgress* pFileTransfer = new FileTransferInProgress(pConnection, destPath, pDestFile, fileSize);
						m_fileTransfers.push_back(pFileTransfer);

						// response
						writer->WriteUint8(1);
					}

					// send response
					IServiceNetworkMessage* pMessage = writer->BuildMessage();
					pConnection->SendMsg(pMessage);
					SAFE_RELEASE(pMessage);

					// if not file was opened and no file transfer was started we can close the connection
					if (NULL == pDestFile)
					{
						pConnection->FlushAndClose();
					}

					// connection was transfered to file transfer list or destroyed
					it = createdConnections.erase(it);
				}
				else
				{
					// no message yet
					++it;
				}
			}
		}

		// update file transfers
		for (TFileTransferList::iterator it = m_fileTransfers.begin();
		     it != m_fileTransfers.end(); )
		{
			FileTransferInProgress* pFileTransfer = (*it);

			if (pFileTransfer->Update())
			{
				// done
				it = m_fileTransfers.erase(it);
				delete pFileTransfer;
			}
			else
			{
				++it;
			}
		}
	}
}

//-----------------------------------------------------------------------------

CHost::CHost()
	: m_pCommandServer(NULL)
	, m_updateTimeOfDay(false)
	, m_bCommandsSuppressed(false)
	, m_pPlatformService(NULL)
	, m_pUtilityService(NULL)
{
}

CHost::~CHost()
{
	// make sure we close the host interface
	CHost::Close();
}

bool CHost::Initialize(u16k listeningPort /* = kDefaultHostListenPort*/,
                       u16k discoveryServiceListeningPort /*= kDefaultDiscoverySerivceListenPost*/,
                       u16k fileTransferServiceListeningPort /*= kDefaultFileTransferServicePort*/)
{
	// Create the command manager
	m_pCommandServer = gEnv->pRemoteCommandUpr->CreateServer(listeningPort);
	if (NULL == m_pCommandServer)
	{
		return false;
	}

	// Register host as listener to raw communication on the command stream
	m_pCommandServer->RegisterAsyncMessageListener(this);
	m_pCommandServer->RegisterSyncMessageListener(this);

	// Create discovery service thread
	if (discoveryServiceListeningPort != 0)
	{
		m_pPlatformService = new CPlatformService(this, discoveryServiceListeningPort);
	}

	// Create file transfer service thread
	if (fileTransferServiceListeningPort != 0)
	{
		m_pUtilityService = new CUtilityService(this, fileTransferServiceListeningPort);
	}

	return true;
}

void CHost::Close()
{
	// Force close the command server
	if (NULL != m_pCommandServer)
	{
		m_pCommandServer->UnregisterAsyncMessageListener(this);
		m_pCommandServer->UnregisterSyncMessageListener(this);
		m_pCommandServer->Delete();
		m_pCommandServer = NULL;
	}

	// Stop discovery service
	if (NULL != m_pPlatformService)
	{
		m_pPlatformService->SingnalStopWork();
		gEnv->pThreadUpr->JoinThread(m_pPlatformService);
		delete m_pPlatformService;
		m_pPlatformService = NULL;
	}

	// Stop file transfer service (will delete all in-flight files)
	if (NULL != m_pUtilityService)
	{
		m_pUtilityService->SignalStopWork();
		gEnv->pThreadUpr->JoinThread(m_pUtilityService);
		delete m_pUtilityService;
		m_pUtilityService = NULL;
	}
}

void CHost::SuppressCommandExecution()
{
	// pass directly to remove command server
	if (NULL != m_pCommandServer && !m_bCommandsSuppressed)
	{
		m_pCommandServer->SuppressCommands();
		m_bCommandsSuppressed = true;

		// broadcast "Busy" message
		{
			IDataWriteStream* pStream = gEnv->pServiceNetwork->CreateMessageWriter();
			if (NULL != pStream)
			{
				pStream->WriteString("Busy");

				IServiceNetworkMessage* pMessage = pStream->BuildMessage();
				if (NULL != pMessage)
				{
					m_pCommandServer->Broadcast(pMessage);
					pMessage->Release();
				}

				pStream->Delete();
			}
		}
	}
}

void CHost::ResumeCommandExecution()
{
	// pass directly to remove command server
	if (NULL != m_pCommandServer && m_bCommandsSuppressed)
	{
		m_pCommandServer->ResumeCommands();
		m_bCommandsSuppressed = false;

		// broadcast "Ready" message
		{
			IDataWriteStream* pStream = gEnv->pServiceNetwork->CreateMessageWriter();
			if (NULL != pStream)
			{
				pStream->WriteString("Ready");

				IServiceNetworkMessage* pMessage = pStream->BuildMessage();
				if (NULL != pMessage)
				{
					m_pCommandServer->Broadcast(pMessage);
					pMessage->Release();
				}

				pStream->Delete();
			}
		}
	}
}

void CHost::DrawOverlay()
{
	DrawSelection();
}

void CHost::ExecuteCommands()
{
	// execute RPC commands
	if (NULL != m_pCommandServer)
	{
		m_pCommandServer->FlushCommandQueue();
	}

	if (m_updateTimeOfDay)
	{
		m_updateTimeOfDay = false;
		gEnv->p3DEngine->GetTimeOfDay()->Update(true, true);
	}
}

bool CHost::OnRawMessageSync(const ServiceNetworkAddress& remoteAddress, IDataReadStream& msg, IDataWriteStream& response)
{
	const string cmd = msg.ReadString();

	// External console command (usually "map", "unload", and "quit")
	if (cmd == "ConsoleCommand")
	{
		// read unique console command ID
		const uint64 lo = msg.ReadUint64();
		const uint64 hi = msg.ReadUint64();
		const DrxGUID id = DrxGUID::Construct(hi, lo);

		// read command string
		const string cmdString = msg.ReadString();

		if (cmdString == "SuppressCommands")
		{
			SuppressCommandExecution();
		}
		else if (cmdString == "ResumeCommands")
		{
			ResumeCommandExecution();
		}
		else
		{
			// was already executed ?
			TLastCommandIDs::const_iterator it = m_lastConsoleCommandsExecuted.find(id);
			if (it != m_lastConsoleCommandsExecuted.end())
			{
				gEnv->pLog->Log("LiveCreate: command '%s' already executed. Skipping.", cmdString.c_str());
			}
			else
			{
				m_lastConsoleCommandsExecuted.insert(id);
				gEnv->pConsole->ExecuteString(cmdString.c_str(), false, true);
			}
		}

		// write general OK response
		response.WriteString("CmdResponse");
		response.WriteUint64(lo);
		response.WriteUint64(hi);
		response.WriteString("OK");

		return true;
	}

	// not processed
	return false;
}

bool CHost::OnRawMessageAsync(const ServiceNetworkAddress& remoteAddress, IDataReadStream& msg, IDataWriteStream& response)
{
	const string cmd = msg.ReadString();

	// Ready flag
	if (cmd == "GetReadyFlag")
	{
		if (m_bCommandsSuppressed)
		{
			response.WriteString("Busy");
		}
		else
		{
			response.WriteString("Ready");
		}

		return true;
	}

	// not processed
	return false;
}

bool CHost::GetEntityCRC(EntityId id, u32& outCrc) const
{
	TEntityCRCMap::const_iterator it = m_entityCrc.find(id);
	if (it != m_entityCrc.end())
	{
		outCrc = it->second;
		return true;
	}
	else
	{
		return false;
	}
}

void CHost::SetEntityCRC(EntityId id, u32k outCrc)
{
	m_entityCrc[id] = outCrc;
}

void CHost::RemoveEntityCRC(EntityId id)
{
	TEntityCRCMap::iterator it = m_entityCrc.find(id);
	if (it != m_entityCrc.end())
	{
		m_entityCrc.erase(it);
	}
}

bool CHost::SyncFullLevelState(tukk szFileName)
{
	string fullPath = szFileName;
	IDataReadStream* pStream = CLiveCreateFileReader::CreateReader(fullPath);
	if (NULL == pStream)
	{
		gEnv->pLog->LogAlways("LiveCreate full sync failed: failed to open file '%s'", (tukk)fullPath);
		return false;
	}

	// get the data header
	u32k magic = pStream->ReadUint32();
	if (magic != 'LCFS')
	{
		gEnv->pLog->LogAlways("LiveCreate full sync failed: invalid file '%s' header (0x%X)", (tukk)fullPath, magic);
		return false;
	}

	// Flush rendering thread :( another length stuff that is unfortunately needed here because it crashes without it
	gEnv->pRenderer->FlushRTCommands(true, true, true);

	// sync entity system (layers & entities)
	gEnv->pEntitySystem->LoadInternalState(*pStream);

	// sync objects system (brushes, etc)
	{
		u32k kMaxLayers = 1024;

		// get mask for visible layers
		u8 layerMask[kMaxLayers / 8];
		memset(layerMask, 0, sizeof(layerMask));
		gEnv->pEntitySystem->GetVisibleLayerIDs(layerMask, kMaxLayers);

		// layer mapping
		std::vector<string> layerNames;
		std::vector<u16> layerIds;
		*pStream << layerNames;
		*pStream << layerIds;

		// load layer names
		u16 layerIdTable[kMaxLayers];
		memset(layerIdTable, 0, sizeof(layerIdTable));
		for (u32 i = 0; i < layerNames.size(); ++i)
		{
			u16k dataId = layerIds[i];
			if (dataId < kMaxLayers)
			{
				i32k layerId = gEnv->pEntitySystem->GetLayerId(layerNames[i].c_str());
				if (layerId != -1)
				{
					layerIdTable[dataId] = (u16)layerId;
				}
			}
		}

		gEnv->p3DEngine->LoadInternalState(*pStream, layerMask, layerIdTable);
	}

	// parse the CVars
	gEnv->pConsole->LoadInternalState(*pStream);

	// parse the TOD dataa
	gEnv->p3DEngine->GetTimeOfDay()->LoadInternalState(*pStream);

	// done
	return true;
}

void CHost::RequestTimeOfDayUpdate()
{
	m_updateTimeOfDay = true;
}

void CHost::FillHostInfo(CHostInfoPacket& outHostInfo) const
{
	outHostInfo.platformName = GetPlatformName();
	outHostInfo.hostName = gEnv->pSystem->GetPlatformOS()->GetHostName();
	outHostInfo.gameFolder = gEnv->pDrxPak->GetGameFolder();
	outHostInfo.rootFolder = gEnv->pSystem->GetRootFolder();
	outHostInfo.bAllowsLiveCreate = true;
	outHostInfo.bHasLiveCreateConnection = m_pCommandServer ? m_pCommandServer->HasConnectedClients() : false;
	outHostInfo.screenWidth = gEnv->pRenderer ? gEnv->pRenderer->GetWidth() : 0;
	outHostInfo.screenHeight = gEnv->pRenderer ? gEnv->pRenderer->GetHeight() : 0;

	if (gEnv->pGameFramework)
	{
		ILevelSystem* pSystem = gEnv->pGameFramework->GetILevelSystem();
		if (NULL != pSystem)
		{
			ILevel* pLevel = pSystem->GetCurrentLevel();
			if (NULL != pLevel && (NULL != pLevel->GetLevelInfo()))
			{
				outHostInfo.currentLevel = pLevel->GetLevelInfo()->GetName();
			}
		}
	}

	#if DRX_PLATFORM_WINAPI
	// extract valid executable path from module name
	char szExecutablePath[MAX_PATH];
	drx_strcpy(szExecutablePath, gEnv->pSystem->GetICmdLine()->GetArg(0)->GetValue());
	for (size_t i = 0; i < MAX_PATH; ++i)
	{
		if (szExecutablePath[i] == '/')
		{
			szExecutablePath[i] = '\\';
		}
	}
	outHostInfo.buildExecutable = szExecutablePath;

	// extract valid build path from executable name
	tuk ch = strrchr(szExecutablePath, '\\');
	if (ch != NULL)
	{
		ch[1] = 0;
	}
	outHostInfo.buildDirectory = szExecutablePath;
	#endif
}

void CHost::ClearSelectionData()
{
	m_selection.clear();
}

void CHost::SetSelectionData(const std::vector<SSelectedObject>& selectedObjects)
{
	m_selection = selectedObjects;
}

void CHost::DrawSelection()
{
	for (TSelectionObjects::const_iterator it = m_selection.begin();
	     it != m_selection.end(); ++it)
	{
		const SSelectedObject& obj = (*it);

		IRenderAuxGeom* pAux = gEnv->pRenderer->GetIRenderAuxGeom();

		// bounding box
		const ColorB lineColor(255, 255, 128, 255);
		if (!obj.m_box.IsEmpty())
		{
			pAux->DrawAABB(obj.m_box, obj.m_matrix, false, lineColor, eBBD_Faceted);
		}

		// pivot
		const float s = 0.1f;
		pAux->DrawLine(obj.m_position - Vec3(s, 0, 0), lineColor,
		               obj.m_position + Vec3(s, 0, 0), lineColor, 2.0f);
		pAux->DrawLine(obj.m_position - Vec3(0, s, 0), lineColor,
		               obj.m_position + Vec3(0, s, 0), lineColor, 2.0f);
		pAux->DrawLine(obj.m_position - Vec3(0, 0, s), lineColor,
		               obj.m_position + Vec3(0, 0, s), lineColor, 2.0f);

		// name
		if (!obj.m_name.empty())
		{
			IRenderAuxText::DrawText(obj.m_position, Vec2(2), NULL, eDrawText_Center | eDrawText_FixedSize | eDrawText_800x600, obj.m_name.c_str());
		}
	}
}

}

#endif
