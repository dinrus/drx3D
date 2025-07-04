// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
	-------------------------------------------------------------------------
	$Id$
	$DateTime$
	Описание: Logs data to a centralised server for later analysis
				 various game subsystems, forwarding necessary events to stats
				 recording system.

	-------------------------------------------------------------------------
	История:
	- 18:11:2009  : Created by Mark Tully

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/TelemetryCollector.h>
#include <drx3D/Game/Game.h>

#include <drx3D/CoreX/Lobby/IDrxTCPService.h>
#include <drx3D/Act/ILevelSystem.h>
#include <drx3D/Act/IPlayerProfiles.h>
#include <drx3D/Game/Network/Lobby/GameLobbyData.h>
#include <drx3D/Game/Network/GameNetworkUtils.h>
#include <drx3D/Game/Utility/DrxDebugLog.h>
#include <drx3D/Game/PlaylistUpr.h>
#include <drx3D/Game/Utility/StringUtils.h>
#include <drx3D/Sys/ZLib/IZLibCompressor.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/Sys/IStatoscope.h>
#include <drx3D/Game/DataPatchDownloader.h>
#include <drx3D/Game/GameRules.h>

#if !defined (_RELEASE)
#define TELEMETRY_CHECKS_FOR_OLD_ERRORLOGS (1)
#define TELEMETRY_OUTPUTS_HINT_FILE (1)
#endif

// these two must be the same. they are used to reserve space in the http header for the payload size which is then replaced after the
// payload is ready
static i32k		k_magicSizeInt=1234567890;
static const char		*k_magicSizeStr="1234567890";

enum ETelemetryTransactionRecording			// if you change these values, update the cvar help string
{
	k_recording_never=0,
	k_recording_ifServiceUnavailable=1,
	k_recording_always=2
};

enum
{
	TLOG_CRITICAL,
	TLOG_STANDARD,
	TLOG_VERBOSE
};

#define k_salt																		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"			// SECRET is prefixed to data being md5-ed to stop people tampering and then re-md5ing the data. must match salt on server.

#define k_telemetry_submitLogCommand	"telemetry_submitlog"
#define k_telemetry_getSessionIdCommand	"telemetry_getsessionid"
#define k_defaultTelemetryEnabled		1
#define k_defaultTelemetryLogging		0
#define k_defaultTelemetryUploadErrorLogs 1
#define k_defaultTelemetryUploadGameLogs 2 
#define k_defaultTelemetryCompressGameLogs 1
#define k_hintFileName "%USER%/MiscTelemetry/telemetry_server_hint.txt"
#define k_httpTimeOut				5.0f																								// stops tasks clogging up crynetwork if we're waiting for replies that never come

#define k_defaultTelemetryTransactionRecording	k_recording_never

#define STRINGIZE(x)		#x

CTelemetryCollector* CTelemetryCollector::s_telemetryCollector=NULL;

class CTelemetryMD5 : public ITelemetryProducer
{
	protected:
		ITelemetryProducer				*m_pSource;
		SMD5Context								m_context;
		enum EState
		{
			k_producingData,
			k_writingHash,
			k_finished
		};
		EState										m_state;
		i32												m_bytesMD5ed;

	public:
															CTelemetryMD5(
																const char					*pInSalt,
																ITelemetryProducer	*pInSource) :
																m_pSource(pInSource),
																m_state(k_producingData),
																m_bytesMD5ed(0)
															{
																IZLibCompressor			*pZLib=GetISystem()->GetIZLibCompressor();
																
																pZLib->MD5Init(&m_context);
																if (pInSalt)
																{
																	i32		len=strlen(pInSalt);
																	pZLib->MD5Update(&m_context,pInSalt,len);
																	m_bytesMD5ed+=len;
																}
															}

		virtual										~CTelemetryMD5()
															{
																delete m_pSource;
															}

		virtual EResult						ProduceTelemetry(
																char				*pOutBuffer,
																i32					inMinRequired,
																i32					inBufferSize,
																i32					*pOutWritten)
															{
																EResult			result=eTS_EndOfStream;

																// could code this so it could write <20 bytes of the hash and finish the rest on a subsequent call, but it shouldn't be neccessary as the callers don't generally work
																// with buffers so small
																DRX_ASSERT_MESSAGE(inBufferSize>=20,"MT : insuffient space for md5 hash to be written");

																switch (m_state)
																{
																	case k_producingData:
																		{
																			result=m_pSource->ProduceTelemetry(pOutBuffer,inMinRequired,inBufferSize,pOutWritten);

																			switch (result)
																			{
																				case eTS_EndOfStream:
																					{
																						// finished
																						m_state=k_writingHash;
																						result=eTS_Available;
																					}
																					// intentional fallthrough...
																				case eTS_Available:
																					if (*pOutWritten>0)
																					{
																						GetISystem()->GetIZLibCompressor()->MD5Update(&m_context,pOutBuffer,*pOutWritten);
																						m_bytesMD5ed+=*pOutWritten;
																					}
																					break;
																			}
																		}
																		break;

																	case k_writingHash:
																		if (inBufferSize>=20)
																		{
																			GetISystem()->GetIZLibCompressor()->MD5Final(&m_context,pOutBuffer);
																			// end of buffer magic, allows the server to do a quick truncation check before doing the md5
																			pOutBuffer[16]='M';
																			pOutBuffer[17]='5';
																			pOutBuffer[18]='E';
																			pOutBuffer[19]='D';
																			*pOutWritten=20;
																			result=eTS_EndOfStream;
																			m_state=k_finished;

/*
																			DrxLog("Uploaded data md5 %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x for %d bytes",
																				((u8*)pOutBuffer)[0],
																				((u8*)pOutBuffer)[1],
																				((u8*)pOutBuffer)[2],
																				((u8*)pOutBuffer)[3],
																				((u8*)pOutBuffer)[4],
																				((u8*)pOutBuffer)[5],
																				((u8*)pOutBuffer)[6],
																				((u8*)pOutBuffer)[7],
																				((u8*)pOutBuffer)[8],
																				((u8*)pOutBuffer)[9],
																				((u8*)pOutBuffer)[10],
																				((u8*)pOutBuffer)[11],
																				((u8*)pOutBuffer)[12],
																				((u8*)pOutBuffer)[13],
																				((u8*)pOutBuffer)[14],
																				((u8*)pOutBuffer)[15],
																				m_bytesMD5ed);
*/
																		}
																		break;

																	case k_finished:
																		break;

																	default:
																		assert(0);
																		break;
																}

																return result;
															}
};

class CTelemetryHTTPPostChunkSplitter : public ITelemetryProducer
{
	protected:
		ITelemetryProducer									*m_producer;
		i32																	m_contentLength;
		// TODO add crc of sent data to prevent tampering
		enum EState
		{
			k_writeChunks,
			k_writeFooter,
			k_writeTerminatingChunkAndFooter,
			k_done
		};
		EState															m_state;

	public:

																				CTelemetryHTTPPostChunkSplitter(
																					ITelemetryProducer			*inProducer);
		virtual															~CTelemetryHTTPPostChunkSplitter();

		virtual EResult											ProduceTelemetry(
																					char				*pOutBuffer,
																					i32					inMinRequired,
																					i32					inBufferSize,
																					i32					*pOutWritten);

};

void CTelemetryCollector::Log(i32 level, tukk format, ...)
{
	va_list args;
	va_start(args, format);

	if (level == TLOG_CRITICAL)
	{
		gEnv->pLog->LogV( ILog::eAlways,format,args );
	}
	else if (level <= m_telemetryServerLogging->GetIVal())
	{
		gEnv->pLog->LogV( ILog::eMessage,format,args );
	}

	va_end(args);
}

CTelemetryCollector::CTelemetryCollector() :
	m_pTelemetry(NULL),
	m_transfersCounter(0)
{
	assert(s_telemetryCollector==NULL);
	s_telemetryCollector=this;

	DRX_ASSERT_MESSAGE(k_largeFileSubmitChunkMaxDataSize > 0, string().Format("CTelemetryCollector() k_largeFileSubmitChunkSize=%d has been set smaller than k_maxHttpHeaderSize=%d", k_largeFileSubmitChunkSize, k_maxHttpHeaderSize));

	m_telemetryCompressionLevel=REGISTER_INT("g_telemetry_compression_level",2,0,"zlib deflateInit2 level value");
	m_telemetryCompressionWindowBits=REGISTER_INT("g_telemetry_compression_window_bits",24,0,"zlib deflateInit2 window bits");
	m_telemetryCompressionMemLevel=REGISTER_INT("g_telemetry_compression_mem_level",3,0,"zlib deflateInit2 mem level");

	m_telemetryTransactionRecordings=REGISTER_INT("g_telemetry_transaction_recording",k_defaultTelemetryTransactionRecording,0,
		"Usage: g_telemetry_transaction_recording <2/1/0 - record condition>\n"
		"Telemetry can be logged to a file in a format that can be uploaded to the server later\n"
		"0 - never log, 1 - only log if server is unreachable and telemetry is enabled, 2 - always log\n"
		"Default is " STRINGIZE(k_defaultTelemetryTransactionRecording));
	m_telemetryEnabled=REGISTER_INT("g_telemetry_enabled",k_defaultTelemetryEnabled,0,
		"Usage: g_telemetry_enabled <1/0 - upload telemetry>\n"
		"When set telemetry will be uploaded to the server configured in g_telemetry_server\n");

	m_telemetryServerLogging = REGISTER_INT("g_telemetry_logging", k_defaultTelemetryLogging, 0, "Usage: g_telemetry_logging <level>\nWhere level is 0, 1, or 2 - 0 is critical log messages only, 2 being verbose\nDefault is " STRINGIZE(k_defaultTelemetryLogging));

	m_telemetryUploadErrorLog = REGISTER_INT("g_telemetry_upload_errorlogs", k_defaultTelemetryUploadErrorLogs, 0, "Usage: g_telemetry_upload_errorlogs <enabled>\nWith enabled=0 no error logs will be uploaded\nWith enabled=1 error logs from previous sessions will be uploaded to the previous session");
	m_telemetryUploadGameLog = REGISTER_INT("g_telemetry_upload_gamelogs", k_defaultTelemetryUploadGameLogs, 0, "Usage: g_telemetry_upload_gamelogs <enabled>\nWith enabled=0 no game logs will be uploaded\nWith enabled=1 game logs from previous sessions, and currently ending sessions where log_verbosity>0 will be uploaded\nWith enabled=2 only gamelogs from the previous session will be uploaded, and only when the previous session has crashed");
	m_telemetryCompressGameLog = REGISTER_INT("g_telemetry_compress_gamelog", k_defaultTelemetryCompressGameLogs, 0, "Usage: g_telemetry_compress_gamelog <1/0>\nWith 1, game.log is gzipped and uploaded as game.log.gz to the server, otherwise it is uploaded raw");

	m_telemetryUploadInProgress = REGISTER_INT("g_telemetry_upload_in_progress", 0, VF_NULL, "Usage: only used to communicate with Amble scripts/stress tester");

	REGISTER_COMMAND(k_telemetry_submitLogCommand, (ConsoleCommandFunc)SubmitGameLog, 0, "Saves the Game.log to the gamelogger server");
	REGISTER_COMMAND(k_telemetry_getSessionIdCommand, (ConsoleCommandFunc)OutputSessionId, 0, "Outputs the current telemetry session id to the console");

	m_lastLevelRotationIndex = 0;
	m_previousSessionCrashChecked=false;
	m_previousSessionCrashed = false;

	g_pGame->GetIGameFramework()->GetILevelSystem()->AddListener(this);

#ifdef ENABLE_PROFILING_CODE
	char fileName[] = "%USER%/MiscTelemetry/memory_stats.txt";
	m_telemetryMemoryLogPath=fileName;

	CDebugAllowFileAccess allowFileAccess;
	gEnv->pDrxPak->RemoveFile(m_telemetryMemoryLogPath.c_str());
	FILE *file = gEnv->pDrxPak->FOpen(m_telemetryMemoryLogPath.c_str(), "at");
	allowFileAccess.End();

	if (file)
	{
		gEnv->pDrxPak->FPrintf(file, "<sheet>\n");
		gEnv->pDrxPak->FPrintf(file, "</sheet>\n");
		gEnv->pDrxPak->FClose(file);
	}

	m_fileUploadCounter = 0; // before any files are uploaded
#endif

	// populate largefilesubmit data array with default constructed elements
	for (i32 i=0; i<k_maxNumLargeFilesSubmitting; i++)
	{
		m_largeFileSubmits.push_back();
	}


#if TELEMETRY_CHECKS_FOR_OLD_ERRORLOGS
	// before we nuke any old session hint files
	CheckForPreviousSessionCrash();
#endif

	//bool submittedFile=UploadLargeFileForPreviousSession(".\\system.cfg", "system.cfg.test");
	//submittedFile=UploadLargeFileForPreviousSession(".\\testMeFile.txt", "testMeFile.txt.test");

	UploadLastGameLogToPreviousSession();

	SetNewSessionId(false);

	UpdateClientName();

#ifdef ENABLE_PROFILING_CODE
	string	tmpStr = "%USER%/TelemetryTransactions";
	char path[IDrxPak::g_nMaxPath];
	path[sizeof(path) - 1] = 0;
	gEnv->pDrxPak->AdjustFileName(tmpStr, path, IDrxPak::FLAGS_PATH_REAL | IDrxPak::FLAGS_FOR_WRITING);

	m_telemetryRecordingPath=path;

	gEnv->pDrxPak->MakeDir(m_telemetryRecordingPath.c_str());
#endif
}

