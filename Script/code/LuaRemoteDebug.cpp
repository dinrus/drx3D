// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Script/StdAfx.h>
#include <drx3D/Script/LuaRemoteDebug.h>

#if LUA_REMOTE_DEBUG_ENABLED

CLuaRemoteDebug::CLuaRemoteDebug(CScriptSystem* pScriptSystem)
	: m_sendBuffer(10 * 1024)
	, m_nCallLevelUntilBreak(0)
	, m_pScriptSystem(pScriptSystem)
	, m_bForceBreak(false)
	, m_bExecutionStopped(false)
	, m_breakState(bsNoBreak)
	, m_pHaltedLuaDebug(NULL)
	, m_bBinaryLuaDetected(false)
	, m_clientVersion(0)
	, m_enableCppCallstack(false)
{
	if (INotificationNetwork* pNotificationNetwork =
	      gEnv->pSystem->GetINotificationNetwork())
	{
		pNotificationNetwork->ListenerBind(LUA_REMOTE_DEBUG_CHANNEL, this);
	}
}

void CLuaRemoteDebug::OnDebugHook(lua_State* L, lua_Debug* ar)
{
	switch (ar->event)
	{
	// function call
	case LUA_HOOKCALL:
		m_nCallLevelUntilBreak++;

		// Fetch a description of the call and add it to the exposed call stack
		{
			lua_getinfo(L, "lnS", ar);
			tukk name = ar->name ? ar->name : "-";
			tukk source = ar->source ? ar->source : "-";
			m_pScriptSystem->ExposedCallstackPush(name, ar->currentline, source);
		}
		break;

	// return from function
	case LUA_HOOKRET:
	//case LUA_HOOKTAILRET:
		if (m_breakState == bsStepOut && m_nCallLevelUntilBreak <= 0)
			BreakOnNextLuaCall();
		else
			m_nCallLevelUntilBreak--;

		// Remove the last call from the exposed call stack
		m_pScriptSystem->ExposedCallstackPop();
		break;

	// on each line
	case LUA_HOOKLINE:
		lua_getinfo(L, "S", ar);

		if (!m_bBinaryLuaDetected && ar->currentline == 0 && ar->source[0] == '=' && ar->source[1] == '?' && ar->source[2] == 0)
		{
			// This is a binary file
			m_bBinaryLuaDetected = true;
			SendBinaryFileDetected();
		}

		if (m_bForceBreak)
		{
			m_nCallLevelUntilBreak = 0;
			m_breakState = bsNoBreak;
			m_bForceBreak = false;
			HaltExecutionLoop(L, ar);
			return;
		}

		switch (m_breakState)
		{
		case bsStepInto:
			HaltExecutionLoop(L, ar);
			break;
		case bsStepNext:
			if (m_nCallLevelUntilBreak <= 0)
			{
				HaltExecutionLoop(L, ar);
			}
			break;
		default:
			if (HaveBreakPointAt(ar->source, ar->currentline))
			{
				HaltExecutionLoop(L, ar);
			}
		}
	}
}

void CLuaRemoteDebug::OnScriptError(lua_State* L, lua_Debug* ar, tukk errorStr)
{
	HaltExecutionLoop(L, ar, errorStr);
}

void CLuaRemoteDebug::SendBinaryFileDetected()
{
	// Inform the debugger that debugging may not work due to binary lua files
	m_sendBuffer.Write((char)ePT_BinaryFileDetected);
	SendBuffer();
}

void CLuaRemoteDebug::SendGameFolder()
{
	// Find the root folder from which the game was compiled in
	bool success = true;
	string gameFolder = __FILE__;
	for (i32 i = 0; i < 5; ++i) // Go up 5 folder to reach the root path
	{
		size_t pos = gameFolder.find_last_of("\\/");
		if (pos != string::npos)
		{
			gameFolder.resize(pos);
		}
		else
		{
			DRX_ASSERT_MESSAGE(false, "Error trying to compute root folder");
			success = false;
			break;
		}
	}
	if (success)
	{
		gameFolder.append("\\");
		gameFolder += PathUtil::GetGameFolder();
		m_sendBuffer.Write((char)ePT_GameFolder);
		m_sendBuffer.WriteString(gameFolder.c_str());
		SendBuffer();
	}
}

