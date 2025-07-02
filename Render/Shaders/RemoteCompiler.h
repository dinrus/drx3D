// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   RemoteCompiler.h : socket wrapper for shader compile server connections

   Revision история:
* Created by Michael Kopietz

   =============================================================================*/

#ifndef REMOTECOMPILER_H
#define REMOTECOMPILER_H

#include <drx3D/Network/DrxSocks.h>

namespace NRemoteCompiler
{
typedef std::vector<string> tdEntryVec;

enum EServerError
{
	ESOK,
	ESFailed,
	ESInvalidState,
	ESCompileError,
	ESNetworkError,
	ESSendFailed,
	ESRecvFailed,
};

class CShaderSrv
{
protected:
	static u32 m_LastWorkingServer;

	// root path added to each requestline to store the data per game (eg. MyGame\)
	string m_RequestLineRootFolder;

	CShaderSrv();

	bool         Send(DRXSOCKET Socket, tukk pBuffer, u32 Size) const;
	bool         Send(DRXSOCKET Socket, std::vector<u8>& rCompileData) const;
	EServerError Recv(DRXSOCKET Socket, std::vector<u8>& rCompileData) const;
	EServerError Send(std::vector<u8>& rCompileData) const;

	void         Tokenize(tdEntryVec& rRet, const string& Tokens, const string& Separator) const;
	string       TransformToXML(const string& rIn) const;
	string       CreateXMLNode(const string& rTag, const string& rValue)  const;
	//	string							CreateXMLDataNode(const string& rTag,const string& rValue)	const;

	bool RequestLine(const SCacheCombination& cmb,
	                 const string& rLine) const;
	bool CreateRequest(std::vector<u8>& rVec,
	                   std::vector<std::pair<string, string>>& rNodes) const;

	void         Init();
public:
	EServerError Compile(std::vector<u8>& rVec,
	                     tukk pProfile,
	                     tukk pProgram,
	                     tukk pEntry,
	                     tukk pCompileFlags,
	                     tukk pIdent) const;
	bool               CommitPLCombinations(std::vector<SCacheCombination>& rVec);
	bool               RequestLine(const string& rList,
	                               const string& rString)  const;
	tukk        GetPlatform() const;

	static CShaderSrv& Instance();
};
}

#endif