CTelemetryCollector::~CTelemetryCollector()
{
	if (m_pTelemetry)
	{
		m_pTelemetry->Terminate(false);
	}

	while (!m_queuedTransfers.empty())
	{
		delete m_queuedTransfers.front().pProducer;
		m_queuedTransfers.pop();
	}

	IConsole		*ic=gEnv->pConsole;
	if (ic)
	{
		ic->UnregisterVariable(m_telemetryCompressionLevel->GetName());
		ic->UnregisterVariable(m_telemetryCompressionWindowBits->GetName());
		ic->UnregisterVariable(m_telemetryCompressionMemLevel->GetName());
		ic->UnregisterVariable(m_telemetryTransactionRecordings->GetName());
		ic->UnregisterVariable(m_telemetryEnabled->GetName());
		ic->UnregisterVariable(m_telemetryServerLogging->GetName());
		ic->UnregisterVariable(m_telemetryUploadErrorLog->GetName());
		ic->UnregisterVariable(m_telemetryUploadGameLog->GetName());
		ic->UnregisterVariable(m_telemetryCompressGameLog->GetName());
		ic->RemoveCommand(k_telemetry_submitLogCommand);
		ic->RemoveCommand(k_telemetry_getSessionIdCommand);
	}

	g_pGame->GetIGameFramework()->GetILevelSystem()->RemoveListener(this);

	assert(s_telemetryCollector==this);
	s_telemetryCollector=NULL;
}

// static
// outputs the session id to the console
void CTelemetryCollector::OutputSessionId(IConsoleCmdArgs *inArgs)
{
	ITelemetryCollector		*tc=g_pGame->GetITelemetryCollector();
	DrxLogAlways("Telemetry: Session id = '%s'",tc->GetSessionId().c_str());
}

// static
// test function which uploads the game.log
void CTelemetryCollector::SubmitGameLog(IConsoleCmdArgs *inArgs)
{
	CTelemetryCollector		*tc=static_cast<CTelemetryCollector*>(static_cast<CGame*>(gEnv->pGame)->GetITelemetryCollector());
	const char				*logFile=gEnv->pSystem->GetILog()->GetFileName();

	if (tc)
	{
		TTelemetrySubmitFlags		flags=k_tf_none;


		DrxFixedStringT<512>					modLogFile(logFile);

		if (logFile != NULL && logFile[0]!='.' && logFile[0]!='%')		// if the log file isn't set to write into an alias or the current directory, it will default to writing into the current directory. we need to prepend our path to access it from here
		{
			modLogFile.Format(".\\%s",logFile);
		}

		ITelemetryProducer			*prod=new CTelemetryFileReader(modLogFile,0);
		if (tc->m_telemetryCompressGameLog->GetIVal())
		{
			prod=new CTelemetryCompressor(prod);
			modLogFile+=".gz";
		}

		tc->SubmitTelemetryProducer(prod,modLogFile.c_str());
	}
}

// moves file out of the way, adding a datestamp to the target filename
// assumes .log extension is needed to be added onto the end of targetFilename
bool CTelemetryCollector::MoveLogFileOutOfTheWay(
	tukk inSourceFilename,
	tukk inTargetFilename)
{
	bool success=false;

	if (gEnv->pDrxPak->IsFileExist(inSourceFilename, IDrxPak::eFileLocation_OnDisk))
	{
		DrxLog("CTelemetryCollector::MoveLogFileOutOfTheWay() has found that inSourceFilename=%s DOES exist - preparing to rename it.", inSourceFilename);

		DrxFixedStringT<128> timeStr;
		time_t ltime;
		time( &ltime );
		tm *today = localtime( &ltime );
		strftime(timeStr.m_str, timeStr.MAX_SIZE, "%Y-%m-%d-%H-%M-%S", today);

		DrxFixedStringT<128> newTargetFilename;
		newTargetFilename.Format("%s_%s.log", inTargetFilename, timeStr.c_str());

		char sourceFullPathBuf[IDrxPak::g_nMaxPath];
		tukk sourceFullPath = gEnv->pDrxPak->AdjustFileName(inSourceFilename, sourceFullPathBuf, IDrxPak::FOPEN_HINT_QUIET);
		char targetFullPathBuf[IDrxPak::g_nMaxPath];
		tukk targetFullPath = gEnv->pDrxPak->AdjustFileName(newTargetFilename.c_str(), targetFullPathBuf, IDrxPak::FOPEN_HINT_QUIET);

		if (sourceFullPath != NULL && targetFullPath != NULL)
		{
			// file rename - TRC/TCR failure?
			// can use crypak's AdjustFileName() ?
			i32 result=rename(sourceFullPath, targetFullPath);
			if (result)
			{
				DrxLog("CTelemetryCollector::MoveLogFileOutOfTheWay() failed to rename error file from %s to %s return=%d (errno=%d)", sourceFullPath, targetFullPath, result, errno);
			}
			else
			{
				DrxLog("CTelemetryCollector::MoveLogFileOutOfTheWay() succeeded in renaming error file from %s to %s", sourceFullPath, targetFullPath);
				success=true;
			}
		}
		else
		{
			DrxLog("CTelemetryCollector::MoveLogFileOutOfTheWay() failed to generate full paths for the old (%s (%p)) and new (%s (%p)) error log files", inSourceFilename, sourceFullPath, newTargetFilename.c_str(), targetFullPath);
		}
	}
	else
	{
		DrxLog("CTelemetryCollector::MoveLogFileOutOfTheWay() has found that inSourceFilename=%s does NOT exist - doing nothing", inSourceFilename);
	}

	return success;
}

bool CTelemetryCollector::ShouldUploadGameLog(bool forPreviousSession)
{
	bool should=false;

	if (forPreviousSession)
	{
		if (m_telemetryUploadGameLog->GetIVal() == 1)
		{
			should=true;
		}
		else if (m_telemetryUploadGameLog->GetIVal() == 2)
		{
			DRX_ASSERT_MESSAGE(m_previousSessionCrashChecked, "ShouldUploadGameLog() is trying to upload gamelogs only if we've crashed, yet we've not actually tested whether we've crashed yet!!!");
			if (m_previousSessionCrashed)
			{
				should=true;
			}
		}
	}
	else
	{
		// we're uploading current live gamelogs - only worth doing if log_verbosity is > 0
		if (gEnv->pLog->GetVerbosityLevel() > 0)
		{
			should = (m_telemetryUploadGameLog->GetIVal() == 1);	// cvar==2 means only submit previous game session logs if that session crashed
		}
	}

	return should;
}


// uses the current telemetry hint file which contains the details of the last session that was running
// using different parameters are able to transform a local file into a different file on the remote server
bool CTelemetryCollector::UploadFileForPreviousSession(tukk inFileName, tukk inRemoteFileName)
{
	bool submittedFile=false;

	DrxLog("CTelemetryCollector::UploadFileForPreviousSession() inFileName=%s; inRemoteFileName=%s", inFileName, inRemoteFileName);

	if (gEnv->pDrxPak->IsFileExist(inFileName, IDrxPak::eFileLocation_OnDisk))
	{
		DrxLog("CTelemetryCollector::UploadFileForPreviousSession() - %s exists", inFileName);

		i32 fileSize = gEnv->pDrxPak->FGetSize(inFileName);

		if (gEnv->pDrxPak->IsFileExist(k_hintFileName, IDrxPak::eFileLocation_OnDisk))
		{
			DrxLog("CTelemetryCollector::UploadFileForPreviousSession() %s exists", k_hintFileName);
			
			// Get last session details
			CDebugAllowFileAccess allowFileAccess;
			FILE *file = gEnv->pDrxPak->FOpen(k_hintFileName, "rt");
			allowFileAccess.End();

			if (file)
			{
				char hintFileData[k_maxHttpHeaderSize];
				DrxFixedStringT<k_maxHttpHeaderSize> hintFileString;

				memset(hintFileData, 0, sizeof(hintFileData));

				gEnv->pDrxPak->FRead(hintFileData, sizeof(hintFileData), file);
				gEnv->pDrxPak->FClose(file);
				hintFileData[sizeof(hintFileData)-1]=0;
				hintFileString = hintFileData;

				DrxFixedStringT<128> fileSizeStr;
				fileSizeStr.Format("%d", fileSize);
				hintFileString.replace("_telemetryserverdestfile_", inRemoteFileName);
				hintFileString.replace("-666666", fileSizeStr );

				DrxLog("patched hintFileString is %s", hintFileString.c_str());

				submittedFile = SubmitFile(inFileName, inRemoteFileName, hintFileString.c_str(), hintFileString.length());
			}
			else
			{
				DrxLog("CTelemetryCollector::UploadFileForPreviousSession() - failed to open hintFile=%s", k_hintFileName);
			}
		}
		else
		{
			DrxLog("CTelemetryCollector::UploadFileForPreviousSession() - hintFile=%s doesn't exist", k_hintFileName);
		}
	}
	else
	{
		DrxLog("CTelemetryCollector::UploadFileForPreviousSession() - inFileName=%s doesn't exist", inFileName);
	}

	return submittedFile;
}

// uses the current telemetry hint file which contains the details of the last session that was running
// using different parameters are able to transform a local file into a different file on the remote server
bool CTelemetryCollector::UploadLargeFileForPreviousSession(tukk inFileName, tukk inRemoteFileName, TTelemetrySubmitFlags inFlags)
{
	bool submittedFile=false;

	DrxLogAlways("CTelemetryCollector::UploadLargeFileForPreviousSession() inFileName=%s; inRemoteFileName=%s", inFileName, inRemoteFileName);

	if (gEnv->pDrxPak->IsFileExist(inFileName, IDrxPak::eFileLocation_OnDisk))
	{
		DrxLogAlways("CTelemetryCollector::UploadLargeFileForPreviousSession() - %s exists", inFileName);

		i32 fileSize = gEnv->pDrxPak->FGetSize(inFileName);

		if (gEnv->pDrxPak->IsFileExist(k_hintFileName, IDrxPak::eFileLocation_OnDisk))
		{
			DrxLogAlways("CTelemetryCollector::UploadLargeFileForPreviousSession() %s exists", k_hintFileName);
			
			// Get last session details
			CDebugAllowFileAccess allowFileAccess;
			FILE *file = gEnv->pDrxPak->FOpen(k_hintFileName, "rt");
			allowFileAccess.End();

			if (file)
			{
				char hintFileData[k_maxHttpHeaderSize];
				memset(hintFileData, 0, sizeof(hintFileData));

				i32 readSize = gEnv->pDrxPak->FRead(hintFileData, sizeof(hintFileData), file);
				gEnv->pDrxPak->FClose(file);
				hintFileData[readSize]=0;

				submittedFile = SubmitLargeFile(inFileName, inRemoteFileName, 0, hintFileData, readSize+1, inFlags);
			}
			else
			{
				DrxLogAlways("CTelemetryCollector::UploadLargeFileForPreviousSession() - failed to open hintFile=%s", k_hintFileName);
			}
		}
		else
		{
			DrxLogAlways("CTelemetryCollector::UploadLargeFileForPreviousSession() - hintFile=%s doesn't exist", k_hintFileName);
		}
	}
	else
	{
		DrxLogAlways("CTelemetryCollector::UploadFileForPreviousSession() - inFileName=%s doesn't exist", inFileName);
	}

	return submittedFile;
}

void CTelemetryCollector::UploadLastGameLogToPreviousSession()
{
	DrxLog("CTelemetryCollector::UploadLastGameLogToPreviousSession()");

	if (ShouldUploadGameLog(true))
	{
		DrxLog("CTelemetryCollector::UploadLastGameLogToPreviousSession() we SHOULD upload gamelog");

		// Not bothering with the sanity file checks that error.log uploading does as the last game.log will change every run anyway
		tukk constBackupLogFileName = gEnv->pLog->GetBackupFileName();
		
		DrxLog("CTelemetryCollector::UploadLastGameLogToPreviousSession() constBackupLogFileName=%s", constBackupLogFileName);

		DrxFixedStringT<256> backupLogFileName;
		if (constBackupLogFileName[0] == '.' && constBackupLogFileName[1] == '/')
		{
			DrxLog("CTelemetryCollector::UploadLastGameLogToPreviousSession() found first characters of backupLogFileName is ./ using fileName as is");
			backupLogFileName=constBackupLogFileName;
		}
		else if (constBackupLogFileName[0] == 'd' && constBackupLogFileName[1] == ':' && constBackupLogFileName[2]=='\\')
		{
			DrxLog("CTelemetryCollector::UploadLastGameLogToPreviousSession() found first characters of backupLogFileName is d:\\ using fileName as is");
			backupLogFileName=constBackupLogFileName;
		}
		else
		{
			DrxLog("CTelemetryCollector::UploadLastGameLogToPreviousSession() found first characters of backupLogFileName are NOT ./ so adding them now to fix path");
			backupLogFileName.Format("./%s", constBackupLogFileName);
			DrxLog("CTelemetryCollector::UploadLastGameLogToPreviousSession() created new backupLogFileName of %s", backupLogFileName.c_str());
		}
		
		if (gEnv->pDrxPak->IsFileExist(backupLogFileName.c_str(), IDrxPak::eFileLocation_OnDisk))
		{
			DrxLog("CTelemetryCollector::UploadLastGameLogToPreviousSession() backupLogFileName=%s exists - submitting it using last sessionID", backupLogFileName.c_str());

			bool submittedFile=UploadLargeFileForPreviousSession(backupLogFileName.c_str(), "game.log", m_telemetryCompressGameLog->GetIVal() ? k_tf_gzipRemoteFile : k_tf_none);
			
			if (submittedFile)
			{
				DrxLog("CTelemetryCollector::UploadLastGameLogToPreviousSession() has succeeded uploading backupLogFileName=%s for previous session", backupLogFileName.c_str());
			}
			else
			{
				DrxLog("CTelemetryCollector::UploadLastGameLogToPreviousSession() has failed uploading backupLogFileName=%s for previous session", backupLogFileName.c_str());
			}
		}
	}
	else
	{
		DrxLog("CTelemetryCollector::UploadLastGameLogToPreviousSession() we should NOT upload gamelog - doing nothing");
	}
}