void CLuaRemoteDebug::SendScriptError(tukk errorStr)
{
	if (m_clientVersion >= 5)
	{
		m_sendBuffer.Write((char)ePT_ScriptError);
		m_sendBuffer.WriteString(errorStr);
		SendBuffer();
	}
}

bool CLuaRemoteDebug::HaveBreakPointAt(tukk sSourceFile, i32 nLine)
{
	i32 nCount = m_breakPoints.size();
	for (i32 i = 0; i < nCount; i++)
	{
		if (m_breakPoints[i].nLine == nLine && strcmp(m_breakPoints[i].sSourceFile, sSourceFile) == 0)
		{
			return true;
		}
	}
	return false;
}

void CLuaRemoteDebug::AddBreakPoint(tukk sSourceFile, i32 nLine)
{
	LuaBreakPoint bp;
	bp.nLine = nLine;
	bp.sSourceFile = sSourceFile;
	stl::push_back_unique(m_breakPoints, bp);
}

void CLuaRemoteDebug::RemoveBreakPoint(tukk sSourceFile, i32 nLine)
{
	i32 nCount = m_breakPoints.size();
	for (i32 i = 0; i < nCount; i++)
	{
		if (m_breakPoints[i].nLine == nLine && strcmp(m_breakPoints[i].sSourceFile, sSourceFile) == 0)
		{
			m_breakPoints.erase(m_breakPoints.begin() + i);
			return;
		}
	}
}

void CLuaRemoteDebug::HaltExecutionLoop(lua_State* L, lua_Debug* ar, tukk errorStr)
{
	m_pHaltedLuaDebug = ar;

	SendLuaState(ar);
	SendVariables();
	if (errorStr)
	{
		SendScriptError(errorStr);
	}

	m_bExecutionStopped = true;
	if (INotificationNetwork* pNotificationNetwork = gEnv->pSystem->GetINotificationNetwork())
	{
		while (m_bExecutionStopped)
		{
			pNotificationNetwork->Update();

			DrxSleep(50);
		}
	}

	m_pHaltedLuaDebug = NULL;
}

void CLuaRemoteDebug::BreakOnNextLuaCall()
{
	m_bForceBreak = true;
	m_nCallLevelUntilBreak = 0;
	m_breakState = bsNoBreak;
}

void CLuaRemoteDebug::Continue()
{
	m_bExecutionStopped = false;
	m_breakState = bsContinue;
}

void CLuaRemoteDebug::StepOver()
{
	m_bExecutionStopped = false;
	m_nCallLevelUntilBreak = 0;
	m_breakState = bsStepNext;
}

void CLuaRemoteDebug::StepInto()
{
	m_bExecutionStopped = false;
	m_breakState = bsStepInto;
}

void CLuaRemoteDebug::StepOut()
{
	m_bExecutionStopped = false;
	m_breakState = bsStepOut;
}

void CLuaRemoteDebug::OnNotificationNetworkReceive(ukk pBuffer, size_t length)
{
	if (!pBuffer || length == 0)
		return;
	CSerializationHelper bufferUtil((tuk)pBuffer, length);
	char packetType;
	bufferUtil.Read(packetType);
	switch (packetType)
	{
	case ePT_Version:
		ReceiveVersion(bufferUtil);
		break;
	case ePT_Break:
		BreakOnNextLuaCall();
		break;
	case ePT_Continue:
		Continue();
		break;
	case ePT_StepOver:
		StepOver();
		break;
	case ePT_StepInto:
		StepInto();
		break;
	case ePT_StepOut:
		StepOut();
		break;
	case ePT_SetBreakpoints:
		ReceiveSetBreakpoints(bufferUtil);
		break;
	case ePT_FileMD5:
		ReceiveFileMD5Request(bufferUtil);
		break;
	case ePT_FileContents:
		ReceiveFileContentsRequest(bufferUtil);
		break;
	case ePT_EnableCppCallstack:
		ReceiveEnableCppCallstack(bufferUtil);
		break;
	default:
		DRX_ASSERT_MESSAGE(false, "Unrecognised packet type");
		break;
	}
}

