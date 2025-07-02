// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __OPTION_H__
#define __OPTION_H__

struct IPlayerProfile;

//////////////////////////////////////////////////////////////////////////


enum EProfileOptionType
{
	ePOT_Normal = 0,
	ePOT_CVar,
	ePOT_ScreenResolution,
	ePOT_SysSpec
};


class COption
{

public:

	COption();
	COption(tukk name, tukk val);

	virtual ~COption();

	void						SetPlayerProfile(IPlayerProfile* profile);
	IPlayerProfile* GetPlayerProfile() const {return m_currentProfile; }

	const string&		GetName() const { return m_currentName; }

	virtual void		InitializeFromProfile();
	virtual void		InitializeFromCVar();
	virtual void		ResetDefault();

	virtual void		SetToProfile();

	virtual void					Set(tukk param);
	virtual const string&	Get() const;

	virtual const string&	GetDefault() const;

	virtual void		SetPreview(const bool _preview);
	virtual bool		IsPreview() const { return m_preview; }

	virtual void		SetConfirmation(const bool _confirmation);
	virtual bool		IsConfirmation() const { return m_confirmation; }

	virtual void		SetRequiresRestart(const bool _restart);
	virtual bool		IsRequiresRestart() const { return m_restart; }

	virtual void		SetWriteToConfig(const bool write);
	virtual bool		IsWriteToConfig() const { return m_writeToConfig; }
	virtual void		GetWriteToConfigString(DrxFixedStringT<128> &outString, ICVar* pCVar, tukk param) const {}

	virtual EProfileOptionType GetType() { return ePOT_Normal; }

protected:

	string					m_currentName;
	string					m_currentValue;
	string					m_defaultValue;
	IPlayerProfile* m_currentProfile;

private:

	bool						m_preview;
	bool						m_confirmation;
	bool						m_restart;
	bool						m_writeToConfig;

};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CCVarOption : public COption
{

public:

	CCVarOption();
	CCVarOption(tukk name, tukk val, tukk cvarName);

	virtual ~CCVarOption();

	virtual EProfileOptionType GetType() { return ePOT_CVar; }

	virtual void		InitializeFromProfile();
	virtual void		InitializeFromCVar();

#if DRX_PLATFORM_WINDOWS
	virtual void		SetToProfile();
#endif

	virtual void		Set(tukk param);

	const string&		GetCVar() { return m_cVar; }
	virtual void		GetWriteToConfigString(DrxFixedStringT<128> &outString, ICVar* pCVar, tukk param) const;

protected:

	string		m_cVar;

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


class CScreenResolutionOption : public COption
{

public:

	CScreenResolutionOption();
	CScreenResolutionOption(tukk name, tukk val);

	virtual ~CScreenResolutionOption();

	virtual EProfileOptionType GetType() { return ePOT_ScreenResolution; }

	virtual void		InitializeFromProfile();
	virtual void		InitializeFromCVar();

	virtual void		Set(tukk param);
	virtual void		GetWriteToConfigString(DrxFixedStringT<128> &outString, ICVar* pCVar, tukk param) const;

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


class CSysSpecOption : public CCVarOption
{

public:

	CSysSpecOption();
	CSysSpecOption(tukk name, tukk val, tukk cvarName);
	virtual ~CSysSpecOption();

	virtual EProfileOptionType	GetType() { return ePOT_SysSpec; }
	virtual const string&				Get() const;
	virtual void								Set(tukk param);


	virtual void								InitializeFromProfile();
	virtual bool								IsValid() const;

protected:

	static string s_customEntry;

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


class CSysSpecAllOption : public CSysSpecOption
{

public:

	CSysSpecAllOption();
	CSysSpecAllOption(tukk name, tukk val, tukk cvarName);
	virtual ~CSysSpecAllOption();

	virtual void								InitializeFromProfile();
	virtual bool								IsValid() const;

};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


class CNotSavedOption : public CCVarOption
{

public:

	CNotSavedOption();
	CNotSavedOption(tukk name, tukk val, tukk cvarName);
	virtual ~CNotSavedOption();

	virtual void		InitializeFromProfile();
	virtual void		InitializeFromCVar();

	virtual void		SetToProfile();

	virtual void		Set(tukk param);
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


class CAutoResetOption : public CCVarOption
{

public:

	CAutoResetOption();
	CAutoResetOption(tukk name, tukk val);

	virtual ~CAutoResetOption();

	virtual void		InitializeFromProfile();
};

//////////////////////////////////////////////////////////////////////////

#endif