void CTelemetryCollector::CheckForPreviousSessionCrash()
{
	m_previousSessionCrashChecked=true;

#if TELEMETRY_CHECKS_FOR_OLD_ERRORLOGS
	DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash()");

	const char errorLogFileName[] = "./error.log";
	const char crashCheckFileName[] = "./_checking_for_crash_.txt";
	bool successfullyCheckedForCrash=true;

	if (m_telemetryUploadErrorLog->GetIVal() == 0)
	{
		DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() has found that g_telemetry_upload_errorlogs is set to 0 so skipping all error.log checking");
		return;
	}

	if (gEnv->pDrxPak->IsFileExist(crashCheckFileName, IDrxPak::eFileLocation_OnDisk))
	{
		DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() has found crash checkFileName - assuming that a previous attempt to check for crashes has itself crashed.. moving any offending files out of the way");
		
		bool removeCrashCheckFile=false;

		if (gEnv->pDrxPak->IsFileExist(errorLogFileName, IDrxPak::eFileLocation_OnDisk))
		{
			DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() clearing up crash causing files and has found an error.log, moving it out of the way");

			bool movedFile=MoveLogFileOutOfTheWay(errorLogFileName, "./error_crashing");

			// only remove the crash check file if we've successfully moved the error.log file that has caused crashing
			if (movedFile)
			{
				DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() succeeded renaming and moving the error.log file out of the way. Now removing the crashCheckFile to allow crashes to be processed again later");
				removeCrashCheckFile=true;
			}
		}
		else
		{
			DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() has found an existing checkFileName but cannot find any error.log. clearing up the checkFileName and continuing");
			removeCrashCheckFile=true;
		}

		if (removeCrashCheckFile)
		{
			if (gEnv->pDrxPak->RemoveFile(crashCheckFileName))
			{
				DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() succeeded deleting the crashCheckFileName=%s - everything is setup to work with crashes again", crashCheckFileName);
			}
			else
			{
				DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() has failed to delete the crashCheckFileName=%s - crash checking processing will be skipped on future runs", crashCheckFileName);
			}
		}

		return;
	}

	DrxFixedStringT<128> timeStr;
	time_t ltime;
	time( &ltime );
	tm *today = localtime( &ltime );
	strftime(timeStr.m_str, timeStr.MAX_SIZE, "%Y-%m-%d-%H-%M-%S", today);

	CDebugAllowFileAccess allowFileAccess;
	FILE *crashCheckFile = gEnv->pDrxPak->FOpen(crashCheckFileName, "wt");
	allowFileAccess.End();

	if (crashCheckFile)
	{
		DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() succeeded opening crashCheckFile=%s for writing", crashCheckFileName);
		gEnv->pDrxPak->FWrite(timeStr.c_str(), timeStr.length(), crashCheckFile);

		if (gEnv->pDrxPak->FError(crashCheckFile))
		{
			DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() failed to write to crashCheckFile=%s", crashCheckFileName);
		}
		gEnv->pDrxPak->FClose(crashCheckFile);
	}
	else
	{
		DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() failed to open crashCheckFile=%s for writing.", crashCheckFileName);
	}

	if (!gEnv->pDrxPak->IsFileExist(crashCheckFileName, IDrxPak::eFileLocation_OnDisk))
	{
		DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() crashCheckFile=%s; doesn't exist when it should and so we're not able to safeguard this operation. Skipping any error.log handling", crashCheckFileName);
		return;
	}

	if (gEnv->pDrxPak->IsFileExist(errorLogFileName, IDrxPak::eFileLocation_OnDisk))
	{
		DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() - %s exists", errorLogFileName);

		m_previousSessionCrashed=true;

		bool submittedFile=UploadFileForPreviousSession(errorLogFileName, errorLogFileName);
		
		DrxFixedStringT<128> newErrorLogFilename;
		if (submittedFile)
		{
			newErrorLogFilename.Format("./error_success");
		}
		else
		{
			newErrorLogFilename.Format("./error_failed");
		}

		bool movedFile = MoveLogFileOutOfTheWay(errorLogFileName, newErrorLogFilename.c_str());

		if (movedFile)
		{
			DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() has succeeded in moving the error.log file out of the way. All done");
		}
		else
		{
			DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() has failed in moving the error.log file out of the way. As last resort trying to delete this file to ensure that error.logs aren't ever submitted to the wrong session");
			if (gEnv->pDrxPak->RemoveFile(errorLogFileName))
			{
				DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() has succeeded in deleting error.log - crash details have been lost but no session has been polluted by a crash that wasn't its");
			}
			else
			{
				DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() has failed to delete the error.log - leaving crashCheckFile to ensure that no session gets polluted by this rogue error.log");
				successfullyCheckedForCrash=false;
			}
		}
	}
	else
	{
		DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() - error logfile=%s doesn't exist", errorLogFileName);
	}

	if (successfullyCheckedForCrash)
	{
		DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() has finished and successfully checked for crash so removing safeguard file=%s", crashCheckFileName);
	
		if (gEnv->pDrxPak->RemoveFile(crashCheckFileName))
		{
			DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() succeeded deleting the crashCheckFileName=%s - everything is setup to work with crashes again", crashCheckFileName);
		}
		else
		{
			DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() has failed to delete the crashCheckFileName=%s - crash checking processing will be skipped on future runs", crashCheckFileName);
		}
	}
	else
	{
		DrxLog("CTelemetryCollector::CheckForPreviousSessionCrash() has finished and NOT successfully checked for crash so leaving safeguard file=%s. crash checking processing will be skipped on future runs", crashCheckFileName);
	}
#endif // TELEMETRY_CHECKS_FOR_OLD_ERRORLOGS
}

tukk CTelemetryCollector::GetWebSafePlatformName()
{
	// This function chould use CGame::GetPlatform() function
#if DRX_PLATFORM_ANDROID
    return "android";
#elif DRX_PLATFORM_WINDOWS || DRX_PLATFORM_MAC || DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	return "pc";
#elif DRX_PLATFORM_DURANGO
	return "durango";
#elif DRX_PLATFORM_ORBIS
	return "ps4";
#elif DRX_PLATFORM_IOS
    return "ios";
#else
#error unknown platform
	return "unknown";
#endif
}

string CTelemetryCollector::GetWebSafeSessionId()
{
	string	webSafe(m_curSessionId);
	GameNetworkUtils::WebSafeEscapeString(&webSafe);
	return webSafe;
}

IPlatformOS::TUserName CTelemetryCollector::GetHostName()
{
	IPlatformOS::TUserName hostName;
#if DRX_PLATFORM_WINDOWS
	// On PC use the actual host name
	hostName=gEnv->pSystem->GetPlatformOS()->GetHostName();
#else
	// On consoles use the signed in user name
	hostName=gEnv->pSystem->GetUserName();
#endif

	if ( hostName.empty() )
	{
		hostName="unknown_host";
	}

	u32k hostNameHash = CCrc32::Compute(hostName.c_str());

	hostName.Format("%lu",hostNameHash);
	return hostName;
}

IPlatformOS::TUserName CTelemetryCollector::GetProfileName()
{
	u32k userIndex = g_pGame->GetExclusiveControllerDeviceIndex();

	IPlatformOS::TUserName profileName;

	if (gEnv->pSystem->GetPlatformOS()->UserGetOnlineName(userIndex, profileName) == false)
	{
		gEnv->pSystem->GetPlatformOS()->UserGetName(userIndex, profileName);
	}

	u32k profileNameHash = CCrc32::Compute(profileName.c_str());

	profileName.Format("%lu",profileNameHash);
	return profileName;
}

string CTelemetryCollector::GetWebSafeClientName()
{
	return m_websafeClientName;
}

void CTelemetryCollector::UpdateClientName()
{
	const char	*hostName=GetHostName();
	const char	*profileName=GetProfileName();
	
	m_websafeClientName.clear();
#if DRX_PLATFORM_WINDOWS
	i32 instance = gEnv->pSystem->GetApplicationInstance();
	if( instance != 0 )
	{
		m_websafeClientName.Format("%d_%s_%s", instance, hostName, profileName);
	}
	else
	{
		m_websafeClientName.Format("%s_%s", hostName, profileName);
	}
#else
	// On consoles hostName and profileName are always the same
	m_websafeClientName.Format("%s", profileName);
#endif

	GameNetworkUtils::WebSafeEscapeString(&m_websafeClientName);
}

bool CTelemetryCollector::InitService()
{
	// for debugging
	IDrxLobbyService*		pLobbyService = gEnv->pNetwork->GetLobby()->GetLobbyService();

	if ( pLobbyService )
	{
		{
			m_pTelemetry = pLobbyService->GetTCPService( g_pGameCVars->g_dataCentreConfig );
		}
	}

	return m_pTelemetry != 0;
}

// called from network thread to free large file data
// if the large file data is in a state where it is waiting for the main thread to do some work
// lock to ensure we don't clear the data at the same time the main thread is working on it
void CTelemetryCollector::ThreadSafeClear(
	SLargeFileSubmitData			*pInData)
{
	bool			locked=false;

	if (pInData->m_state==CTelemetryCollector::SLargeFileSubmitData::k_state_chunk_submitted_send_next_one)
	{
		m_largeFileMutex.Lock();
		locked=true;
	}

	if (pInData->m_pDownloadableResource)
	{
		// this will be handled on the main thread later
		pInData->m_state=CTelemetryCollector::SLargeFileSubmitData::k_state_dispatchCallbacksAndClear;
	}
	else
	{
		pInData->Clear();
	}

	if (locked)
	{
		m_largeFileMutex.Unlock();
	}
}

// static
bool CTelemetryCollector::SubmitFileTelemetryCallback(EDrxTCPServiceResult res, uk arg, STCPServiceDataPtr pInUploadData, tukk pData, size_t dataLen, bool endOfStream)
{
	CTelemetryCollector *telemetryCollector=CTelemetryCollector::GetTelemetryCollector();
	bool								keepAlive=false;
	bool								keepAliveForReceive=false;

	CTelemetryCollector::SLargeFileSubmitData *largeSubmitData=NULL;

	if (telemetryCollector)
	{
		largeSubmitData=telemetryCollector->FindLargeFileSubmitDataFromData((tukk)arg);
		if (largeSubmitData)
		{
			// if we're in the state of dispatching callbacks and clearing, then this callback has queued this resource
			// for deletion and it can be deleted from the main thread at any time - do not call it further
			if (largeSubmitData->m_pDownloadableResource && (largeSubmitData->m_state != CTelemetryCollector::SLargeFileSubmitData::k_state_dispatchCallbacksAndClear))
			{
				keepAliveForReceive=!largeSubmitData->m_pDownloadableResource->ReceiveDataCallback(res,largeSubmitData->m_pDownloadableResource.get(),pInUploadData,pData,dataLen,endOfStream);
			}
		}
	}

	if (res == eCTCPSR_Ok)
	{
		ICVar	*telemetryServerLogging = gEnv->pConsole->GetCVar("g_telemetry_logging");

		if (!telemetryServerLogging || telemetryServerLogging->GetIVal() >= 2)
		{
			DrxLog("Telemetry: Successfully sent job with data=%p", arg);
			if (dataLen)
			{
				DrxFixedStringT<512>		nullT(pData,dataLen);

				DrxLog("%s",nullT.c_str());
			}
		}

		if (telemetryCollector)
		{
			if (largeSubmitData)
			{
				keepAlive=((largeSubmitData->m_flags&k_tf_chunked)!=0) || keepAliveForReceive;

				switch (largeSubmitData->m_state)
				{
					case CTelemetryCollector::SLargeFileSubmitData::k_state_submitted:
						if (!keepAliveForReceive)
						{
							keepAlive=false;							// must not keep alive unnecessarily
							// if the server has finished and sent a reply and end of stream to us and we try and keep it alive, it will
							// close it anyway and our counter logic will get messed up

							if (!keepAlive)
							{
								telemetryCollector->ThreadSafeClear(largeSubmitData);
							}
						}
						break;

					case CTelemetryCollector::SLargeFileSubmitData::k_state_dispatchCallbacksAndClear:
						break;

					case CTelemetryCollector::SLargeFileSubmitData::k_state_chunk_submitted_waiting:
						largeSubmitData->m_state = CTelemetryCollector::SLargeFileSubmitData::k_state_chunk_submitted_send_next_one;
						break;

					case CTelemetryCollector::SLargeFileSubmitData::k_state_chunk_submitted_send_next_one:
						// should only get a callback in this state if we've previously kept the connection alive for chunked transfers
						// it means the network has called back again, but the main thread still hasn't supplied the data
						assert(largeSubmitData->m_flags&k_tf_chunked);
						break;

					case CTelemetryCollector::SLargeFileSubmitData::k_state_chunked_transfer_data_available:
					case CTelemetryCollector::SLargeFileSubmitData::k_state_chunked_transfer_data_ended:
						{
							// data has been supplied by main thread, provide it to the caller
							pInUploadData->length=largeSubmitData->m_chunkDataSize;
							pInUploadData->sent=0;

							if (largeSubmitData->m_state==CTelemetryCollector::SLargeFileSubmitData::k_state_chunked_transfer_data_available)
							{
								largeSubmitData->m_state=CTelemetryCollector::SLargeFileSubmitData::k_state_chunk_submitted_waiting;
							}
							else
							{
								largeSubmitData->m_state=CTelemetryCollector::SLargeFileSubmitData::k_state_submitted;
							}
						}
						break;
				}

				if (keepAlive && endOfStream)
				{
					// if we get here, it means the server has ended the session when we wanted it to stay open. this generally won't happen as we should get eCTCPSR_Failed
					// however, if the server times out on us, it could close the socket or send us an end of stream whilst we're still waiting for the main thread to feed us
					// data. in this case, we hit this and abort and clean up
					telemetryCollector->ThreadSafeClear(largeSubmitData);
					keepAlive=false;
				}
			}
			else
			{
				if (!endOfStream)
				{
					if (dataLen>0)		// received reply yet? otherwise keep alive so we can get the http result
					{
							DrxLog("HTTP REPLY FOR JOB %p : %s",arg,string(pData,dataLen).c_str());
					}
					else
					{
						if (pInUploadData->m_quietTimer>k_httpTimeOut)
						{
							DrxLog("HTTP REPLY FOR JOB %p : TIMED OUT - upload may or may not have succeed",arg);
						}
						else
						{
							DrxLog("HTTP REPLY FOR JOB %p : <PENDING>",arg);
							keepAlive=true;
						}
					}
				}
			}
		}
	}
	else
	{
		DrxLogAlways("Telemetry: Failed to send job with data=%p", arg);

		// Find LargeFileData from data

		if (telemetryCollector)
		{
			if (largeSubmitData)
			{
				telemetryCollector->ThreadSafeClear(largeSubmitData);
			}
		}
	}

	// for chunked transfers, don't decrement the transfers in progress count, as we aren't finished with this transfer yet and the next chunk will go into this one and not a new one
	if (telemetryCollector != NULL && !keepAlive)
	{
		telemetryCollector->UpdateTransfersInProgress(-1);
	}

	return !keepAlive;
}