void CLuaRemoteDebug::ReceiveVersion(CSerializationHelper& buffer)
{
	buffer.Read(m_clientVersion);
	if (m_clientVersion != LUA_REMOTE_DEBUG_HOST_VERSION)
	{
		DrxLogAlways("Warning: Lua remote debug client connected with version %d, host is version %d", m_clientVersion, LUA_REMOTE_DEBUG_HOST_VERSION);
	}
	else
	{
		DrxLog("Lua remote debug client connected with version: %d, host is version: %d", m_clientVersion, LUA_REMOTE_DEBUG_HOST_VERSION);
	}
	SendVersion();
	SendGameFolder();
	// Make sure the debugger is enabled when the remote debugger connects
	ICVar* pCvar = gEnv->pConsole->GetCVar("lua_debugger");
	if (pCvar)
	{
		pCvar->Set(1);
	}
	// Send Lua state to newly connected debugger
	if (m_bExecutionStopped && m_pHaltedLuaDebug)
	{
		SendLuaState(m_pHaltedLuaDebug);
		SendVariables();
	}
	if (m_bBinaryLuaDetected)
	{
		SendBinaryFileDetected();
	}
}

void CLuaRemoteDebug::ReceiveSetBreakpoints(CSerializationHelper& buffer)
{
	m_breakPoints.clear();
	i32 numBreakpoints;
	buffer.Read(numBreakpoints);
	for (i32 i = 0; i < numBreakpoints; ++i)
	{
		tukk fileName;
		i32 line;
		fileName = buffer.ReadString();
		buffer.Read(line);
		AddBreakPoint(fileName, line);
	}
}

void CLuaRemoteDebug::ReceiveFileMD5Request(CSerializationHelper& buffer)
{
	// Compute the file MD5
	tukk fileName = buffer.ReadString();
	u8 md5[16];
	if (gEnv->pDrxPak->ComputeMD5(fileName + 1, md5))
	{
		// And send back the response
		m_sendBuffer.Write((char)ePT_FileMD5);
		m_sendBuffer.WriteString(fileName);
		for (i32 i = 0; i < 16; ++i)
		{
			m_sendBuffer.Write(md5[i]);
		}
		SendBuffer();
	}
	else
	{
		assert(false);
	}
}

void CLuaRemoteDebug::ReceiveFileContentsRequest(CSerializationHelper& buffer)
{
	tukk fileName = buffer.ReadString();
	IDrxPak* pDrxPak = gEnv->pDrxPak;
	FILE* pFile = pDrxPak->FOpen(fileName + 1, "rb");
	if (pFile != NULL)
	{
		m_sendBuffer.Write((char)ePT_FileContents);
		m_sendBuffer.WriteString(fileName);

		// Get file length
		pDrxPak->FSeek(pFile, 0, SEEK_END);
		u32 length = (u32)pDrxPak->FTell(pFile);
		pDrxPak->FSeek(pFile, 0, SEEK_SET);

		m_sendBuffer.Write(length);

		i32k CHUNK_BUF_SIZE = 1024;
		char buf[CHUNK_BUF_SIZE];
		size_t lenRead;

		while (!pDrxPak->FEof(pFile))
		{
			lenRead = pDrxPak->FRead(buf, CHUNK_BUF_SIZE, pFile);
			m_sendBuffer.WriteBuffer(buf, (i32)lenRead);
		}

		SendBuffer();
	}
	else
	{
		assert(false);
	}
}

void CLuaRemoteDebug::ReceiveEnableCppCallstack(CSerializationHelper& buffer)
{
	buffer.Read(m_enableCppCallstack);
	if (m_bExecutionStopped && m_pHaltedLuaDebug)
	{
		// Re-send the current Lua state (including or excluding the cpp callstack)
		SendLuaState(m_pHaltedLuaDebug);
	}
}

