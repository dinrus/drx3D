// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PROFILEOPTIONS_H__
#define __PROFILEOPTIONS_H__

#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Game/Option.h>

//////////////////////////////////////////////////////////////////////////
#include <drx3D/Act/IPlayerProfiles.h>

class CProfileOptions : public IPlayerProfileListener
{
public:

	CProfileOptions();
	virtual		~CProfileOptions();

	struct SPendingOption
	{
		DrxFixedStringT<64> command;
		DrxFixedStringT<512> param;
		DrxFixedStringT<512> original;
		bool								preview;
		bool								confirmation;
		bool								restart;
		bool								writeToConfig;

		SPendingOption(tukk _command, tukk _param, tukk _original, bool _preview, bool _confirmation, bool _restart, bool _write):command(_command), param(_param), original(_original), preview(_preview), confirmation(_confirmation), restart(_restart), writeToConfig(_write)
		{}
	};

	void			Init();
	void			InitializeFromCVar();

	void			SaveProfile(u32 reason = ePR_Options);
	COption*		GetOption(tukk option) const;
	COption*		GetOptionByCVar(tukk cvar) const;
	bool			IsOption(tukk option);

	string			GetOptionValue(tukk command, bool getDefault=false);
	string			GetOptionValueOrCreate(tukk command, bool getDefault=false);
	void			SetOptionValue(tukk command, tukk param, bool toPendingOptions = false);
	void			SetOptionValue(tukk command, i32 value, bool toPendingOptions = false);
	void			SetOptionValue(tukk command, float value, bool toPendingOptions = false);

	i32				GetOptionValueAsInt(tukk command, bool getDefault=false);
	float			GetOptionValueAsFloat(tukk command, bool getDefault=false);

	string			GetPendingOptionValue(tukk command);
	bool			HasPendingOptionValues()const;
	bool			HasPendingOptionValue(tukk option)const;
	bool			HasPendingOptionValuesWithConfirmation()const;
	bool			HasPendingOptionValuesWithRequiresRestart() const;
	void			ApplyPendingOptionsValuesForConfirmation();
	void			FlushPendingOptionValues();
	void			ClearPendingOptionValues();
	void			ClearPendingOptionValuesFromConfirmation();

	void			ResetToDefault();

	const std::vector<SPendingOption>& GetPendingOptions() { return m_pendingOptions; }

	//IOnlinePlayerProfileListener
	void			SaveToProfile(IPlayerProfile* pProfile, bool online, u32 reason);
	void			LoadFromProfile(IPlayerProfile* pProfile, bool online, u32 reason);
	//~IOnlinePlayerProfileListener

	bool			ShouldDeferredMPSave() const { return m_bDeferredSaveMP; }
	void			SetDeferredMPSave(const bool deferredSave) { m_bDeferredSaveMP = deferredSave; }

	void			ResetOverscanBorders();

	void			SetInitialProfileLoaded(const bool bLoaded) { m_bInitialProfileLoaded = bLoaded; }
	ILINE bool		IsInitialProfileLoaded() const { return m_bInitialProfileLoaded; }

private:

	struct CCVarSink : public ICVarDumpSink
	{
		CCVarSink(CProfileOptions* pOptions, FILE* pFile)
		{
			m_pOptions = pOptions;
			m_pFile = pFile;
		}

		void OnElementFound(ICVar *pCVar);
		CProfileOptions* m_pOptions;
		FILE* m_pFile;
	};

	bool IsCVar(tukk cVarName);
	// Suppress passedByValue for smart pointers like XmlNodeRef
	// cppcheck-suppress passedByValue
	void AddOption(const XmlNodeRef node);
	void AddOption(tukk name, tukk value, tukk cvar = NULL, const bool preview = false, const bool confirmation = false, const bool restart = false, const bool writeToConfig = false);

	bool IsAdvancedOption(tukk command)const;
	void AddOrReplacePendingOption(tukk command, tukk option);

	bool WriteGameCfg();

	std::vector<SPendingOption> m_pendingOptions;
	std::vector<COption*> m_allOptions;

	bool m_bDeferredSaveMP;
	bool m_bInitialProfileLoaded;
	bool m_bLoadingProfile;
};


//////////////////////////////////////////////////////////////////////////

#endif