// writes a http post header for the telemetry request into the buffer provided
// returns the length of the header so other data can be packed behind it
// inDataLength should be the length of the data that will follow
i32 CTelemetryCollector::MakePostHeader(
	const char			*inRemoteFileName,
	i32					inDataLength,
	char				*outBuffer,
	i32					inMaxBufferSize,
	TTelemetrySubmitFlags	inFlags)
{
	DrxLog("MakePostHeader");
	INDENT_LOG_DURING_SCOPE();
#define MAX_HEADER_SIZE			(1024)
	DrxFixedStringT<MAX_HEADER_SIZE>	httpPostHeader;
	const char	*destFile=inRemoteFileName;
	const char	*pathPrefix= m_pTelemetry ? m_pTelemetry->GetURLPrefix() : "";

	if (m_pTelemetry==NULL)
	{
		return 0;
	}
	DrxLog("SessionId : %s",m_curSessionId.c_str());
	DrxLog("destFile='%s'",destFile);
	// NOTE: Most of this path adjustment code is redundant as long as inRemoteFileName is in fact
	// a path on the remote server rather than a path on the local file system
	destFile += GetIndexOfFirstValidCharInFilePath(destFile, strlen(destFile), true, true);

	char szFullPathBuf[IDrxPak::g_nMaxPath];

	// Would be nice to also specify IDrxPak::FLAGS_NO_LOWCASE here, but that also stops the expansion of %USER% into the user directory [TF]
	IDrxPak			*pak=gEnv->pDrxPak;
	pak->AdjustFileName(destFile, szFullPathBuf, IDrxPak::FLAGS_NO_FULL_PATH);

	// strip .\ off the beginning of file names
	// .\ is used to refer to a file in the games root directory but putting .\ at the beginning of file names on the server is not the intention
	// start with a drive letter? we don't want those on the server either
	string webSafeFileName(szFullPathBuf + GetIndexOfFirstValidCharInFilePath(szFullPathBuf, strlen(szFullPathBuf), true, true));
	DrxLog( "webSafeFileName='%s'",webSafeFileName.c_str());

	GameNetworkUtils::WebSafeEscapeString(&webSafeFileName);

	// No longer needed - this is done by AdjustFileName
#if 0
	// replace windows slashes with unix slashes (proper slashes :) )
	webSafeFileName.replace('\\','/');
#endif

	tukk contentType="text/plain";

	if (inFlags&k_tf_md5Digest)
	{
		contentType="crysis2/md5";
	}

	string sTags = g_pGameCVars->g_telemetryTags;
	GameNetworkUtils::WebSafeEscapeString(&sTags);

	if (inFlags&k_tf_chunked)
	{
		tukk hostName=m_pTelemetry->GetServerName();		// HTTP 1.1 requires hostname
		i32k hostPort=m_pTelemetry->GetPort();		// And port if it's not 80

		httpPostHeader.Format("POST %s/filestore.php?filename=%s&session2=%s&client=%s&platform=%s&isserver=%d&isstream=%d&append=%d&tags=%s HTTP/1.1\n"
			"Host: %s:%d\n"
			"Content-Type: %s\n"
			"Transfer-Encoding: chunked\n"
			"Connection: close\n"
			"\n",
			pathPrefix,
			webSafeFileName.c_str(),GetWebSafeSessionId().c_str(),GetWebSafeClientName().c_str(),GetWebSafePlatformName(),gEnv->bServer!=0,(inFlags&k_tf_isStream)!=0,(inFlags&k_tf_appendToRemoteFile)!=0,sTags.c_str(),hostName,hostPort,contentType);
	}
	else
	{
		DRX_ASSERT_MESSAGE((inFlags&k_tf_isStream) == 0, "Streaming is only supported with chunked uploads");

		httpPostHeader.Format("POST %s/filestore.php?filename=%s&session2=%s&client=%s&platform=%s&isserver=%d&append=%d&tags=%s HTTP/1.0\n"
			"Content-Type: %s\n"
			"Content-Length: %d\n"
			"\n",
			pathPrefix,
			webSafeFileName.c_str(),GetWebSafeSessionId().c_str(),GetWebSafeClientName().c_str(),GetWebSafePlatformName(),gEnv->bServer!=0,(inFlags&k_tf_appendToRemoteFile)!=0,sTags.c_str(),contentType,inDataLength);
	}

	i32 len=httpPostHeader.length();

	if (len>=inMaxBufferSize)
	{
		DRX_ASSERT_MESSAGE(len<inMaxBufferSize,"Http header too long for provided buffer");
		len=inMaxBufferSize;
	}

	memcpy(outBuffer,httpPostHeader.c_str(),len);

	return len;
}

void CTelemetryCollector::UpdateQueuedProducers()
{
	while (!m_queuedTransfers.empty())
	{
		SQueuedProducer		&prod=m_queuedTransfers.front();
		if (TrySubmitTelemetryProducer(prod.pProducer,prod.postHeader.c_str(),prod.postHeader.length(),prod.callback,prod.callbackData,prod.flags))
		{
			m_queuedTransfers.pop();
		}
		else
		{
			break;
		}
	}
}

void CTelemetryCollector::Update()
{
	UpdateLargeFileSubmitData();
	UpdateQueuedProducers();

	m_telemetryUploadInProgress->Set(AreTransfersInProgress() ? 1 : 0);
}

i32 CTelemetryCollector::GetIndexOfFirstValidCharInFilePath(tukk pPath, i32k len, const bool stripDotSlash, const bool stripDriveSpec)
{

	tukk  pChr = pPath;

	if (stripDotSlash)
	{
		if (pChr[0]=='.' && (pChr[1]=='\\'))  
		{
			pChr += 2;
		}
		if (pChr[0]=='.' && (pChr[1]=='/'))  
		{
			pChr += 2;
		}
	}

	if (stripDriveSpec)
	{
		if (pChr[0] && pChr[1]==':')
		{
			pChr += 2;
			if (pChr[0]=='/' || pChr[0]=='\\')
			{
				pChr += 1;
			}
		}
	}

	i32  idx = (i32)(pChr - pPath);

	DRX_ASSERT(idx >= 0);
	DRX_ASSERT(idx < len);
	return idx;
}

// pass in NULL inHeaderData to generate a new header for current session
bool CTelemetryCollector::SubmitFile(
	const char			*inLocalFilePath,
	const char			*inRemoteFilePath,
	const char			*inHeaderData,
	i32k				inHeaderLength)
{
	bool				success=false;
	i32					shouldSubmit=ShouldSubmitTelemetry();

	DrxLog("CTelemetryCollector::SubmitFile(), inLocalFilePath=%s, sessionId='%s', inRemoteFilePath=%s, inHeaderData=%p, inHeaderLength=%d, shouldSubmit=%d", inLocalFilePath, GetSessionId().c_str(), inRemoteFilePath, inHeaderData, inHeaderLength, shouldSubmit);

	if (shouldSubmit)
	{
		IDrxPak			*pak=gEnv->pDrxPak;

		CDebugAllowFileAccess allowFileAccess;
		FILE			*file=pak->FOpen(inLocalFilePath,"rb",IDrxPak::FLAGS_PATH_REAL|IDrxPak::FOPEN_ONDISK);
		allowFileAccess.End();

		if (!file)
		{
			Log(TLOG_CRITICAL, "Telemetry: Failed to open file '%s' for submit",inLocalFilePath);
		}
		else
		{
			pak->FSeek(file,0, SEEK_END);
			i32 nSize=pak->FTell(file);
			pak->FSeek(file, 0, SEEK_SET);
			
			STCPServiceDataPtr pUploadData = new STCPServiceData();

			if (inHeaderData && inHeaderLength>0)
			{
				pUploadData->length = nSize + inHeaderLength;
			}
			else
			{
				pUploadData->length = nSize + k_maxHttpHeaderSize;
			}
			pUploadData->pData = new char[ pUploadData->length ];

			i32 headerLen=0;
			if (inHeaderData && inHeaderLength>0)
			{
				memcpy(pUploadData->pData, inHeaderData, inHeaderLength);
				headerLen=inHeaderLength;
			}
			else
			{
				headerLen=MakePostHeader(inRemoteFilePath,nSize,pUploadData->pData,k_maxHttpHeaderSize,k_tf_none);
				pUploadData->length = nSize + headerLen;
			}

			pak->FReadRaw(pUploadData->pData+headerLen,nSize,1,file);

			success=UploadData(pUploadData,inRemoteFilePath);

			pak->FClose(file);
		}
	}

	return success;
}

// will adopt the passed producer immediately, if it returns false the producer will have been deleted
// will return true / false depending on whether it successfully set the transfer going
bool CTelemetryCollector::SubmitTelemetryProducer(
		ITelemetryProducer		*pInProducer,
		const char						*pInRemoteFilePath,
		TSubmitResultCallback	inCallback,
		void									*inCallbackData,
		TTelemetrySubmitFlags inFlags)
{
	ScopedSwitchToGlobalHeap globalHeapSwitch;
	char				postHeader[k_maxHttpHeaderSize];
	inFlags |= k_tf_chunked;	// Telemetry producers must be chunked
	if ((inFlags & k_tf_isStream) && (inFlags & k_tf_md5Digest))
	{
		DRX_ASSERT_MESSAGE(false, "Streamed uploads cannot use MD5!");
		delete pInProducer;
		return false;
	}
	i32					len=MakePostHeader(pInRemoteFilePath,k_magicSizeInt,postHeader,sizeof(postHeader),inFlags);

	if (!TrySubmitTelemetryProducer(pInProducer,postHeader,len,inCallback,inCallbackData,inFlags))
	{
		SQueuedProducer		q;
		q.pProducer=pInProducer;
		q.postHeader.assign(postHeader,len);
		q.callback=inCallback;
		q.callbackData=inCallbackData;
		q.flags=inFlags;
		m_queuedTransfers.push(q);
	}

	return true;
}

// will attempt to start a transfer using the producer passed
// will return true/false depending on success
// if it returns true, it will adopt the producer. if it returns false it will not.
bool CTelemetryCollector::TrySubmitTelemetryProducer(
	ITelemetryProducer		*pInProducer,
	const char						*inPostHeader,
	i32										inLen,
	TSubmitResultCallback	inCallback,
	void									*inCallbackData,
	TTelemetrySubmitFlags inFlags)
{
	bool	success=false;

	if (pInProducer)
	{
		SLargeFileSubmitData *pLargeSubmitData = GetAvailableLargeFileSubmitData();

		if (pLargeSubmitData)
		{
			if (inFlags & k_tf_md5Digest)
			{
				pInProducer=new CTelemetryMD5(k_salt,pInProducer);
			}

			CTelemetryHTTPPostChunkSplitter *pSplit=new CTelemetryHTTPPostChunkSplitter(pInProducer);
			pInProducer=pSplit;
			pLargeSubmitData->m_pProducer=pInProducer;
			drx_strcpy(pLargeSubmitData->m_remoteFileName,"<telemetry producer>");		// can always extract name from post header later if it is needed

			assert(inFlags&k_tf_chunked);		// Telemetry producers must be chunked
			DRX_ASSERT_MESSAGE(inLen<=i32(sizeof(pLargeSubmitData->m_postHeaderContents)),"http post header too long, truncating - message liable to get lost");
			inLen=min(inLen,i32(sizeof(pLargeSubmitData->m_postHeaderContents)));
			memcpy(pLargeSubmitData->m_postHeaderContents,inPostHeader,inLen);
			pLargeSubmitData->m_postHeaderSize=inLen;
			pLargeSubmitData->m_state=SLargeFileSubmitData::k_state_chunk_submitted_send_next_one;
			pLargeSubmitData->m_flags=inFlags;

			if (inCallback)
			{
				pLargeSubmitData->m_callback=inCallback;
				pLargeSubmitData->m_callbackData=inCallbackData;
				pLargeSubmitData->m_pDownloadableResource=new CDownloadableResource();
				pLargeSubmitData->m_pDownloadableResource->InitHTTPParser();
			}

			success=true;
		}
	}

	return success;
}