void CLuaRemoteDebug::SendVersion()
{
	m_sendBuffer.Write((char)ePT_Version);
	m_sendBuffer.Write((i32)LUA_REMOTE_DEBUG_HOST_VERSION);
	if (m_clientVersion >= 4)
	{
	#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
		m_sendBuffer.Write((char)eP_Win64);
	#elif DRX_PLATFORM_WINDOWS && DRX_PLATFORM_32BIT
		m_sendBuffer.Write((char)eP_Win32);
	#else
		m_sendBuffer.Write((char)eP_Unknown);
	#endif
	}
	SendBuffer();
}

void CLuaRemoteDebug::SerializeLuaValue(const ScriptAnyValue& scriptValue, CSerializationHelper& buffer, i32 maxDepth)
{
	buffer.Write((char)scriptValue.GetType());
	switch (scriptValue.GetType())
	{
	case EScriptAnyType::Any:
	case EScriptAnyType::Nil:
		// Nothing to serialize
		break;
	case EScriptAnyType::Boolean:
		buffer.Write(scriptValue.GetBool());
		break;
	case EScriptAnyType::Handle:
		buffer.Write(scriptValue.GetScriptHandle().ptr);
		break;
	case EScriptAnyType::Number:
		buffer.Write(scriptValue.GetNumber());
		break;
	case EScriptAnyType::String:
		buffer.WriteString(scriptValue.GetString());
		break;
	case EScriptAnyType::Table:
		SerializeLuaTable(scriptValue.GetScriptTable(), buffer, maxDepth - 1);
		break;
	case EScriptAnyType::Function:
		buffer.Write(scriptValue.GetScriptFunction());
		break;
	case EScriptAnyType::UserData:
		buffer.Write(scriptValue.GetUserData().ptr);
		buffer.Write(scriptValue.GetUserData().nRef);
		break;
	case EScriptAnyType::Vector:
		buffer.Write(scriptValue.GetVector().x);
		buffer.Write(scriptValue.GetVector().y);
		buffer.Write(scriptValue.GetVector().z);
		break;
	}
}

void CLuaRemoteDebug::SerializeLuaTable(IScriptTable* pTable, CSerializationHelper& buffer, i32 maxDepth)
{
	if (maxDepth <= 0)
	{
		// If we reached the max table depth then serialize out an empty table
		buffer.Write((u16)0);
		return;
	}

	i32 numEntries = 0;
	const bool resolvePrototypeTableAsWell = true;

	// First count the number of entries (pTable->Count() seems unreliable)
	IScriptTable::Iterator iter = pTable->BeginIteration(resolvePrototypeTableAsWell);
	while (pTable->MoveNext(iter))
	{
		++numEntries;
	}
	pTable->EndIteration(iter);

	if (numEntries <= 0xFFFF)
	{
		buffer.Write((u16)numEntries);

		// Then serialize them
		iter = pTable->BeginIteration(resolvePrototypeTableAsWell);
		while (pTable->MoveNext(iter))
		{
			SerializeLuaValue(iter.key, buffer, maxDepth);
			SerializeLuaValue(iter.value, buffer, maxDepth);
		}
		pTable->EndIteration(iter);
	}
	else
	{
		buffer.Write((u16)0);
		DRX_ASSERT_MESSAGE(false, "Table count is too great to fit in 2 bytes");
	}
}

void CLuaRemoteDebug::GetLuaCallStack(std::vector<SLuaStackEntry>& callstack)
{
	lua_State* L = m_pScriptSystem->GetLuaState();
	callstack.clear();

	i32 level = 0;
	lua_Debug ar;
	string description;
	while (lua_getstack(L, level++, &ar))
	{
		description.clear();

		lua_getinfo(L, ("Sl"), &ar);
		if (ar.source && *ar.source && _stricmp(ar.source, "=C") != 0 && ar.linedefined != 0)
		{
			description = GetLineFromFile(ar.source + 1, ar.linedefined);
		}

		SLuaStackEntry se;
		se.description = description;
		se.line = ar.currentline;
		se.source = ar.source;

		callstack.push_back(se);
	}
}

