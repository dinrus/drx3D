// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __LOGINMANAGER_H__
#define __LOGINMANAGER_H__

/*************************************************************************
-------------------------------------------------------------------------
Описание:
	- Game side Upr to hide away login/Authentication Implementation
	  specifics, whilst exposing relevant data-hooks/events (e.g. Account details 
	  / DOB/ Show Terms of service/ handle login error)
-------------------------------------------------------------------------
История:
	- 10/02/2012 : Created by Jonathan Bunner and Colin Gulliver

*************************************************************************/

#include <drx3D/Game/ICrysis3Lobby.h>

//-------------------------------------------------------------------------

#define BLAZE_PERSONA_MAX_LENGTH 16
#define BLAZE_PERSONA_MIN_LENGTH 4
#define BLAZE_PERSONA_VALID_CHARACTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_ -"

//-------------------------------------------------------------------------

struct IGameWarningsListener;

class CLoginUpr : public ICrysis3AuthenticationHandler, public IGameWarningsListener
{
public:
	enum ELocalFieldValidateResult
	{
		eLFVR_Unavailable = 0,
		eLFVR_Valid,
		eLFVR_Empty,
		eLFVR_TooLong,
		eLFVR_TooShort,
		eLFVR_InvalidChars,
		eLFVR_TooFewDigits,
		eLFVR_TooFewLowercase,
		eLFVR_TooFewUppercase
	};

	enum ELogoutAction // Ordered by priority, highest last
	{
		eLA_default = 0,
		eLA_restartLogin,
		eLA_switchToSP,
	};

public:

	CLoginUpr(ICrysis3Lobby* pCrysis3Lobby); 
	~CLoginUpr();

	void StartLoginProcess(bool bDriveMenu, bool bSwitchGameTypeWhenSignedIn);

	EOnlineState GetOnlineState(u32 user) const { return (user == m_currentUser) ? m_currentState : eOS_Unknown; }
	bool LoginProcessInProgress() const { return (m_taskId != DrxLobbyInvalidTaskID); }
	bool IsSwitchingToMultiplayer() const { return AreAnyFlagsSet(eLF_SwitchGameTypeWhenSignedIn); }

	// ICrysis3AuthenticationHandler
	virtual void OnDisplayCountrySelect();
	virtual void OnDisplayLegalDocs(tukk pTermsOfServiceText, tukk pPrivacyText);
	virtual void OnDisplayEntitleGame();

	virtual void OnDisplayCreateAccount();
	virtual void OnDisplayLogin();
	virtual void OnDisplayPersonas(const SPersona *pPersonas, i32k numPersonas);
	virtual void OnPasswordRequired();

	virtual void OnCreateAccountError(EAuthenticationError errorCode, const SValidationError *pErrors, i32k numErrors);
	virtual void OnCreatePersonaError(EAuthenticationError error);
	virtual void OnForgotPasswordError(EAuthenticationError error);
	virtual void OnAssociateAccountError(EAuthenticationError error);
	virtual void OnGeneralError(EAuthenticationError error);
	virtual void OnLoginFailure(EAuthenticationError error);

	// Xbox only
	virtual void OnProfileSelected();
	virtual void OnProfileLoaded(EAuthenticationError errorCode); // error can also be err_ok.. so log as 'result' and only error really if errorCode != err_ok
	virtual void OnProfileUnloaded(EAuthenticationError errorCode);
	// ~ICrysis3AuthenticationHandler
	
	// IGameWarningsListener
	virtual bool OnWarningReturn( THUDWarningId id, tukk pReturnValue );
	virtual void OnWarningRemoved( THUDWarningId id ) { };
	// ~IGameWarningsListener

	void OnCreateAccountRequested();
	void OnDisplayCountrySelectResponse(const SDisplayCountrySelectResponse& response);
	void OnDisplayLegalDocsResponse(bool bAccepted);
	void BeginAssociateAccount(const SAssociateAccountResponse& response);
	void OnDisplayCreateAccountResponse(const SCreateAccountResponse& response);
	void OnDisplayLoginResponse(const SDisplayLoginResponse& response);
	void OnDisplayEntitleGameResponse(const SDisplayEntitleGameResponse& response);
	void OnDisplayPersonasResponse(const SDisplayPersonasResponse& response);
	void OnPasswordRequiredResponse(const SPasswordRequiredResponse& response);
	void CreatePersona(const SDisplayPersonasResponse& response);
	void Back();
	void Logout(ELogoutAction action=eLA_default, bool bDriveMenu = true);
	void ForgotPassword(tukk pEmail);