// pass in NULL inHintFileData to generate a new header for current session
// if inHintFileData is NOT NULL then its data must be 0 terminated 
bool CTelemetryCollector::SubmitLargeFile(
	const char						*inLocalFilePath,
	const char						*inRemoteFilePath,
	i32										inLocalFileOffset,				// = 0
	const char						*inHintFileData,					// = NULL
	i32k							inHintFileDataLength,			// = 0
	TTelemetrySubmitFlags	inFlags)									// = k_tf_none
{
	bool success=false;
	i32	shouldSubmit=ShouldSubmitTelemetry();

	if (shouldSubmit)
	{
		SLargeFileSubmitData *pLargeSubmitData = GetAvailableLargeFileSubmitData();
		bool									shouldCompress=((inFlags&k_tf_gzipRemoteFile)!=0);
		bool									shouldDigest=((inFlags&k_tf_md5Digest)!=0);

		if (pLargeSubmitData)
		{
			ITelemetryProducer		*pProducer=new CTelemetryFileReader(inLocalFilePath,inLocalFileOffset);
			CTelemetryCompressor	*pCompressor=NULL;

			if (shouldCompress)
			{
				pCompressor = new CTelemetryCompressor(pProducer);		// pCompressor will adopt pFileReader
				pProducer=pCompressor;
			}

			if (shouldDigest)
			{
				CTelemetryMD5	*pMD5 = new CTelemetryMD5(k_salt,pProducer);
				pProducer = pMD5;
			}

			pLargeSubmitData->m_pProducer=pProducer;
			pLargeSubmitData->m_flags=inFlags;

			drx_strcpy(pLargeSubmitData->m_remoteFileName,inRemoteFilePath);

			if (inHintFileData && inHintFileDataLength>0)
			{
				DRX_ASSERT_MESSAGE(inHintFileDataLength < k_maxHttpHeaderSize, "SubmitLargeFile() hintFile passed in is too big.. not using it!");
				if (inHintFileDataLength < k_maxHttpHeaderSize)
				{
					memcpy(pLargeSubmitData->m_postHeaderContents, inHintFileData, inHintFileDataLength);
					pLargeSubmitData->m_postHeaderSize = inHintFileDataLength;
				}
			}

			if (pLargeSubmitData->m_postHeaderSize==0)
			{
				DrxFixedStringT<512>		rname(inRemoteFilePath);
				if (pCompressor)
				{
					rname+=".gz";
				}
				pLargeSubmitData->m_postHeaderSize=MakePostHeader(rname.c_str(),k_magicSizeInt,pLargeSubmitData->m_postHeaderContents,k_maxHttpHeaderSize,inFlags);
			}

			pLargeSubmitData->m_state=SLargeFileSubmitData::k_state_chunk_submitted_send_next_one;

			success=true;
		}
		else
		{
			Log(TLOG_CRITICAL,"Failed to allocate large file stream to upload %s",inRemoteFilePath);
		}
	}

	return success;
}

CTelemetrySaveToFile::CTelemetrySaveToFile(
	ITelemetryProducer						*pInProducer,
	string												inFileToSaveTo) :
	m_fileName(inFileToSaveTo),
	m_pFile(NULL),
	m_pSource(pInProducer)
{
	CDebugAllowFileAccess allowFileAccess;
	m_pFile=gEnv->pDrxPak->FOpen(m_fileName.c_str(),"wt");
	if (!m_pFile)
	{
		DrxLog("Failed to open file '%s' for writing",m_fileName.c_str());
	}
}

CTelemetrySaveToFile::~CTelemetrySaveToFile()
{
	if (m_pFile && gEnv->pDrxPak)
	{
		gEnv->pDrxPak->FClose(m_pFile);
	}
	delete m_pSource;
}

ITelemetryProducer::EResult CTelemetrySaveToFile::ProduceTelemetry(
	tuk													pOutBuffer,
	i32														inMinRequired,
	i32														inBufferSize,
	i32*													pOutWritten)
{
	EResult												result;

	result=m_pSource->ProduceTelemetry(pOutBuffer,inMinRequired,inBufferSize,pOutWritten);

	switch (result)
	{
		case eTS_Available:
		case eTS_EndOfStream:
			{
				i32		toWrite=*pOutWritten;
				i32		written = 0;
				if (toWrite > 0)
				{
					if (m_pFile)
						written = gEnv->pDrxPak->FWrite(pOutBuffer,1,toWrite,m_pFile);
					
					if (written != toWrite)
					{
						DrxLog("Failed to write %d bytes to file %s",toWrite,m_fileName.c_str());
					}
				}
			}
			break;

		default:
			break;
	}

	return result;
}

// passed producer is adopted and will be deleted when this producer is
CTelemetryCompressor::CTelemetryCompressor(
	ITelemetryProducer		*inPSource) :
	m_pSource(inPSource),
	m_lastSourceResult(eTS_Available),
	m_lastUncompressedSize(0),
	m_zstream(NULL),
	m_dataIncoming(false)
{
}

CTelemetryCompressor::~CTelemetryCompressor()
{
	SAFE_RELEASE(m_zstream);
	delete m_pSource;
}

ITelemetryProducer::EResult CTelemetryCompressor::ProduceTelemetry(
	char				*pOutBuffer,
	i32					inMinRequired,
	i32					inBufferSize,
	i32					*pOutWritten)
{
	i32					maxUncompressedDataSize=inBufferSize/2;
	EResult			result=eTS_EndOfStream;
	CTelemetryCollector			*tc=CTelemetryCollector::GetTelemetryCollector();

	DRX_ASSERT_MESSAGE(maxUncompressedDataSize>=inMinRequired,"CTelemetryCompressor the minimum requested amount of data doesn't leave enough space in the buffer to produce a compressed copy");

	*pOutWritten=0;

	if (!m_zstream)
	{
		m_zstream=GetISystem()->GetIZLibCompressor()->CreateDeflateStream(tc->m_telemetryCompressionLevel->GetIVal(),eZMeth_Deflated,tc->m_telemetryCompressionWindowBits->GetIVal(),tc->m_telemetryCompressionMemLevel->GetIVal(),eZStrat_Default,eZFlush_NoFlush);

		if (!m_zstream)
		{
			result=eTS_EndOfStream;
			CTelemetryCollector::GetTelemetryCollector()->Log(TLOG_CRITICAL,"Failed to allocate zlib compressor stream");
		}
	}

	if (maxUncompressedDataSize>inMinRequired && m_zstream)
	{
		// store uncompressed data in back half of buffer, compress into fore half of buffer
		char				*pUncompressedData=pOutBuffer+maxUncompressedDataSize;
		char				*pCompressedData=pOutBuffer;

		EZDeflateState		zstreamState=m_zstream->GetState();

		switch (zstreamState)
		{
			case eZDefState_AwaitingInput:
				if (m_lastSourceResult==eTS_EndOfStream)
				{
					m_zstream->EndInput();
					m_dataIncoming=true;
					result=eTS_Pending;
				}
				else
				{
					m_lastSourceResult=m_pSource->ProduceTelemetry(pUncompressedData,inMinRequired,maxUncompressedDataSize,&m_lastUncompressedSize);
					switch (m_lastSourceResult)
					{
						case eTS_Pending:
							result=eTS_Pending;
							break;

						case eTS_Available:
						case eTS_EndOfStream:
							m_zstream->Input(pUncompressedData,m_lastUncompressedSize);
							m_dataIncoming=true;
							result=eTS_Pending;
							break;

						default:
							assert(0);
							break;
					}
				}
				break;

			case eZDefState_ConsumeOutput:
			case eZDefState_Finished:
				{
					bool		outputHasBeenConsumed=!m_dataIncoming;

					if (!outputHasBeenConsumed)
					{
						// data has just arrived this poll, return to caller without setting the compressor off again
						m_dataIncoming=false;

						i32 bytesAvailable=m_zstream->GetBytesOutput();

						*pOutWritten=bytesAvailable;

						if (zstreamState==eZDefState_Finished)
						{
							result=eTS_EndOfStream;

#if !defined(_RELEASE)
							IZLibDeflateStream::SStats		stats;
							m_zstream->GetStats(&stats);
							CTelemetryCollector::GetTelemetryCollector()->Log(TLOG_VERBOSE,"telemetry compression complete - input %d bytes, out %d bytes, mem peak %d bytes",stats.bytesInput,stats.bytesOutput,stats.peakMemoryUsed);
#endif
						}
						else if (bytesAvailable>0)
						{
							result=eTS_Available;
						}
						else
						{
							outputHasBeenConsumed=true;
						}
					}

					if (outputHasBeenConsumed)
					{
						// data arrived on previous poll, begin next compress
						if (zstreamState!=eZDefState_Finished)
						{
							m_dataIncoming=true;
							m_zstream->SetOutputBuffer(pCompressedData,maxUncompressedDataSize);
							result=eTS_Pending;
						}
						else
						{
							result=eTS_EndOfStream;
						}
					}
				}
				break;

			case eZDefState_Deflating:
				result=eTS_Pending;
				break;

			case eZDefState_Error:
				DrxLog("CTelemetryCompressor :: error compressing data stream");
				result=eTS_EndOfStream;
				break;

			default:
				assert(0);
				break;
		}
	}

	return result;
}

CTelemetryStreamCipher::CTelemetryStreamCipher(
	ITelemetryProducer	*pInSource,
	const char					*pInKey,
	i32									inKeyLen) :
	m_pSource(pInSource)
{
	m_cipher=GetISystem()->GetINetwork()->BeginCipher((u8k*)pInKey,inKeyLen);
}

CTelemetryStreamCipher::~CTelemetryStreamCipher()
{
	GetISystem()->GetINetwork()->EndCipher(m_cipher);
	delete m_pSource;
}

ITelemetryProducer::EResult CTelemetryStreamCipher::ProduceTelemetry(
	char				*pOutBuffer,
	i32					inMinRequired,
	i32					inBufferSize,
	i32					*pOutWritten)
{
	i32					half=inBufferSize/2;

	DRX_ASSERT_MESSAGE(inMinRequired<=half,"min required is too great for the size of the output buffer in CTelemetryStreamCipher");

	EResult result = m_pSource->ProduceTelemetry(pOutBuffer+half,min(half,inMinRequired),half,pOutWritten);

	switch (result)
	{
		case eTS_Available:
		case eTS_EndOfStream:
			GetISystem()->GetINetwork()->Encrypt(m_cipher,(u8*)pOutBuffer,(u8*)pOutBuffer+half,*pOutWritten);
			break;

		default:
			break;
	}

	return result;
}

// adopts the passed producer and will delete it upon destruction
CTelemetryHTTPPostChunkSplitter::CTelemetryHTTPPostChunkSplitter(
	ITelemetryProducer			*inProducer) :
	m_producer(inProducer),
	m_contentLength(0),
	m_state(k_writeChunks)
{
}

CTelemetryHTTPPostChunkSplitter::~CTelemetryHTTPPostChunkSplitter()
{
	delete m_producer;
}

ITelemetryProducer::EResult CTelemetryHTTPPostChunkSplitter::ProduceTelemetry(
	char				*pOutBuffer,
	i32					inMinRequired,
	i32					inBufferSize,
	i32					*pOutWritten)
{
	EResult			result=eTS_EndOfStream;
	// http 1.1 chunks are each prefixed by a size in ascii hex, followed by a crlf, then the data, followed by another crlf
	// at the end, the stream is terminated by a 0 size chunk
	// after that footers can optionally follow

	*pOutWritten=0;
	
	switch (m_state)
	{
		case k_writeChunks:
			{
				// reserve some space in the buffer to write the chunk size
				static i32k					k_headerReserveSpace=6+2;		// 2 bytes crlf, 6 hex digits (0 pad)
				static i32k					k_footerReserveSpace=2;
				static i32k					k_reserveSpace=k_headerReserveSpace+k_footerReserveSpace;

				assert((k_reserveSpace+inMinRequired)<=inBufferSize);

				if ((k_reserveSpace+inMinRequired)<=inBufferSize)
				{
					result=m_producer->ProduceTelemetry(pOutBuffer+k_headerReserveSpace,inMinRequired,inBufferSize-k_reserveSpace,pOutWritten);

					switch (result)
					{
						case eTS_Pending:
							break;

						case eTS_Available:
						case eTS_EndOfStream:
							{
								i32			written=*pOutWritten;

								assert(written<=(inBufferSize-k_reserveSpace));		// something would have gone very wrong if this fires

								// insert chunk header and footer
								char		buff[k_headerReserveSpace+1];
								drx_sprintf(buff,sizeof(buff),"%06x\r\n",written);
								memcpy(pOutBuffer,buff,k_headerReserveSpace);

								pOutBuffer[written+k_headerReserveSpace]='\r';
								pOutBuffer[written+k_headerReserveSpace+1]='\n';

								*pOutWritten+=k_reserveSpace;

								if (result==eTS_EndOfStream)
								{
									if (written==0)		// no data, this counts as a terminating chunk
									{
										m_state=k_writeFooter;
									}
									else
									{
										m_state=k_writeTerminatingChunkAndFooter;		// last chunk and it's not empty, need to write a terminating chunk
									}

									result=eTS_Available;
								}

								m_contentLength+=written;
							}
							break;

						default:
							assert(0);
							break;
					}
				}
			}
			break;

		case k_writeTerminatingChunkAndFooter:
		case k_writeFooter:
			{
				DrxFixedStringT<64>		footer;

				if (m_state==k_writeTerminatingChunkAndFooter)
				{
					footer.Format("0\r\n");
				}

#if 0	// TODO - need to figure out how to get php to process trailing footers on the server side, aldo, can't use the Content-Length tag, will probably have to use a magic byte sequence + EOM string
				if (result==k_writeTerminatingChunkAndFooter)
				{
					footer.Format("0\r\nContent-Length: %d\r\n",m_contentLength);
				}
				else
				{
					footer.Format("Content-Length: %d\r\n",m_contentLength);
				}
#endif

				footer+="\r\n";

				i32			footerLength=footer.length();

				assert(footerLength<=inBufferSize);			// could fix this by splitting footer over multiple produce() calls, but it is an unnecessary complication for the buffer sizes in use

				footerLength=min(footerLength,inBufferSize);

				memcpy(pOutBuffer,footer.c_str(),footerLength);

				*pOutWritten=footerLength;

				m_state=k_done;

				result=eTS_EndOfStream;
			}
			break;

		case k_done:
			result=eTS_EndOfStream;
			break;

		default:
			assert(0);
			break;
	}

	return result;
}
	
