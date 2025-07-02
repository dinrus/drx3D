// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include "Mailer.h"

#if DRX_PLATFORM_WINDOWS

	#include <mapi.h>
	#include "Mailer.h"

bool CMailer::SendMail(tukk subject,
                       tukk messageBody,
                       const std::vector<tukk >& _recipients,
                       const std::vector<tukk >& _attachments,
                       bool bShowDialog)
{
	// Preserve directory, (Can be changed if attachment specified)
	WCHAR dir[MAX_PATH];
	GetCurrentDirectoryW(sizeof(dir), dir);

	// Load MAPI dll
	HMODULE hMAPILib = LoadLibrary("MAPI32.DLL");
	LPMAPISENDMAIL lpfnMAPISendMail = (LPMAPISENDMAIL) GetProcAddress(hMAPILib, "MAPISendMail");

	i32 numRecipients = (i32)_recipients.size();

	// Handle Attachments
	MapiFileDesc* attachments = new MapiFileDesc[_attachments.size()];

	i32 i = 0;
	for (u32 k = 0; k < _attachments.size(); k++)
	{
		FILE* file = fopen(_attachments[k], "r");
		if (!file)
			continue;
		fclose(file);

		attachments[i].ulReserved = 0;
		attachments[i].flFlags = 0;
		attachments[i].nPosition = (ULONG)-1;
		attachments[i].lpszPathName = (tuk)(tukk)_attachments[k];
		attachments[i].lpszFileName = NULL;
		attachments[i].lpFileType = NULL;
		i++;
	}
	i32 numAttachments = i;

	// Handle Recipients
	MapiRecipDesc* recipients = new MapiRecipDesc[numRecipients];

	std::vector<string> addresses;
	addresses.resize(numRecipients);
	for (i = 0; i < numRecipients; i++)
	{
		addresses[i] = string("SMTP:") + _recipients[i];
	}

	for (i = 0; i < numRecipients; i++)
	{
		recipients[i].ulReserved = 0;
		recipients[i].ulRecipClass = MAPI_TO;
		recipients[i].lpszName = (tuk)(tukk)_recipients[i];
		recipients[i].lpszAddress = (tuk)addresses[i].c_str();
		recipients[i].ulEIDSize = 0;
		recipients[i].lpEntryID = NULL;
	}

	MapiMessage message;
	memset(&message, 0, sizeof(message));
	message.lpszSubject = (tuk)(tukk)subject;
	message.lpszNoteText = (tuk)(tukk)messageBody;
	message.lpszMessageType = NULL;

	message.nRecipCount = numRecipients;
	message.lpRecips = recipients;

	message.nFileCount = numAttachments;
	message.lpFiles = attachments;

	//Next, the client calls the MAPISendMail function and stores the return status so it can detect whether the call succeeded. You should use a more sophisticated error reporting mechanism than the C library function printf.

	FLAGS flags = bShowDialog ? MAPI_DIALOG : 0;
	flags |= MAPI_LOGON_UI;
	ULONG err = (*lpfnMAPISendMail)(0L,       // use implicit session.
	                                0L,       // ulUIParam; 0 is always valid
	                                &message, // the message being sent
	                                flags,    // if user allowed to edit the message
	                                0L);      // reserved; must be 0
	delete[] attachments;
	delete[] recipients;

	FreeLibrary(hMAPILib);   // Free DLL module through handle

	// Restore previous directory.
	SetCurrentDirectoryW(dir);

	if (err != SUCCESS_SUCCESS)
		return false;

	return true;
}

#else // DRX_PLATFORM_WINDOWS

bool CMailer::SendMail(tukk subject,
                       tukk messageBody,
                       const std::vector<tukk >& _recipients,
                       const std::vector<tukk >& _attachments,
                       bool bShowDialog)
{
	return true;
}

#endif // DRX_PLATFORM_WINDOWS

