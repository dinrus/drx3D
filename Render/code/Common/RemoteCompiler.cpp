// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   RemoteCompiler.h : socket wrapper for shader compile server connections

   Revision история:
* Created by Michael Kopietz

   =============================================================================*/

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/RemoteCompiler.h>
#include <DinrusXNetwork/DrxSocks.h>

namespace NRemoteCompiler
{

u32 CShaderSrv::m_LastWorkingServer = 0;

CShaderSrv::CShaderSrv()
{
	Init();
}

void CShaderSrv::Init()
{
#ifdef _MSC_VER
	WSADATA Data;
	if (WSAStartup(MAKEWORD(2, 0), &Data))
	{
		iLog->Log("ERROR: CShaderSrv::Init: Could not init root socket\n");
		return;
	}
#endif

	ICVar* pGameFolder = gEnv->pConsole->GetCVar("sys_game_folder");
	ICVar* pCompilerFolderName = CRenderer::CV_r_ShaderCompilerFolderName;

	string gameFolderName = pGameFolder ? pGameFolder->GetString() : "";
	string compilerFolderName = pCompilerFolderName ? pCompilerFolderName->GetString() : "";

	string folder = "";

	if (!compilerFolderName.empty())
	{
		folder = compilerFolderName;
	}
	else if (!gameFolderName.empty())
	{
		folder = gameFolderName;
	}
	else
	{
		iLog->Log("ERROR: CShaderSrv::Init: Game folder has not been specified\n");
		return;
	}

	ICVar* pCompilerFolderSuffix = CRenderer::CV_r_ShaderCompilerFolderSuffix;
	
	folder.Trim();
	if (pCompilerFolderSuffix)
	{
		string suffix = pCompilerFolderSuffix->GetString();
		suffix.Trim();
		folder.append(suffix);
	}
	folder.append("/");

	m_RequestLineRootFolder = folder;
}

CShaderSrv& CShaderSrv::Instance()
{
	static CShaderSrv g_ShaderSrv;
	return g_ShaderSrv;
}

string CShaderSrv::CreateXMLNode(const string& rTag, const string& rValue) const
{
	string Tag = rTag;
	Tag += "=\"";
	Tag += rValue;
	Tag += "\" ";
	return Tag;
}

/*string CShaderSrv::CreateXMLDataNode(const string& rTag,const string& rValue)	const
   {
   string Tag="<";
   Tag+=rTag;
   Tag+="><![CDATA[";
   Tag+=rValue;
   Tag+="]]>";
   return Tag;
   }*/

string CShaderSrv::TransformToXML(const string& rIn)  const
{
	string Out;
	for (size_t a = 0, Size = rIn.size(); a < Size; a++)
	{
		const char C = rIn.c_str()[a];
		if (C == '&')
			Out += "&amp;";
		else if (C == '<')
			Out += "&lt;";
		else if (C == '>')
			Out += "&gt;";
		else if (C == '\"')
			Out += "&quot;";
		else if (C == '\'')
			Out += "&apos;";
		else
			Out += C;
	}
	return Out;
}

bool CShaderSrv::CreateRequest(std::vector<u8>& rVec,
                               std::vector<std::pair<string, string>>& rNodes) const
{
	string Request = "<?xml version=\"1.0\"?><Compile ";
	Request += CreateXMLNode("Version", TransformToXML("2.1"));
	for (size_t a = 0; a < rNodes.size(); a++)
		Request += CreateXMLNode(rNodes[a].first, TransformToXML(rNodes[a].second));

	Request += " />";
	rVec = std::vector<u8>(Request.c_str(), &Request.c_str()[Request.size() + 1]);
	return true;
}

tukk CShaderSrv::GetPlatform() const
{
	tukk szTarget = "unknown";
	if (CParserBin::m_nPlatform == SF_ORBIS)
		szTarget = "ORBIS";
	else if (CParserBin::m_nPlatform == SF_DURANGO)
		szTarget = "DURANGO";
	else if (CParserBin::m_nPlatform == SF_D3D11)
		szTarget = "D3D11";
	else if (CParserBin::m_nPlatform == SF_GL4)
		szTarget = "GL4";
	else if (CParserBin::m_nPlatform == SF_GLES3)
		szTarget = "GLES3";
	else if (CParserBin::m_nPlatform == SF_VULKAN)
		szTarget = "VULKAN";

	return szTarget;
}

bool CShaderSrv::RequestLine(const SCacheCombination& cmb, const string& rLine) const
{
	const string List(string(GetPlatform()) + "/" + cmb.Name.c_str() + "ShaderList.txt");
	return RequestLine(List, rLine);
}

bool CShaderSrv::CommitPLCombinations(std::vector<SCacheCombination>& rVec)
{
	u32k STEPSIZE = 32;
	float T0 = iTimer->GetAsyncCurTime();
	for (u32 i = 0; i < rVec.size(); i += STEPSIZE)
	{
		string Line;
		string levelRequest;

		levelRequest.Format("<%d>%s", rVec[i].nCount, rVec[i].CacheName.c_str());
		//printf("CommitPL[%d] '%s'\n", i, levelRequest.c_str());
		Line = levelRequest;
		for (u32 j = 1; j < STEPSIZE && i + j < rVec.size(); j++)
		{
			Line += string(";");
			levelRequest.Format("<%d>%s", rVec[i + j].nCount, rVec[i + j].CacheName.c_str());
			//printf("CommitPL[%d] '%s'\n", i+j, levelRequest.c_str());
			Line += levelRequest;
		}
		if (!RequestLine(rVec[i], Line))
			return false;
	}
	float T1 = iTimer->GetAsyncCurTime();
	iLog->Log("%3.3f to commit %" PRISIZE_T " Combinations\n", T1 - T0, rVec.size());

	return true;
}

EServerError CShaderSrv::Compile(std::vector<u8>& rVec,
                                 tukk pProfile,
                                 tukk pProgram,
                                 tukk pEntry,
                                 tukk pCompileFlags,
                                 tukk pIdent) const
{
	EServerError errCompile = ESOK;

	std::vector<u8> CompileData;
	std::vector<std::pair<string, string>> Nodes;
	Nodes.resize(9);
	Nodes[0] = std::pair<string, string>(string("JobType"), string("Compile"));
	Nodes[1] = std::pair<string, string>(string("Profile"), string(pProfile));
	Nodes[2] = std::pair<string, string>(string("Program"), string(pProgram));
	Nodes[3] = std::pair<string, string>(string("Entry"), string(pEntry));
	Nodes[4] = std::pair<string, string>(string("CompileFlags"), string(pCompileFlags));
	Nodes[5] = std::pair<string, string>(string("HashStop"), string("1"));
	Nodes[6] = std::pair<string, string>(string("ShaderRequest"), string(pIdent));
	Nodes[7] = std::pair<string, string>(string("Project"), string(m_RequestLineRootFolder.c_str()));
	Nodes[8] = std::pair<string, string>(string("Platform"), string(GetPlatform()));

	if (gRenDev->CV_r_ShaderEmailTags && gRenDev->CV_r_ShaderEmailTags->GetString() &&
	    strlen(gRenDev->CV_r_ShaderEmailTags->GetString()) > 0)
	{
		Nodes.resize(Nodes.size() + 1);
		Nodes[Nodes.size() - 1] = std::pair<string, string>(string("Tags"), string(gRenDev->CV_r_ShaderEmailTags->GetString()));
	}

	if (gRenDev->CV_r_ShaderEmailCCs && gRenDev->CV_r_ShaderEmailCCs->GetString() &&
	    strlen(gRenDev->CV_r_ShaderEmailCCs->GetString()) > 0)
	{
		Nodes.resize(Nodes.size() + 1);
		Nodes[Nodes.size() - 1] = std::pair<string, string>(string("EmailCCs"), string(gRenDev->CV_r_ShaderEmailCCs->GetString()));
	}

	if (gRenDev->CV_r_ShaderCompilerDontCache)
	{
		Nodes.resize(Nodes.size() + 1);
		Nodes[Nodes.size() - 1] = std::pair<string, string>(string("Caching"), string("0"));
	}
	//	Nodes[5]	=	std::pair<string,string>(string("ShaderRequest",string(pShaderRequestLine));
	i32 nRetries = 3;
	do
	{
		if (errCompile != ESOK)
			DrxSleep(5000);

		if (!CreateRequest(CompileData, Nodes))
		{
			iLog->LogError("ERROR: CShaderSrv::Compile: failed composing Request XML\n");
			return ESFailed;
		}

		errCompile = Send(CompileData);
	}
	while (errCompile == ESRecvFailed && nRetries-- > 0);

	rVec = CompileData;

	if (errCompile != ESOK)
	{
		bool logError = true;
		tuk why = "";
		switch (errCompile)
		{
		case ESNetworkError:
			why = "Network Error";
			break;
		case ESSendFailed:
			why = "Send Failed";
			break;
		case ESRecvFailed:
			why = "Receive Failed";
			break;
		case ESInvalidState:
			why = "Invalid Return State (compile issue ?!?)";
			break;
		case ESCompileError:
			logError = false;
			why = "";
			break;
		case ESFailed:
			why = "";
			break;
		}
		if (logError)
			iLog->LogError("ERROR: CShaderSrv::Compile: failed to compile %s (%s)", pEntry, why);
	}
	return errCompile;
}

bool CShaderSrv::RequestLine(const string& rList, const string& rString) const
{
	if (!gRenDev->CV_r_shaderssubmitrequestline)
		return true;

	string list = m_RequestLineRootFolder.c_str() + rList;

	std::vector<u8> CompileData;
	std::vector<std::pair<string, string>> Nodes;
	Nodes.resize(3);
	Nodes[0] = std::pair<string, string>(string("JobType"), string("RequestLine"));
	Nodes[1] = std::pair<string, string>(string("Platform"), list);
	Nodes[2] = std::pair<string, string>(string("ShaderRequest"), rString);
	if (!CreateRequest(CompileData, Nodes))
	{
		iLog->LogError("ERROR: CShaderSrv::RequestLine: failed composing Request XML\n");
		return false;
	}

	return (Send(CompileData) == ESOK);
}

bool CShaderSrv::Send(DRXSOCKET Socket, tukk pBuffer, u32 Size)  const
{
	//size_t w;
	size_t wTotal = 0;
	while (wTotal < Size)
	{
		i32 result = DrxSock::send(Socket, pBuffer + wTotal, Size - wTotal, 0);
		DrxSock::eDrxSockError sockErr = DrxSock::TranslateSocketError(result);
		if (sockErr != DrxSock::eCSE_NO_ERROR)
		{
			;
			iLog->Log("ERROR:CShaderSrv::Send failed (%d, %d)\n", (i32)result, DrxSock::TranslateToSocketError(sockErr));
			return false;
		}
		wTotal += (size_t)result;
	}
	return true;
}

bool CShaderSrv::Send(DRXSOCKET Socket, std::vector<u8>& rCompileData) const
{
	const uint64 Size = static_cast<u32>(rCompileData.size());
	return Send(Socket, (tukk)&Size, 8) && Send(Socket, (tukk)&rCompileData[0], static_cast<u32>(Size));
}

#define MAX_TIME_TO_WAIT 100000

EServerError CShaderSrv::Recv(DRXSOCKET Socket, std::vector<u8>& rCompileData)  const
{
	const size_t Offset = 5;  //version 2 has 4byte size and 1 byte state
	//	u32k Size	=	static_cast<u32>(rCompileData.size());
	//	return	Send(Socket,(tukk)&Size,4) ||
	//		Send(Socket,(tukk)&rCompileData[0],Size);

	//	delete[] optionsBuffer;
	u32 nMsgLength = 0;
	u32 nTotalRecived = 0;
	const size_t BLOCKSIZE = 4 * 1024;
	const size_t SIZELIMIT = 1024 * 1024;
	rCompileData.resize(0);
	rCompileData.reserve(64 * 1024);
	i32 CurrentPos = 0;
	while (rCompileData.size() < SIZELIMIT)
	{
		rCompileData.resize(CurrentPos + BLOCKSIZE);

		i32 Recived = DRX_SOCKET_ERROR;
		i32 waitingtime = 0;
		while (Recived < 0)
		{
			Recived = DrxSock::recv(Socket, reinterpret_cast<tuk>(&rCompileData[CurrentPos]), BLOCKSIZE, 0);
			DrxSock::eDrxSockError sockErr = DrxSock::TranslateSocketError(Recived);
			if (sockErr != DrxSock::eCSE_NO_ERROR)
			{
				if (sockErr == DrxSock::eCSE_EWOULDBLOCK)
				{
					// are we out of time
					if (waitingtime > MAX_TIME_TO_WAIT)
					{
						iLog->LogError("ERROR: CShaderSrv::Recv:  error in recv() from remote server. Out of time during waiting %d seconds on block, sys_net_errno=%i\n",
						               MAX_TIME_TO_WAIT, DrxSock::TranslateToSocketError(sockErr));
						return ESRecvFailed;
					}

					waitingtime += 5;

					// sleep a bit and try again
					DrxSleep(5);
				}
				else
				{
					// count on retry to fix this after a small sleep
					iLog->LogError("ERROR: CShaderSrv::Recv:  error in recv() from remote server at offset %lu: error %li, sys_net_errno=%i\n",
					               (u64)rCompileData.size(), (long)Recived, DrxSock::TranslateToSocketError(sockErr));
					return ESRecvFailed;
				}
			}
		}

		if (Recived >= 0)
			nTotalRecived += Recived;

		if (nTotalRecived >= 4)
			nMsgLength = *(u32*)&rCompileData[0] + Offset;

		if (Recived == 0 || nTotalRecived == nMsgLength)
		{
			rCompileData.resize(nTotalRecived);
			break;
		}
		CurrentPos += Recived;
	}
	if (rCompileData.size() == 0)
	{
		iLog->LogError("ERROR: CShaderSrv::Recv:  compile data empty from server (didn't receive anything - sys_net_errno=%i)\n", DrxSock::GetLastSocketError());
		return ESFailed;
	}
	if (rCompileData[4] != 1)  //1==ECSJS_DONE state on server, dont change!
	{
		u8 state = rCompileData[4];

		if (rCompileData.size() > Offset)
		{
			memmove(&rCompileData[0], &rCompileData[Offset], rCompileData.size() - Offset);
			rCompileData.resize(rCompileData.size() - Offset);
		}

		// Decompress incoming error data
		std::vector<u8> rCompressedErrorMessage;
		rCompressedErrorMessage.swap(rCompileData);

		u32 nSrcUncompressedLen = *(u32*)&rCompressedErrorMessage[0];
		SwapEndian(nSrcUncompressedLen);

		size_t nUncompressedLen = (size_t)nSrcUncompressedLen;

		if (nUncompressedLen < 1000000 && nUncompressedLen > 0)
		{
			std::vector<u8> rUncompressedErrorMessage;
			rUncompressedErrorMessage.resize(nUncompressedLen);
			if (gEnv->pSystem->DecompressDataBlock(&rCompressedErrorMessage[4],
			                                       rCompressedErrorMessage.size() - 4, &rUncompressedErrorMessage[0], nUncompressedLen))
			{
				rCompileData.swap(rUncompressedErrorMessage);
			}
		}

		// don't print compile errors here, they'll be handled later
		if (state == 5) // 5==ECSJS_COMPILE_ERROR state on server, dont change!
			return ESCompileError;

		iLog->LogError("ERROR: CShaderSrv::Recv:  compile data contains invalid return status: state = %u \n", state);

		return ESInvalidState;
	}

	//	iLog->Log("Recv = %d",(u64)rCompileData.size() );
	if (rCompileData.size() > Offset)
	{
		memmove(&rCompileData[0], &rCompileData[Offset], rCompileData.size() - Offset);
		rCompileData.resize(rCompileData.size() - Offset);
	}
	return rCompileData.size() >= Offset &&
	       rCompileData.size() < SIZELIMIT ? ESOK : ESFailed;
}

void CShaderSrv::Tokenize(tdEntryVec& rRet, const string& Tokens, const string& Separator)  const
{
	rRet.clear();
	string::size_type Pt;
	string::size_type Start = 0;
	string::size_type SSize = Separator.size();

	while ((Pt = Tokens.find(Separator, Start)) != string::npos)
	{
		string SubStr = Tokens.substr(Start, Pt - Start);
		rRet.push_back(SubStr);
		Start = Pt + SSize;
	}

	rRet.push_back(Tokens.substr(Start));
}

EServerError CShaderSrv::Send(std::vector<u8>& rCompileData) const
{
	DRXSOCKET Socket = DRX_SOCKET_ERROR;
	i32 Err = DRX_SOCKET_ERROR;

	tdEntryVec ServerVec;
	if (gRenDev->CV_r_ShaderCompilerServer)
		Tokenize(ServerVec, gRenDev->CV_r_ShaderCompilerServer->GetString(), ";");

	// Always add localhost as last resort
	ServerVec.push_back("localhost");

#if DRX_PLATFORM_WINDOWS
	i32 nPort = 0;
#endif

	//connect
	for (u32 nRetries = m_LastWorkingServer; nRetries < m_LastWorkingServer + ServerVec.size() + 6; nRetries++)
	{
		string Server = ServerVec[nRetries % ServerVec.size()];
		Socket = DrxSock::socket(AF_INET, SOCK_STREAM, 0);
		if (Socket == DRX_INVALID_SOCKET)
		{
			iLog->LogError("ERROR: CShaderSrv::Compile: can't create client socket: error %i\n", DrxSock::GetLastSocketError());
			return ESNetworkError;
		}

		i32 arg = 1;
		DrxSock::setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, (tuk)&arg, sizeof arg);

		DRXSOCKADDR_IN addr;
		memset(&addr, 0, sizeof addr);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(gRenDev->CV_r_ShaderCompilerPort);
		addr.sin_addr.s_addr = DrxSock::GetAddrForHostOrIP(Server.c_str(), 0);

		if (addr.sin_addr.s_addr == 0)
			continue;

		Err = DrxSock::connect(Socket, (DRXSOCKADDR*)&addr, sizeof addr);
		u16 nPort = 0;
		if (Err >= 0)
		{
			DRXSOCKADDR sockAddres;
			DRXSOCKLEN_T len = sizeof(sockAddres);
			if (DrxSock::getsockname(Socket, &sockAddres, &len) == DRX_SOCKET_ERROR)
			{
				iLog->LogError("ERROR: CShaderSrv::Compile: invalid socket after trying to connect: error %i, sys_net_errno=%i\n", Err, DrxSock::GetLastSocketError());
			}

			nPort = ntohs(((DRXSOCKADDR_IN*)&sockAddres)->sin_port);
			m_LastWorkingServer = nRetries % ServerVec.size();
			break;
		}

		if (Err < 0)
		{
			//iLog->LogError("ERROR: CShaderSrv::Compile: could not connect to %s\n", Server.c_str());
			DrxSock::eDrxSockError sockErr = DrxSock::TranslateLastSocketError();
			iLog->LogError("ERROR: CShaderSrv::Compile: could not connect to %s (error %i, sys_net_errno=%i, retrying %u, check r_ShaderCompilerServer and r_ShadersRemoteCompiler settings)\n", Server.c_str(), Err, DrxSock::TranslateToSocketError(sockErr), nRetries);

			// if buffer is full try sleeping a bit before retrying
			// (if you keep getting this issue then try using same shutdown mechanism as server is doing (see server code))
			// (for more info on windows side check : http://www.proxyplus.cz/faq/articles/EN/art10002.htm)
			if (sockErr == DrxSock::eCSE_ENOBUFS)
			{
				DrxSleep(5000);
			}

			DrxSock::closesocket(Socket);
			Socket = DRXSOCKET(DRX_INVALID_SOCKET);

			return ESNetworkError;
		}
	}

