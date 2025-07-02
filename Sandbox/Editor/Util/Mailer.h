// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __Editor_Mailer_h__
#define __Editor_Mailer_h__

#if _MSC_VER > 1000
	#pragma once
#endif

//////////////////////////////////////////////////////////////////////////
class CMailer
{
public:
	static bool SendMail(tukk _subject,                         // E-Mail Subject
	                     tukk _messageBody,                     // Message Text
	                     const std::vector<tukk >& _recipients,  // All Recipients' Addresses
	                     const std::vector<tukk >& _attachments, // All File Attachments
	                     bool bShowDialog);                            // Whether to allow editing by user
};

#endif // __Editor_Mailer_h__