	void StoreLoginDetails(tukk pEmail, tukk pPassword, const bool bRememberCheck);
	SPasswordRules* GetPasswordRules() { return (AreAnyFlagsSet(eLF_HasPasswordRules) ? &m_passwordRules : NULL); }

	ELocalFieldValidateResult ValidatePersona(tukk pPersona, i32* pOutRequired=NULL);
	ELocalFieldValidateResult ValidatePassword(tukk pPassword, i32* pOutRequired=NULL);
	tukk GetValidationError(ELocalFieldValidateResult validationResult, tukk pFieldName, i32k required);
	static bool ValidateEmail(tukk pEmail);

	void PIIOptInScreenClose ();

#ifndef _RELEASE
	tukk Debug_GetEmail();
	tukk Debug_GetPassword();
	tukk Debug_GetPersona();
#endif

private:

#if !defined(_RELEASE) && !defined(DEDICATED_SERVER)
	static void CmdStartLoginProcess(IConsoleCmdArgs* pCmdArgs);
	static void CmdReprompt(IConsoleCmdArgs *pArgs);
	static void CmdChangeEmail(IConsoleCmdArgs *pArgs);
	static void CmdChangePassword(IConsoleCmdArgs *pArgs);

	void InitDebugVars();
	void ReleaseDebugVars();
#endif

	static void Crysis3LobbyPasswordCallback( DrxLobbyTaskID taskID, EDrxLobbyError error, SPasswordRules *pRules, uk pArg );
	static void OnlineStateChangedCallback(UDrxLobbyEventData eventData, uk pArg);
	static void LoginProcessCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg);
	static void LogoutProcessCallback(DrxLobbyTaskID taskID, EDrxLobbyError error, uk pArg);

	bool Menu_GoToPage(tukk pPage);
	void Menu_GoBack();
	void Internal_StartLoginProcess();
	void FinishLogin();

	bool HandleAuthenticationErrorWarning(const EAuthenticationError error);

	SPasswordRules m_passwordRules;

	DrxFixedStringT<IC3_MAX_EMAIL_LENGTH> m_forgotPasswordEmail;
	DrxFixedStringT<IC3_MAX_EMAIL_LENGTH> m_loginEmail;
	DrxFixedStringT<IC3_MAX_PASSWORD_LENGTH> m_loginPassword;

	ICrysis3Lobby* m_pCrysis3Lobby;
	DrxLobbyTaskID m_taskId;
	DrxLobbyTaskID m_logoutTaskId;
	EOnlineState m_currentState;
	ELogoutAction m_logoutAction;
	DrxUserID m_lastSignedInUser;

	u32 m_currentUser;
	
	enum ELoginFlags
	{
		eLF_RememberDetails							= BIT(0),
		eLF_HasPasswordRules						= BIT(1),
		eLF_DriveMenu										= BIT(2),
		eLF_NeedToLogin									= BIT(3),
		eLF_SwitchGameTypeWhenSignedIn	= BIT(4),
		eLF_DeclinedTOS									= BIT(5),
		eLF_IsCreatingAccount						= BIT(6),
	};

	typedef u8 TLoginFlagsType;

	void AddFlag(ELoginFlags flag) { m_flags |= (TLoginFlagsType)flag; }
	void RemoveFlag(ELoginFlags flag) { m_flags &= ~(TLoginFlagsType)flag; }
	void SetFlag(ELoginFlags flag, bool bAdd) { if (bAdd) { AddFlag(flag); } else { RemoveFlag(flag); } }
	bool AreAnyFlagsSet(ELoginFlags flags) const { return (m_flags & (TLoginFlagsType)flags) != 0; }

	TLoginFlagsType m_flags;

	// Statics
	static tukk const	s_pEmailLegal;
	static tukk const	s_pEmailPartEnd;
};


#endif // __LOGINMANAGER_H__
