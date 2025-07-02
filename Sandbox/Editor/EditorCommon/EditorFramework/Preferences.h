// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.


// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <drx3D/Sandbox/Editor/EditorCommon/AutoRegister.h>
#include <drx3D/Sandbox/Editor/EditorCommon/QtViewPane.h>

#include "PreferencesDialog.h"

#include <drx3D/CoreX/Serialization/Enum.h>
#include <drx3D/CoreX/String/DrxString.h>
#include <drx3D/CoreX/Sandbox/DrxSignal.h>

class CPreferences;

typedef CAutoRegister<CPreferences> CAutoRegisterPreferencesHelper;

#define REGISTER_PREFERENCES_PAGE(Type)                                                               \
  namespace Internal                                                                                  \
  {                                                                                                   \
  void RegisterPreferencesPage ## Type()                                                              \
  {                                                                                                   \
    GetIEditor()->GetPreferences()->RegisterPage<Type>();                                             \
  }                                                                                                   \
  CAutoRegisterPreferencesHelper g_AutoRegPreferencesHelper ## Type(RegisterPreferencesPage ## Type); \
  }

#define REGISTER_PREFERENCES_PAGE_PTR(Type, TypePtr)                                                  \
  namespace Internal                                                                                  \
  {                                                                                                   \
  void RegisterPreferencesPage ## Type()                                                              \
  {                                                                                                   \
    GetIEditor()->GetPreferences()->RegisterPage<Type>(TypePtr);                                      \
  }                                                                                                   \
  CAutoRegisterPreferencesHelper g_AutoRegPreferencesHelper ## Type(RegisterPreferencesPage ## Type); \
  }

#define ADD_PREFERENCE_PAGE_PROPERTY(type, accessor, mutator) \
  public:                                                     \
    void mutator(const type val)                              \
    {                                                         \
      if (m_ ## accessor != val)                              \
      {                                                       \
        m_ ## accessor = val;                                 \
        accessor ## Changed();                                \
        signalSettingsChanged();                              \
      }                                                       \
    }                                                         \
    const type& accessor() const { return m_ ## accessor; }   \
    CDrxSignal<void()> accessor ## Changed;                   \
  private:                                                    \
    type m_ ## accessor;                                      \

struct SPreferencePage;

struct EDITOR_COMMON_API SPreferencePage
{
	SPreferencePage(tukk name, tukk path)
		: m_name(name)
		, m_path(path)
	{
	}

	virtual ~SPreferencePage() {}

	const string& GetName() const     { return m_name; }
	const string& GetPath() const     { return m_path; }
	string        GetFullPath() const { return m_path + "/" + m_name; }

	virtual bool  Serialize(yasli::Archive& ar)
	{
		return true;
	}

	void operator=(const SPreferencePage& other)
	{
		if (this != &other)
		{
			m_name = other.m_name;
			m_path = other.m_path;
		}
	}

private:
	// private empty constructor to make sure we have a valid name & path
	SPreferencePage() {}

	friend class CPreferences;
	void SetName(const string& name) { m_name = name; }
	void SetPath(const string& path) { m_path = path; }

	QString GetSerializedProperties();
	void FromSerializedProperties(const QByteArray& jsonBlob);

public:
	CDrxSignal<void()> signalSettingsChanged;

private:
	CDrxSignal<void()> signalRequestReset;

	string m_name;
	string m_path;
};

class EDITOR_COMMON_API CPreferences
{
	friend yasli::Serializer;
public:
	CPreferences();
	~CPreferences();

	// Need to call init after all preferences have been registered
	void Init();

	// Tray area controls the lifetime of the widget
	template<typename Type>
	void RegisterPage()
	{
		Type* pPreferencePage = new Type();
		pPreferencePage->signalSettingsChanged.Connect(std::function<void()>([this]() { signalSettingsChanged(); }));
		pPreferencePage->signalRequestReset.Connect(std::function<void()>([pPreferencePage]()
		{
			*pPreferencePage =  Type();
			pPreferencePage->signalSettingsChanged();
		}));

		AddPage(pPreferencePage);
	}

	template<typename Type>
	void RegisterPage(Type* pPreferencePage)
	{
		pPreferencePage->signalSettingsChanged.Connect(std::function<void()>([this]() { signalSettingsChanged(); }));
		pPreferencePage->signalRequestReset.Connect(std::function<void()>([pPreferencePage]() {
			*pPreferencePage = Type();
			pPreferencePage->signalSettingsChanged();
		}));

		AddPage(pPreferencePage);
	}

	std::vector<SPreferencePage*> GetPages(tukk path);
	QWidget*                      GetPageWidget(tukk path)
	{
		return new QPreferencePage(GetPages(path), path);
	}

	const std::map<string, std::vector<SPreferencePage*>>& GetPages() const { return m_preferences; }

	bool         IsLoading() const { return m_bIsLoading; }
	void         Reset(tukk path);
	void         Save();

private:
	void         Load();
	void         Load(const QString& path);
	void         AddPage(SPreferencePage* pPreferencePage);

	virtual bool Serialize(yasli::Archive& ar);

public:
	CDrxSignal<void()> signalSettingsChanged;
	CDrxSignal<void()> signalSettingsReset;

private:
	std::map<string, std::vector<SPreferencePage*>> m_preferences;
	bool m_bIsLoading;
};

ILINE bool Serialize(yasli::Archive& ar, SPreferencePage* val, tukk name, tukk label)
{
	if (!val)
		return false;

	return ar(*val, name ? name : val->GetFullPath(), label ? label : val->GetName());
}

