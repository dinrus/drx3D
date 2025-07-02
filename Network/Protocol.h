// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: interface definition file for the Drxsis remote control system
   -------------------------------------------------------------------------
   История:
   - Created by Lin Luo, November 06, 2006
*************************************************************************/

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#pragma once

static u8k RCONSERVERMSGTYPE_INSESSION = 0;
static u8k RCONSERVERMSGTYPE_CHALLENGE = 1;
static u8k RCONSERVERMSGTYPE_AUTHORIZED = 2;
static u8k RCONSERVERMSGTYPE_AUTHFAILED = 3;
static u8k RCONSERVERMSGTYPE_RCONRESULT = 4;
static u8k RCONSERVERMSGTYPE_AUTHTIMEOUT = 5;

static u8k RCONCLIENTMSGTYPE_MD5DIGEST = 0;
static u8k RCONCLIENTMSGTYPE_RCONCOMMAND = 1;

static u32k RCON_MAGIC = 0x18181818;

/* cross-platform padding issue.

   SRCONMessageHdr contains variables which aren't aligned to machine word size.
   It is base structure and all RCON network messages are derived from it.

   Since base structure isn't aligned to machine word size so there is
   different memory organization in case of MS VC and GCC under Linux.

   In case of MS VC compiler memory is aligned as follows:
   [SRCONMessageHdr][00 00 00][derived structure]

   While in case of GCC under Linux:
   [SRCONMessageHdr][derived structure][00 00 00]

   So in order to fix it we will force align base structure to have the same
   memory structure under both platforms. */
#pragma pack(push,1)

struct SRCONMessageHdr
{
	SRCONMessageHdr() : magic(0), messageType(255) {}
	SRCONMessageHdr(u32 m, u8 t) : magic(m), messageType(t) {}
	u32 magic; // to filter out accidental erroneous connections
	u8  messageType;
};

struct SRCONServerMsgChallenge : public SRCONMessageHdr
{
	SRCONServerMsgChallenge() : SRCONMessageHdr(RCON_MAGIC, RCONSERVERMSGTYPE_CHALLENGE) {}
	u8 challenge[16]; // 16 bytes array
};

struct SRCONServerMsgInSession : public SRCONMessageHdr
{
	SRCONServerMsgInSession() : SRCONMessageHdr(RCON_MAGIC, RCONSERVERMSGTYPE_INSESSION) {}
};

struct SRCONServerMsgAuthorized : public SRCONMessageHdr
{
	SRCONServerMsgAuthorized() : SRCONMessageHdr(RCON_MAGIC, RCONSERVERMSGTYPE_AUTHORIZED) {}
};

struct SRCONServerMsgAuthFailed : public SRCONMessageHdr
{
	SRCONServerMsgAuthFailed() : SRCONMessageHdr(RCON_MAGIC, RCONSERVERMSGTYPE_AUTHFAILED) {}
};

struct SRCONServerMsgRConResult : public SRCONMessageHdr
{
	SRCONServerMsgRConResult() : SRCONMessageHdr(RCON_MAGIC, RCONSERVERMSGTYPE_RCONRESULT), commandId(0), resultLen(0) {}
	u32 commandId;
	u32 resultLen; // NOTE: this message is handled on the client side, so security considerations are not that strict as for the server
	//u8 result[4080]; // null terminated string
};

struct SRCONServerMsgAuthTimeout : public SRCONMessageHdr
{
	SRCONServerMsgAuthTimeout() : SRCONMessageHdr(RCON_MAGIC, RCONSERVERMSGTYPE_AUTHTIMEOUT) {}
};

struct SRCONClientMsgMD5Digest : public SRCONMessageHdr
{
	SRCONClientMsgMD5Digest() : SRCONMessageHdr(RCON_MAGIC, RCONCLIENTMSGTYPE_MD5DIGEST) {}
	u8 digest[CWhirlpoolHash::DIGESTBYTES]; // 16 bytes array
};

struct SRCONClientMsgRConCommand : public SRCONMessageHdr
{
	SRCONClientMsgRConCommand() : SRCONMessageHdr(RCON_MAGIC, RCONCLIENTMSGTYPE_RCONCOMMAND), commandId(0) { memset(command, 0, sizeof(command)); }
	u32 commandId;
	u8  command[256]; // null terminated string
};

#pragma pack(pop)

#endif
