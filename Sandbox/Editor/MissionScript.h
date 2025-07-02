// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CMissionScript
{
private:
	CString              m_sFilename;
	std::vector<CString> m_methods;
	std::vector<CString> m_events;
public:
	CMissionScript();
	virtual ~CMissionScript();

	void SetScriptFile(const CString& file);
	bool Load();
	void Edit();

	//! Call on reset of mission.
	void OnReset();

	//! Get Lua filename.
	const CString& GetFilename() { return m_sFilename; }

	//////////////////////////////////////////////////////////////////////////
	i32            GetMethodCount() { return m_methods.size(); }
	const CString& GetMethod(i32 i) { return m_methods[i]; }

	//////////////////////////////////////////////////////////////////////////
	i32            GetEventCount() { return m_events.size(); }
	const CString& GetEvent(i32 i) { return m_events[i]; }
};

