// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   WarningsUpr.h
//  Version:     v1.00
//  Created:     25/6/2012 by Paul Reindell.
//  Описание: 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __WarningsUpr_H__
#define __WarningsUpr_H__

#include <drx3D/Game/UIDialogs.h>

#define INVALID_HUDWARNING_ID ((THUDWarningId)~0)


struct SWarningDefinition
{
	SWarningDefinition(tukk msgId, EDialogType type, tukk title, tukk message, tukk param, tukk positiveResonse, tukk negativeResponse, THUDWarningId id)
		: messageId(msgId)
		, diagType(type)
		, diagTitle(title)
		, diagMessage(message)
		, diagParam(param)
		, warningId(id)
	{
		response[0] = positiveResonse;
		response[1] = negativeResponse;
	}
	tukk messageId;
	EDialogType diagType;
	tukk diagTitle;
	tukk diagMessage;
	tukk diagParam;
	tukk response[2];
	THUDWarningId warningId;
};

class CWarningsUpr : public IDialogCallback
{
public:
	CWarningsUpr();
	~CWarningsUpr();

	THUDWarningId AddGameWarning(tukk stringId, tukk paramMessage = NULL, IGameWarningsListener* pListener = NULL);
	void AddGameWarning(THUDWarningId id, tukk paramMessage = NULL, IGameWarningsListener* pListener = NULL);
	void RemoveGameWarning(THUDWarningId Id);
	void RemoveGameWarning(tukk stringId);
	void CancelCallbacks(IGameWarningsListener* pListener);
	void CancelWarnings();

	THUDWarningId GetWarningId(tukk stringId) const;
	bool IsWarningActive(tukk stringId) const;
	bool IsWarningActive(THUDWarningId Id) const;

	//IDialogCallback
	virtual void DialogCallback(u32 dialogId, EDialogResponse response, tukk param);
	//~IDialogCallback

private:
	struct SGameWarning
	{
		SGameWarning() 
			: pWarningDef(NULL)
			, pListener(NULL)
			, DialogId(~0)
		{}

		const SWarningDefinition* pWarningDef;
		IGameWarningsListener *pListener;
		u32 DialogId;
	};

	SGameWarning* GetWarningForDialog(u32 dialogId);

	const SWarningDefinition* GetWarningDefinition(THUDWarningId id) const;
	const SWarningDefinition* GetWarningDefinition(tukk stringId) const;

	void AddGameWarning(const SWarningDefinition* pWarningDef, tukk paramMessage = NULL, IGameWarningsListener* pListener = NULL);
	void RemoveGameWarning(const SWarningDefinition* pWarningDef);

	CUIDialogs* GetDialogs();
private:
	typedef std::map<THUDWarningId, SGameWarning> TWarningMap;
	typedef std::vector<SWarningDefinition> TWarningDefMap;

	TWarningDefMap m_WarningDefinitions;
	TWarningMap m_Warnings;
};


#endif // #ifndef __WarningsUpr_H__