void CLuaRemoteDebug::SendLuaState(lua_Debug* ar)
{
	m_sendBuffer.Write((char)ePT_ExecutionLocation);

	// Current execution location
	m_sendBuffer.WriteString(ar->source);
	m_sendBuffer.Write(ar->currentline);

	// Call stack
	std::vector<SLuaStackEntry> callstack;
	GetLuaCallStack(callstack);
	m_sendBuffer.Write((i32)callstack.size());

	for (size_t i = 0; i < callstack.size(); ++i)
	{
		SLuaStackEntry& le = callstack[i];

		m_sendBuffer.WriteString(le.source.c_str());
		m_sendBuffer.Write(le.line);
		m_sendBuffer.WriteString(le.description.c_str());
		if (m_clientVersion < 6)
		{
			m_sendBuffer.WriteString("");
		}
	}

	if (m_clientVersion >= 3)
	{
		if (m_enableCppCallstack)
		{
			// C++ Call stack
			tukk funcs[80];
			i32 nFuncCount = 80;
			GetISystem()->debug_GetCallStack(funcs, nFuncCount);
			m_sendBuffer.Write(nFuncCount);
			for (i32 i = 0; i < nFuncCount; ++i)
			{
				m_sendBuffer.WriteString(funcs[i]);
			}
		}
		else
		{
			// Send an empty callstack
			m_sendBuffer.Write(0);
		}
	}

	SendBuffer();
}

void CLuaRemoteDebug::SendVariables()
{
	m_sendBuffer.Write((char)ePT_LuaVariables);

	m_sendBuffer.Write((u8)sizeof(uk )); // Serialise out the size of pointers to cope with 32 and 64 bit systems

	// Local variables
	IScriptTable* pLocalVariables = m_pScriptSystem->GetLocalVariables(0, true);
	if (pLocalVariables)
	{
		SerializeLuaTable(pLocalVariables, m_sendBuffer, 8);
		pLocalVariables->Release();
		pLocalVariables = NULL;
	}

	SendBuffer();
}

string CLuaRemoteDebug::GetLineFromFile(tukk sFilename, i32 nLine)
{
	CDrxFile file;

	if (!file.Open(sFilename, "rb"))
	{
		return "";
	}
	i32 nLen = file.GetLength();
	tuk sScript = new char[nLen + 1];
	tuk sString = new char[nLen + 1];
	file.ReadRaw(sScript, nLen);
	sScript[nLen] = '\0';

	i32 nCurLine = 1;
	string strLine;

	strcpy(sString, "");

	tukk strLast = sScript + nLen;

	tukk str = sScript;
	while (str < strLast)
	{
		tuk s = sString;
		while (str < strLast && *str != '\n' && *str != '\r')
		{
			*s++ = *str++;
		}
		*s = '\0';
		if (str + 2 <= strLast && str[0] == '\r' && str[1] == '\n')
		{
			// Skip \r\n (Windows style line endings)
			str += 2;
		}
		else
		{
			// Skip \n or \r (Unix/Mac style line endings)
			str += 1;
		}

		if (nCurLine == nLine)
		{
			strLine = sString;
			strLine.replace('\t', ' ');
			strLine.Trim();
			break;
		}
		nCurLine++;
	}

	delete[]sString;
	delete[]sScript;

	return strLine;
}

void CLuaRemoteDebug::SendLogMessage(tukk format, ...)
{
	char message[1024];
	va_list args;
	va_start(args, format);
	drx_vsprintf(message, format, args);
	va_end(args);

	m_sendBuffer.Write((char)ePT_LogMessage);
	m_sendBuffer.WriteString(message);
	SendBuffer();
}

void CLuaRemoteDebug::SendBuffer()
{
	if (INotificationNetwork* pNotificationNetwork =
	      gEnv->pSystem->GetINotificationNetwork())
	{
		if (!m_sendBuffer.Overflow())
		{
			pNotificationNetwork->Send(LUA_REMOTE_DEBUG_CHANNEL, m_sendBuffer.GetBuffer(), m_sendBuffer.GetUsedSize());
		}
		m_sendBuffer.Clear();
	}
}

#endif //LUA_REMOTE_DEBUG_ENABLED