i32k CTelemetryCopier::k_blockSize=16*1024;

CTelemetryCopier::CTelemetryCopier(ITelemetryProducer *pIn)
{
	EResult	res;
	char *tmpBuffer=(tuk)malloc(k_blockSize);
	m_sent=0;
	m_size=0;
	do
	{
		i32 written;
		while (1)
		{
			res=pIn->ProduceTelemetry(tmpBuffer, 0, k_blockSize, &written);
			if (res!=eTS_Pending)
			{
				break;
			}
		}
		AddData(tmpBuffer, written);
	}
	while (res==eTS_Available);
	free(tmpBuffer);
}

void CTelemetryCopier::AddData(tukk src, i32 size)
{
	while (size>0)
	{
		i32 offset=m_size%k_blockSize;
		i32 maxCopy=k_blockSize-offset;
		i32 toCopy=size;
		if (toCopy>maxCopy)
		{
			toCopy=maxCopy;
		}
		if (offset==0)
		{
			m_buffers.push_back((tuk)malloc(k_blockSize));
		}
		memcpy(m_buffers.back()+offset, src, toCopy);
		src+=toCopy;
		m_size+=toCopy;
		size-=toCopy;
	}
}

CTelemetryCopier::~CTelemetryCopier ()
{
	for (i32 i=0, iEnd=m_buffers.size(); i<iEnd; i++)
	{
		free(m_buffers[i]);
	}
}

ITelemetryProducer::EResult CTelemetryCopier::ProduceTelemetry(
	tuk				pOutBuffer,
	i32					inMinRequired,
	i32					inBufferSize,
	i32*				pOutWritten)
{
	EResult			result=eTS_Available;
	i32 copyTo=m_sent+inBufferSize;
	i32 totalSent=0;

	if (copyTo>=m_size)
	{
		result=eTS_EndOfStream;
		copyTo=m_size;
	}

	while (m_sent!=copyTo)
	{
		i32 blockId=m_sent/k_blockSize;
		i32 blockOffset=m_sent%k_blockSize;
		i32 sending=copyTo-m_sent;
		if (sending>(k_blockSize-blockOffset))
		{
			sending=k_blockSize-blockOffset;
		}
		memcpy(pOutBuffer+totalSent, &m_buffers[blockId][blockOffset], sending);
		m_sent+=sending;
		totalSent+=sending;
	}

	*pOutWritten=totalSent;

	return result;
}

CTelemetryMemBufferProducer::CTelemetryMemBufferProducer(
	tukk 														pInBuffer,
	i32																		inBufferSize,
	TTelemetryMemBufferDisposalCallback		pInDisposalCallback,
	uk 																	pInDisposalCallbackData) :
	m_pBuffer(pInBuffer),
	m_bufferSize(inBufferSize),
	m_dataSent(0),
	m_pDisposerCallback(pInDisposalCallback),
	m_pDisposerCallbackData(pInDisposalCallbackData)
{
}

ITelemetryProducer::EResult CTelemetryMemBufferProducer::ProduceTelemetry(
	tuk				pOutBuffer,
	i32					inMinRequired,
	i32					inBufferSize,
	i32*				pOutWritten)
{
	EResult			result=eTS_EndOfStream;
	i32					dataRemaining=m_bufferSize-m_dataSent;
	i32					dataAvailable=min(inBufferSize,dataRemaining);

	memcpy(pOutBuffer,m_pBuffer+m_dataSent,dataAvailable);
	*pOutWritten=dataAvailable;
	m_dataSent+=dataAvailable;

	if (m_dataSent<m_bufferSize)
	{
		result=eTS_Available;
	}

	return result;
}

CTelemetryMemBufferProducer::~CTelemetryMemBufferProducer()
{
	if (m_pDisposerCallback)
	{
		m_pDisposerCallback(m_pDisposerCallbackData);
	}
}

ITelemetryProducer::EResult CTelemetryFileReader::ProduceTelemetry(
	char				*pOutBuffer,
	i32					inMinRequired,
	i32					inBufferSize,
	i32					*pOutWritten)
{
	CDebugAllowFileAccess		allowFileAccess;			// TODO if this is to be used for production - this will need to move to a streaming API if available. this will probably not be possible for upload of game.log
	EResult									result=eTS_EndOfStream;
	IDrxPak									*pPak=gEnv->pDrxPak;

	FILE										*pFile=pPak->FOpen(m_localFilePath.c_str(),"rb",IDrxPak::FLAGS_PATH_REAL|IDrxPak::FOPEN_ONDISK);
	allowFileAccess.End();

	*pOutWritten=0;

	if (!pFile)
	{
		CTelemetryCollector::GetTelemetryCollector()->Log(TLOG_CRITICAL, "Telemetry: Failed to open large file '%s' for submit", m_localFilePath.c_str());
	}
	else
	{
		pPak->FSeek(pFile,0, SEEK_END);

		i32 fileSize=pPak->FTell(pFile);
		i32 offsetToEndSize=fileSize-m_fileOffset;

		i32	amountToRead=min(offsetToEndSize,inBufferSize);

		i32 sresult=pPak->FSeek(pFile,m_fileOffset, SEEK_SET);

		if (sresult)
		{
			CTelemetryCollector::GetTelemetryCollector()->Log(TLOG_CRITICAL,string().Format("CTelemetryFileReader::ProduceTelemetry() has failed to seek to file offset %d in file %s",m_fileOffset,m_localFilePath.c_str()));
		}
		else
		{
			i32		amountRead=pPak->FReadRaw(pOutBuffer,1,amountToRead,pFile);

			if (amountRead!=amountToRead)
			{
				CTelemetryCollector::GetTelemetryCollector()->Log(TLOG_CRITICAL,string().Format("CTelemetryFileReader::ProduceTelemetry() - file truncated when attempting to read %d bytes from file %s (got %d)",amountToRead,m_localFilePath.c_str()),amountRead);
			}
			else if (amountRead<offsetToEndSize)
			{
				result=eTS_Available;
			}

			*pOutWritten=amountRead;

			m_fileOffset+=amountRead;
		}

		pPak->FClose(pFile);
		pFile = NULL;
	}

	return result;
}

ITelemetryProducer::EResult CStreamedTelemetryProducer::ProduceTelemetry(char *pOutBuffer, i32 inMinRequired, i32 inBufferSize, i32 *pOutWritten)
{
	DRX_ASSERT(inMinRequired == 0);
	if (!m_proxy->m_pendingData.empty())
	{
		*pOutWritten = min(inBufferSize, (i32)m_proxy->m_pendingData.size());
		memcpy(pOutBuffer, m_proxy->m_pendingData.c_str(), *pOutWritten);
		m_proxy->m_pendingData.erase(0, *pOutWritten);
	}
	else
	{
		*pOutWritten = 0;
	}
	if (m_proxy->m_finished && m_proxy->m_pendingData.empty())
	{
		return eTS_EndOfStream;
	}
	else if (*pOutWritten != 0)
	{
		return eTS_Available;
	}
	else
	{
		return eTS_Pending;
	}
}

void CStreamedTelemetryProxy::WriteString(tukk string)
{
	m_pendingData.append(string);
	m_pendingData.append("\n");
}

void CStreamedTelemetryProxy::FormatString(tukk format, ...)
{
	char temp[4096]; // Limited to 4096 characters!
	va_list argList;
	va_start(argList, format);
	drx_vsprintf(temp, format, argList); 
	va_end(argList);
	WriteString(temp);
}

void CTelemetryCollector::CreateEventStream()
{
#ifdef USE_TELEMETRY_EVENTS_LOG
	if (!m_eventsStream)
	{
		CStreamedTelemetryProducer* pProducer = new CStreamedTelemetryProducer();
		m_eventsStream = pProducer->GetProxy();
		SubmitTelemetryProducer(pProducer, "events.xml", NULL, NULL, k_tf_isStream);
		m_eventsStream->WriteString("<root>");
	}
	else
	{
		// This currently may happen because NetSerializeTelemetry uses eEA_GameServerStatic as does USE_PC_PREMATCH
		//DRX_ASSERT_MESSAGE(false, "Events stream already exists!");
	}
#endif
}

void CTelemetryCollector::CloseEventStream()
{
#ifdef USE_TELEMETRY_EVENTS_LOG
	if (m_eventsStream)
	{
		m_eventsStream->WriteString("</root>");
		m_eventsStream->CloseStream();
		m_eventsStream.reset();
	}
#endif
}

void CTelemetryCollector::CreateStatoscopeStream()
{
#if USE_STATOSCOPE_TELEMETRY
	if (gEnv->pStatoscope->IsLoggingForTelemetry() && m_pTelemetry)
	{
		i32k maxHeaderSize = CTelemetryCollector::k_maxHttpHeaderSize;
		char headerBuffer[maxHeaderSize];
		i32 headerSize = MakePostHeader("statoscope.bin", 0, headerBuffer, maxHeaderSize-1, CTelemetryCollector::k_tf_chunked|CTelemetryCollector::k_tf_isStream);
		headerBuffer[headerSize] = 0;
		gEnv->pStatoscope->CreateTelemetryStream(headerBuffer, m_pTelemetry->GetServerName(), m_pTelemetry->GetPort());
		gEnv->pStatoscope->Tick();	// tick to enable SS (and allow the AddUserMarker() below to succeed)

		string text;
		text.Format("sessionStart %s", GetSessionId().c_str());
		gEnv->pStatoscope->AddUserMarker("Telemetry", text.c_str());
	}
#endif //#if USE_STATOSCOPE_TELEMETRY
}

void CTelemetryCollector::CloseStatoscopeStream()
{
#if USE_STATOSCOPE_TELEMETRY
	if (gEnv->pStatoscope->IsRunning() && gEnv->pStatoscope->IsLoggingForTelemetry())
	{
		gEnv->pStatoscope->AddUserMarker("Telemetry", "sessionEnd");
		gEnv->pStatoscope->Tick();
		gEnv->pStatoscope->Flush();
		gEnv->pStatoscope->CloseTelemetryStream();
	}
#endif //#if USE_STATOSCOPE_TELEMETRY
}

void CTelemetryCollector::LogEvent(tukk eventName, float value)
{
#ifdef USE_TELEMETRY_EVENTS_LOG
	if (m_eventsStream)
	{
		float serverTimeInSeconds = 0.0f;
		CGameRules *pGameRules = g_pGame->GetGameRules();
		if (pGameRules)
		{
			serverTimeInSeconds = pGameRules->GetServerTime() / 1000.0f;
		}
		m_eventsStream->FormatString("<event name='%s' value='%f' time='%f'/>", eventName, value, serverTimeInSeconds);
	}
	else
	{
		DrxLog("Telemetry Event: %s - %f", eventName, value);
		DRX_ASSERT_MESSAGE(false, "Unable to log event, event stream is not open");
	}
#endif
}

// write a header in for this chunk
// header size is a fixed length for this file of pInLargeSubmitData->m_postHeaderSize
void CTelemetryCollector::UpdateLargeFileChunkPostHeader(
	SLargeFileSubmitData				*pInLargeFile,
	bool												inFirstChunk,
	i32													inPayloadSize)
{
	DrxFixedStringT<32> payloadSizeStr;
	payloadSizeStr.Format("%*d",10,inPayloadSize);				// HTTP - any number of spaces can be between the : and the value in the header

	DrxFixedStringT<k_maxHttpHeaderSize> hintFileString(pInLargeFile->m_postHeaderContents,pInLargeFile->m_postHeaderSize);
	hintFileString.replace(k_magicSizeStr,payloadSizeStr);

	if (inFirstChunk)
	{
		hintFileString.replace("&append=1", "&append=0");
	}
	else
	{
		hintFileString.replace("&append=0", "&append=1");
	}

	DRX_ASSERT_MESSAGE(hintFileString.length()==pInLargeFile->m_postHeaderSize, "CTelemetryCollector::MakeLargeFileChunkPostHeader() header length unexpectedly changed");		// if the header len changes it may overwrite the first few bytes of the payload

	memcpy(pInLargeFile->m_chunkData,hintFileString.c_str(),hintFileString.length());
}