	if (Socket == DRX_INVALID_SOCKET)
	{
		rCompileData.resize(0);
		iLog->LogError("ERROR: CShaderSrv::Compile: invalid socket after trying to connect: error %i, sys_net_errno=%i\n", Err, DrxSock::GetLastSocketError());
		return ESNetworkError;
	}

#ifdef USE_WSAEVENTS
	WSAEVENT wsaEvent = WSACreateEvent();
	if (wsaEvent == WSA_INVALID_EVENT)
	{
		rCompileData.resize(0);
		iLog->LogError("ERROR: CShaderSrv::Send: couldn't create WSAEvent: sys_net_errno=%i\n", WSAGetLastError());
		DrxSock::closesocket(Socket);
		return ESNetworkError;
	}
	;

	i32 Status = WSAEventSelect(Socket, wsaEvent, FD_READ);
	if (Status == DRX_SOCKET_ERROR)
	{
		rCompileData.resize(0);
		iLog->LogError("ERROR: CShaderSrv::Send: couldn't select event to listen to: sys_net_errno=%i\n", WSAGetLastError());
		DrxSock::closesocket(Socket);
		return ESNetworkError;
	}
#endif

	if (!Send(Socket, rCompileData))
	{
		rCompileData.resize(0);
		DrxSock::closesocket(Socket);
		return ESSendFailed;
	}