void CTelemetryCollector::SubmitChunkOfALargeFile(
	SLargeFileSubmitData		*pInLargeSubmitData)
{
	bool								success=false;

	ITelemetryProducer::EResult		result=pInLargeSubmitData->m_pProducer->ProduceTelemetry(pInLargeSubmitData->m_chunkData+pInLargeSubmitData->m_postHeaderSize,0,k_largeFileSubmitChunkSize-pInLargeSubmitData->m_postHeaderSize,&pInLargeSubmitData->m_chunkDataSize);

	switch (result)
	{
		case ITelemetryProducer::eTS_Available:
		case ITelemetryProducer::eTS_EndOfStream:
			{
				STCPServiceDataPtr		pUploadData;

				if (!(pInLargeSubmitData->m_flags&k_tf_chunked) || pInLargeSubmitData->m_isFirstChunk)
				{
					UpdateLargeFileChunkPostHeader(pInLargeSubmitData,pInLargeSubmitData->m_isFirstChunk,pInLargeSubmitData->m_chunkDataSize);
				
					pUploadData=new STCPServiceData();

					pUploadData->pData=pInLargeSubmitData->m_chunkData;
					pUploadData->ownsData=false;		// we own chunkData and don't want it being deleted once the data has been uploaded
					pUploadData->length=pInLargeSubmitData->m_chunkDataSize+pInLargeSubmitData->m_postHeaderSize;

					pInLargeSubmitData->m_state=(result==ITelemetryProducer::eTS_Available ? SLargeFileSubmitData::k_state_chunk_submitted_waiting : SLargeFileSubmitData::k_state_submitted);

					// for chunked transfers, only upload the data once, then rely on the callback updating the network with the next chunk
					success=UploadData(pUploadData,pInLargeSubmitData->m_remoteFileName);

					if (pInLargeSubmitData->m_flags&k_tf_chunked)
					{
						pInLargeSubmitData->m_postHeaderSize=0;			// subsequent chunks shouldn't have a post header
					}

					if (!success)
					{
						Log(TLOG_CRITICAL,"Failed to upload chunk of file %s, stopping transfer",pInLargeSubmitData->m_remoteFileName);
						pInLargeSubmitData->Clear();
					}
				}
				else
				{
					// for chunked transfers, now the data is available we need to indicate that to the network thread
					success=true;
					pInLargeSubmitData->m_state=(result==ITelemetryProducer::eTS_Available ? SLargeFileSubmitData::k_state_chunked_transfer_data_available : SLargeFileSubmitData::k_state_chunked_transfer_data_ended);
				}

				pInLargeSubmitData->m_isFirstChunk=false;
			}
			break;

		case ITelemetryProducer::eTS_Pending:
			break;

		default:
			DRX_ASSERT_MESSAGE(0,"unexpected result from ITelemetryProducer::ProduceTelemetry()");
			break;
	}
}

CTelemetryCollector::SLargeFileSubmitData *CTelemetryCollector::GetAvailableLargeFileSubmitData()
{
	SLargeFileSubmitData *data=NULL;
	for (size_t i = 0; i < m_largeFileSubmits.size(); i++)
	{
		if (m_largeFileSubmits[i].m_state == SLargeFileSubmitData::k_state_available)
		{
			data = &m_largeFileSubmits[i];
			break;
		}
	}

	return data;
}

CTelemetryCollector::SLargeFileSubmitData *CTelemetryCollector::FindLargeFileSubmitDataFromData(tukk pData)
{
	SLargeFileSubmitData *data=NULL;
	for (size_t i = 0; i < m_largeFileSubmits.size(); i++)
	{
		if (m_largeFileSubmits[i].m_state != SLargeFileSubmitData::k_state_available)
		{
			if (m_largeFileSubmits[i].m_chunkData == pData)
			{
				data = &m_largeFileSubmits[i];
				break;
			}
		}
	}

	return data;
}

void CTelemetryCollector::UpdateLargeFileSubmitData()
{
	m_largeFileMutex.Lock();			// need to hold this lock to ensure the network thread callback doesn't free a large file whilst we are processing it

	i32 num=m_largeFileSubmits.size();
	for (i32 i=0; i<num; i++)
	{
		SLargeFileSubmitData		*pLarge=&m_largeFileSubmits[i];

		if (pLarge->m_state == SLargeFileSubmitData::k_state_chunk_submitted_send_next_one)
		{
			SubmitChunkOfALargeFile(pLarge);
		}
		else if (pLarge->m_state==SLargeFileSubmitData::k_state_dispatchCallbacksAndClear)
		{
			if (pLarge->m_pDownloadableResource)
			{
				pLarge->m_pDownloadableResource->ReleaseHTTPParser();

				if (pLarge->m_pDownloadableResource->GetState()&CDownloadableResource::k_dataAvailable)
				{
					char		*pData=NULL;
					i32			len=0;
					pLarge->m_pDownloadableResource->GetRawData(&pData,&len);
					pLarge->m_callback(pLarge->m_callbackData,true,pData,len);
				}
				else
				{
					pLarge->m_callback(pLarge->m_callbackData,false,NULL,0);
				}
				pLarge->m_pDownloadableResource=NULL;	// release
			}
			pLarge->Clear();
		}
	}

	m_largeFileMutex.Unlock();
}

// returns whether telemetry will be submitted or logged. if false, there is no point generating the telemetry and submitting it as it will
// go nowhere
bool CTelemetryCollector::ShouldSubmitTelemetry()
{
	i32					enabled=m_telemetryEnabled->GetIVal();
	i32					recording=m_telemetryTransactionRecordings->GetIVal();
	i32					serviceAvailable=enabled && InitService();

	return (enabled && (serviceAvailable || recording==k_recording_ifServiceUnavailable)) || (recording==k_recording_always);
}

void CTelemetryCollector::UpdateTransfersInProgress(i32 inDiff)
{
	m_transferCounterMutex.Lock();
	i32	newValue=m_transfersCounter+inDiff;
	m_transfersCounter=newValue;
	m_transferCounterMutex.Unlock();

	DRX_ASSERT_MESSAGE(newValue>=0,"CTelemetryCollector transfers in progress counter has become negative - internal state error in telemetry collector or TCP layer");
}

// uploads data to the telemetry server and logs the transaction if required
// returns whether or not the upload was queued successfully
bool CTelemetryCollector::UploadData(
	STCPServiceDataPtr pData,
	const char				*inReferenceFileName)
{
	bool					success=false;
	i32						recording=m_telemetryTransactionRecordings->GetIVal();
	i32						enabled=m_telemetryEnabled->GetIVal();


	// don't upload data if we're not configured - do queue up if we're trying to configure but are waiting on dns resolving
	if (enabled && InitService())
	{
		pData->tcpServReplyCb = &CTelemetryCollector::SubmitFileTelemetryCallback;
		pData->pUserArg = (uk )pData->pData; // m_fileUploadCounter;
		UpdateTransfersInProgress(+1);
		success=m_pTelemetry ? m_pTelemetry->UploadData(pData) : false;
		if (success)
		{
			Log(TLOG_STANDARD, "Telemetry: Successfully queued append to file (sessionId='%s') '%s' to server, job with data=%p", GetSessionId().c_str(), inReferenceFileName, pData->pUserArg);
		}
		else
		{
			UpdateTransfersInProgress(-1);
			Log(TLOG_CRITICAL, "Telemetry: Failed to queue submit of file '%s'.", inReferenceFileName);
		}
	}

#ifdef ENABLE_PROFILING_CODE
	if ((!success && recording==k_recording_ifServiceUnavailable) || recording==k_recording_always)
	{
		IDrxPak		*pak=gEnv->pDrxPak;
		string		sessionId=GetSessionId();

		if (sessionId.empty())
		{
			sessionId="no_session";
		}

		DrxFixedStringT<1024>	path;

		path.Format("%s/%s_%d.tel",m_telemetryRecordingPath.c_str(),sessionId.c_str(),m_fileUploadCounter);

		CDebugAllowFileAccess allowFileAccess;
		FILE		*file=pak->FOpen(path,"wb",IDrxPak::FLAGS_PATH_REAL);
		allowFileAccess.End();

		i32			written=0;
		if (file)
		{
			written=pak->FWrite(pData->pData,1,pData->length,file);
			pak->FClose(file);
		}

		if (!file || written!=pData->length)
		{
			Log(TLOG_CRITICAL, "Telemetry: Failed to record submit of file '%s' to transactions log.", inReferenceFileName);
		}
		else
		{
			Log(TLOG_STANDARD, "Telemetry: Successfully recorded submit of file '%s' to transactions log.",inReferenceFileName);
		}
	}

	// smartptr should delete pData if its failed to be copied to anywhere else
	
	m_fileUploadCounter++;
#endif
	return success;
}

bool CTelemetryCollector::AppendStringToFile(tukk inLocalFilePath, tukk inDataToAppend)
{
	return AppendToFile(inLocalFilePath, inDataToAppend, strlen(inDataToAppend));
}

bool CTelemetryCollector::AppendToFile(tukk inRemoteFilePath, tukk inDataToAppend, i32k inDataLength)
{
	return SubmitFromMemory(inRemoteFilePath,inDataToAppend,inDataLength,k_tf_appendToRemoteFile);
}

bool CTelemetryCollector::SubmitFromMemory(
	const char				*inRemoteFilePath,
	const char				*inDataToStore,
	i32k				inDataLength,
	TTelemetrySubmitFlags	inFlags)
{
	//ScopedSwitchToGlobalHeap useGlobalHeap;
	bool				success=false;
	bool				shouldSubmit=ShouldSubmitTelemetry();

	DRX_ASSERT_MESSAGE((inFlags&~(k_tf_appendToRemoteFile))==0,"unsupported flags passed to SubmitFromMemory");

	if (shouldSubmit)
	{
		i32k	nSize = inDataLength;

		STCPServiceDataPtr pUploadData = new STCPServiceData();

		pUploadData->length = nSize + k_maxHttpHeaderSize;
		pUploadData->pData = new char[ pUploadData->length ];

		if (!pUploadData->pData)
		{
			Log(TLOG_CRITICAL, "Telemetry: Failed to allocate %d bytes to write to file '%s'",nSize+k_maxHttpHeaderSize,inRemoteFilePath);
		}
		else
		{
			i32	headerlen=MakePostHeader(inRemoteFilePath,nSize,pUploadData->pData,k_maxHttpHeaderSize,inFlags);

			memcpy(pUploadData->pData+headerlen,inDataToStore,nSize);

			pUploadData->length=nSize+headerlen;		// update size for new header len

			success=UploadData(pUploadData,inRemoteFilePath);
		}
	}

	return success;
}

void CTelemetryCollector::OutputTelemetryServerHintFile()
{
#if TELEMETRY_OUTPUTS_HINT_FILE
	if ( !InitService() )
	{
		return;
	}

	char headerMem[k_maxHttpHeaderSize];

	i32 headerLen=MakePostHeader("_TelemetryServerDestFile_",-666666,headerMem,k_maxHttpHeaderSize,k_tf_none);
	
	CDebugAllowFileAccess allowFileAccess;
	FILE *file = gEnv->pDrxPak->FOpen( k_hintFileName,"wt" );
	allowFileAccess.End();

	if (file)
	{
		gEnv->pDrxPak->FWrite(headerMem, headerLen, file);
		gEnv->pDrxPak->FClose(file);
	}
#endif //TELEMETRY_OUTPUTS_HINT_FILE
}

void CTelemetryCollector::OutputMemoryUsage(tukk message, tukk newLevelName)
{
#ifdef ENABLE_PROFILING_CODE
	IMemoryUpr::SProcessMemInfo processMemInfo;
	GetISystem()->GetIMemoryUpr()->GetProcessMemInfo(processMemInfo);
	const float cpuMemUsedInMB = (float)(processMemInfo.PagefileUsage)/(1024.0f*1024.0f);

	DrxFixedStringT<128> timeStr;
	time_t ltime;
	time( &ltime );
	tm *today = localtime( &ltime );
	strftime(timeStr.m_str, timeStr.MAX_SIZE, "%Y-%m-%d %H:%M:%S", today);

	tukk mapName;
	if (g_pGame && g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel())
	{
		mapName = g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel()->GetDisplayName();
	}
	else
	{
		mapName = "unknown";
	}

	CDebugAllowFileAccess allowFileAccess;
	FILE *file = gEnv->pDrxPak->FOpen(m_telemetryMemoryLogPath.c_str(), "r+");
	allowFileAccess.End();

	if (file)
	{
		i32 fileSize=gEnv->pDrxPak->FGetSize(file);
		gEnv->pDrxPak->FSeek(file, fileSize-10, SEEK_SET);
		gEnv->pDrxPak->FPrintf(file, "  <row timestamp=\"%s\" type=\"%s\" map1=\"%s\" map2=\"%s\" memoryused=\"%f\">\n", timeStr.c_str(), message, mapName, newLevelName, cpuMemUsedInMB);
		gEnv->pDrxPak->FPrintf(file, "</sheet>\n");
		//gEnv->pDrxPak->FPrintf(file, "%s - %s - %s -> %s - %f\n", timeStr.c_str(), message, mapName, newLevelName, cpuMemUsedInMB);
		gEnv->pDrxPak->FClose(file);
	}

	SubmitFile(m_telemetryMemoryLogPath.c_str(), "USER/MiscTelemetry/memory_stats.txt");
#endif
}

string CTelemetryCollector::GetSessionId()
{
	return m_curSessionId;
}

void CTelemetryCollector::SetSessionId(
	string		inNewId)
{
	if (ShouldUploadGameLog(false))
	{
		DrxLog("CTelemetryCollector::SetSessionID() but shouldUploadGameLog so submitting it now before changing to new sessionID");
		SubmitGameLog(NULL);
	}

	m_curSessionId=inNewId;
	UpdateClientName();
	DrxLog("SetSessionId : %s",m_curSessionId.c_str());


#if TELEMETRY_OUTPUTS_HINT_FILE
	OutputTelemetryServerHintFile();
#endif //TELEMETRY_OUTPUTS_HINT_FILE

	if(gEnv->pStatoscope)
		gEnv->pStatoscope->AddUserMarker("SessionID", m_curSessionId.c_str());

	MEMSTAT_LABEL_FMT("SetSessionId: %s", m_curSessionId.c_str());
}

// the session id is used when uploading to the telemetry server to ensure all clients in the game store their telemetry
// in the same place
void CTelemetryCollector::SetNewSessionId( bool includeMatchDetails )
{
#if defined(USE_SESSION_SEARCH_SIMULATOR)
	ICVar* pSearchSimuCVar = gEnv->pConsole->GetCVar( "gl_searchSimulatorEnabled" );
	if( pSearchSimuCVar != NULL && pSearchSimuCVar->GetIVal() )
	{
		SetTestSessionId();
		//early out
		return;
	}
#endif //defined(USE_SESSION_SEARCH_SIMULATOR)

	DrxFixedStringT<1024>		hostNameStr;

	ICVar		*serverName=gEnv->pConsole->GetCVar("sv_servername");			// generally already encapsulates the player profile name
	if (serverName)
	{
		hostNameStr=serverName->GetString();
	}

	const char	*hostName=GetHostName();

	if (hostNameStr.find(hostName)==hostNameStr.npos)
	{
		if (!hostNameStr.empty())
		{
			hostNameStr+="_";
		}
		hostNameStr+=hostName;
	}

	DrxFixedStringT<64> rotationStr;
	//The playlist manager is multiplayer only. If it's null, don't attempt to add it to the sessionID
	if( includeMatchDetails && g_pGame->GetPlaylistUpr()!=NULL)
	{
		ILevelRotation *pLevelRotation = g_pGame->GetPlaylistUpr()->GetLevelRotation();
		if (pLevelRotation && pLevelRotation->GetLength() > 1)
		{
			i32 next = pLevelRotation->GetNext();

			if (m_lastLevelRotationIndex > 0 && next == 0)
			{
				// next has looped around to the first level, display the last level for this session
				next = pLevelRotation->GetLength();
			}
			rotationStr.Format("round_%02d_", next);
			m_lastLevelRotationIndex = next;
		}
	}

	DrxFixedStringT<128> timeStr;
	time_t ltime;
	time( &ltime );
	tm *today = localtime( &ltime );
	strftime(timeStr.m_str, timeStr.MAX_SIZE, "%H%M%S", today);

	i32	lobbyVersion=GameLobbyData::GetVersion();

	string newId;
	{
		//ScopedSwitchToGlobalHeap globalHeap;
		i32												patchId=0;
		if (CDataPatchDownloader *pDP=g_pGame->GetDataPatchDownloader())
		{
			patchId=pDP->GetPatchId();
		}
		newId.Format("%s_mb%d_mv%d_pat%d_%s%s",hostNameStr.c_str(),g_pGameCVars->g_MatchmakingBlock,g_pGameCVars->g_MatchmakingVersion,patchId,rotationStr.c_str(),timeStr.c_str());
	}

	SetSessionId(newId);
}

// the aborted session id is used when uploading to the telemetry server to ensure
// all clients leaving the session uplod their telemetry in the same place
void CTelemetryCollector::SetAbortedSessionId( tukk abortedSessionName, DrxSessionID& abortedSessionID )
{
#if defined(USE_SESSION_SEARCH_SIMULATOR)
	ICVar* pSearchSimuCVar = gEnv->pConsole->GetCVar( "gl_searchSimulatorEnabled" );
	if( pSearchSimuCVar != NULL && pSearchSimuCVar->GetIVal() )
	{
		SetTestSessionId();
		//early out
		return;
	}
#endif //defined(USE_SESSION_SEARCH_SIMULATOR)

	char sessionIdStr[DRXSESSIONID_STRINGLEN];
	abortedSessionID->AsCStr( sessionIdStr, DRXSESSIONID_STRINGLEN );

	DrxFixedStringT<128> newId;
	newId.Format( "%s_%s_aborted", abortedSessionName, sessionIdStr );
	
	SetSessionId(newId);
}

void CTelemetryCollector::SetTestSessionId()
{
	//Just want a local name, _test and time
	DrxFixedStringT<32> timeStr;
	time_t ltime;
	time( &ltime );
	tm *today = localtime( &ltime );
	strftime(timeStr.m_str, timeStr.MAX_SIZE, "%H%M%S", today);

	DrxFixedStringT<128> newId;
	newId.Format( "%s_test_%s", GetHostName().c_str(), timeStr.c_str() );

	SetSessionId(newId);
}

void CTelemetryCollector::OnLoadingStart(ILevelInfo *pLevel)
{
//	DrxLog("CTelemetryCollector::OnLoadingStart()");
//	OutputMemoryUsage("OnLoadingStart", pLevel->GetDisplayName());
}

void CTelemetryCollector::OnLoadingComplete(ILevelInfo* pLevel)
{
//	DrxLog("CTelemetryCollector::OnLoadingComplete()");
//	OutputMemoryUsage("OnLoadingComplete", pLevel->GetLevelInfo()->GetDisplayName());
}

bool CTelemetryCollector::AreTransfersInProgress()
{
	bool			inProgress=false;

	for (u32 i=0, m=m_largeFileSubmits.size(); i<m; i++)
	{
		SLargeFileSubmitData		*pLarge=&m_largeFileSubmits[i];
		if (pLarge->m_state!=SLargeFileSubmitData::k_state_available)
		{
			inProgress=true;
			break;
		}
	}

	return inProgress || (m_transfersCounter>0) || !m_queuedTransfers.empty();
}

void CTelemetryCollector::GetMemoryUsage( IDrxSizer* pSizer ) const
{
	pSizer->Add(*this);

	pSizer->AddString(m_curSessionId);
	pSizer->AddString(m_websafeClientName);
#ifdef ENABLE_PROFILING_CODE
	m_telemetryRecordingPath.GetMemoryUsage(pSizer);
	m_telemetryMemoryLogPath.GetMemoryUsage(pSizer);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Telemetry buffer utility class
////////////////////////////////////////////////////////////////////////////////
#if USE_TELEMETRY_BUFFERS 
CTelemetryBuffer::CTelemetryBuffer(i32 buffersize, ITelemetryCollector *collector, size_t structSize) : m_collector(collector), m_structSize(structSize)
{
	m_pBuffer = new CRecordingBuffer(buffersize);
}

CTelemetryBuffer::~CTelemetryBuffer()
{
	delete m_pBuffer;
}

void CTelemetryBuffer::AddData(ITelemetryBufferData *dataStruct)
{
	m_pBuffer->AddPacket(*dataStruct);
}

void CTelemetryBuffer::SubmitToServer(tukk filename)
{
	CTelemetryBufferProducer* pProducer = new CTelemetryBufferProducer(m_pBuffer);
	((CTelemetryCollector*)m_collector)->SubmitTelemetryProducer(pProducer, filename);
}

void CTelemetryBuffer::DumpToFile(tukk filename)
{
#define FIXED_BUFFER_SIZE (10 * 1024)
	DrxFixedStringT<FIXED_BUFFER_SIZE> tempBuffer;

	CDebugAllowFileAccess allowFileAccess;
	FILE *file = gEnv->pDrxPak->FOpen(filename, "wt");
	allowFileAccess.End();

	if (file)
	{
		size_t offset = 0;
		for (offset = 0; offset < m_pBuffer->size(); offset+=m_structSize)
		{
			ITelemetryBufferData *packet = (ITelemetryBufferData *) m_pBuffer->at(offset);
			DRX_ASSERT_MESSAGE(packet->size == m_structSize, "Struct size is not correct, something went wrong here");
			DrxFixedStringT<1024> oneString;
			FormatBufferData(packet, oneString);

			if ((tempBuffer.length() + strlen(oneString)) > FIXED_BUFFER_SIZE)
			{
				gEnv->pDrxPak->FWrite(tempBuffer, 1, tempBuffer.length(), file);
				tempBuffer.clear();
			}

			tempBuffer.append(oneString);
		}

		gEnv->pDrxPak->FWrite(tempBuffer, 1, tempBuffer.length(), file);
		gEnv->pDrxPak->FClose(file);
	}
}

void CTelemetryBuffer::FormatBufferData(ITelemetryBufferData* packet, DrxFixedStringT<1024> &outString)
{
	switch (packet->type)
	{
	case eTPT_Performance:
		{
			SPerformanceTelemetry* perfPacket = (SPerformanceTelemetry*)packet;
			outString.Format("%.2f\t%d\t%f\t%d\t%d\t%d\t%f\t%f\t%f\t%f\n", 
				perfPacket->m_timeInSeconds, perfPacket->m_numTicks, 
				perfPacket->m_gpuTime, perfPacket->m_gpuLimited,
				perfPacket->m_drawcalls,perfPacket->m_drawcallOverbudget,
				perfPacket->m_deltaTime,
				perfPacket->m_renderThreadTime, perfPacket->m_mainThreadTime,
				perfPacket->m_waitForGPUTime
											);
		}
		break;
	case eTPT_Bandwidth:
		{
			SBandwidthTelemetry* bandPacket = (SBandwidthTelemetry*)packet;
			outString.Format("%.2f\t%" PRIu64 "\t%" PRIu64 "\t%d\t%f\n", bandPacket->m_timeInSeconds, bandPacket->m_bandwidthSent, bandPacket->m_bandwidthReceived, bandPacket->m_packetsSent, bandPacket->m_deltaTime);
		}
		break;
	case eTPT_Memory:
		{
			SMemoryTelemetry* memPacket = (SMemoryTelemetry*)packet;
			outString.Format("%.2f\t%.3f\t%.3f\n", memPacket->m_timeInSeconds, memPacket->m_cpuMemUsedInMB, memPacket->m_gpuMemUsedInMB);
		}
		break;
	case eTPT_Sound:
		{
			REINST("do we still need this?");
			//SSoundTelemetry* soundPacket = (SSoundTelemetry*)packet;
			//SSoundMemoryInfo& soundInfo = soundPacket->m_soundInfo;
			//outString.Format("%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%d\t%.3f\t%.3f\t%.3f\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", /*soundInfo.fSoundBucketPoolSizeInMB*/ 0.0f, soundInfo.fSoundBucketPoolUsedInMB, 
			//	soundInfo.fSoundPrimaryPoolSizeInMB, soundInfo.fSoundPrimaryPoolUsedInMB, soundInfo.fSoundPrimaryPoolLargestBlockInMB, 
			//	soundInfo.fSoundSecondaryPoolSizeInMB, soundInfo.fSoundSecondaryPoolUsedInMB, soundInfo.nBufferTrashedCount,
			//	soundInfo.fSoundBucketPoolMaxInMB, soundInfo.fSoundPrimaryPoolMaxInMB, soundInfo.fSoundSecondaryPoolMaxInMB,
			//	soundInfo.nSoundBucketPoolAllocCountMax, soundInfo.nSoundPrimaryPoolAllocCountMax, soundInfo.nSoundSecondaryPoolAllocCountMax,
			//	soundInfo.nMemTypeNormalCount, soundInfo.nMemTypeStreamFileCount, soundInfo.nMemTypeStreamDecodeCount,
			//	soundInfo.nMemTypeXBox360PhysicalCount, soundInfo.nMemTypePersistentCount, soundInfo.nMemTypeSecondaryCount);
		}
		break;
	default:
		{
			outString.Format("TelemetryCollector::FormatBufferData ERROR unknown telemetry packet type '%d'", packet->type);
			DrxLog( "TelemetryCollector::FormatBufferData ERROR unknown telemetry packet type '%d'", packet->type);
		}
		break;
	}
}

void CTelemetryBuffer::Reset()
{
	m_pBuffer->Reset();
}

CTelemetryBufferProducer::CTelemetryBufferProducer(CRecordingBuffer *pRecordingBuffer)
	: m_pBuffer(NULL)
	, m_length(0)
	, m_offset(0)
{
	m_length = pRecordingBuffer->size();
	if (m_length > 0)
	{
		m_pBuffer = new u8[m_length];
		pRecordingBuffer->GetData(m_pBuffer, m_length);
	}
}

CTelemetryBufferProducer::~CTelemetryBufferProducer()
{
	SAFE_DELETE_ARRAY(m_pBuffer);
}

ITelemetryProducer::EResult CTelemetryBufferProducer::ProduceTelemetry(
																 char				*pOutBuffer,
																 i32					inMinRequired,
																 i32					inBufferSize,
																 i32					*pOutWritten)
{
	i32 bytesWritten = 0;
	DrxFixedStringT<1024> oneString;
	while (m_offset < m_length)
	{
		ITelemetryBufferData* pPacket = (ITelemetryBufferData*)(m_pBuffer + m_offset);
		CTelemetryBuffer::FormatBufferData(pPacket, oneString);
		if ((i32)oneString.length() + bytesWritten < inBufferSize)
		{
			memcpy(pOutBuffer + bytesWritten, oneString.data(), oneString.length());
			bytesWritten += oneString.length();
			m_offset += pPacket->size;
		}
		else
		{
			break;
		}
	}
	*pOutWritten = bytesWritten;
	if (m_offset >= m_length)
	{
		DRX_ASSERT_MESSAGE(m_offset == m_length, "The offset should be exactly equal to the length of the buffer when finished streaming");
		return eTS_EndOfStream;
	}
	else
	{
		DRX_ASSERT_MESSAGE(*pOutWritten >= inMinRequired, "Haven't written enough data to the buffer");
		return eTS_Available;
	}
}

#endif // #if USE_TELEMETRY_BUFFERS 