	EServerError Error = Recv(Socket, rCompileData);
	/*
	   // inform server that we are done receiving data
	   const uint64 Result	=	1;
	   Send(Socket,(tukk)&Result,8);
	 */

	// shutdown the client side of the socket because we are done listening
	Err = DrxSock::shutdown(Socket, 0x02); //SD_BOTH);
	if (Err == DRX_SOCKET_ERROR)
	{
#if DRX_PLATFORM_APPLE
		// Mac OS X does not forgive calling shutdown on a closed socket, linux and
		// windows don't mind
		if (DrxSock::TranslateLastSocketError() != DrxSock::eCSE_ENOTCONN)
		{
#endif
		iLog->LogError("ERROR: CShaderSrv::Compile: error shutting down socket: sys_net_errno=%i\n", DrxSock::GetLastSocketError());
		DrxSock::closesocket(Socket);
		return ESNetworkError;
#if DRX_PLATFORM_APPLE
	}
#endif
	}

#ifdef USE_WSAEVENTS
	// wait until client has shutdown its socket
	DWORD nReturnCode = WSAWaitForMultipleEvents(1, &wsaEvent,
	                                             FALSE, INFINITE, FALSE);
	if ((nReturnCode != WSA_WAIT_FAILED) && (nReturnCode != WSA_WAIT_TIMEOUT))
	{
		WSANETWORKEVENTS NetworkEvents;
		WSAEnumNetworkEvents(Socket, wsaEvent, &NetworkEvents);
		if (NetworkEvents.lNetworkEvents & FD_CLOSE)
		{
			i32 iErrorCode = NetworkEvents.iErrorCode[FD_CLOSE_BIT];
			if (iErrorCode != 0)
			{
				iLog->LogError("ERROR: CShaderSrv::Compile: error while receiving FD_Close event from server: error code: %i sys_net_errno=%i\n",
				               NetworkEvents.iErrorCode[FD_CLOSE_BIT], WSAGetLastError());
			}
		}
	}
	else
	{
		iLog->LogError("ERROR: CShaderSrv::Compile: couldn't receive FD_Close event from server: return code: %i sys_net_errno=%i\n",
		               nReturnCode, WSAGetLastError());
	}
#endif

	DrxSock::closesocket(Socket);
	if (Error != ESOK)
		return Error;

	if (rCompileData.size() < 4)
		return ESFailed;

	// Decompress incoming shader data
	std::vector<u8> rCompressedData;
	rCompressedData.swap(rCompileData);

	u32 nSrcUncompressedLen = *(u32*)&rCompressedData[0];
	SwapEndian(nSrcUncompressedLen);

	size_t nUncompressedLen = (size_t)nSrcUncompressedLen;

	rCompileData.resize(nUncompressedLen);
	if (nUncompressedLen > 1000000)
	{
		// Shader too big, something is wrong.
		return ESFailed;
	}
	if (nUncompressedLen > 0)
	{
		if (!gEnv->pSystem->DecompressDataBlock(&rCompressedData[4], rCompressedData.size() - 4, &rCompileData[0], nUncompressedLen))
			return ESFailed;
	}

	//if (rCompileData.size() == 0 || strncmp( (tuk)&rCompileData[0],"[ERROR]",MIN(rCompileData.size(),7)) == 0)
	//	return ESFailed;

	return ESOK;
}

}
