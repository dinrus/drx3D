// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace Config
{
struct IConfigVar;
class CConfigGroup;
}

class CConfigPanel : public CDialog
	                   , public IEditorNotifyListener
{
public:
	static u32k kFirstID = 10000;
	static u32k kLastID = 10100;

	static u32k kMarginTop = 10;
	static u32k kMarginLeft = 10;
	static u32k kMarginRight = 10;
	static u32k kControlSeparation = 3;
	static u32k kGroupSeparation = 5;
	static u32k kGroupInnerTopMargin = 20;
	static u32k kGroupInnerBottomMargin = 10;
	static u32k kGroupInnerLeftMargin = 10;
	static u32k kGroupInnerRightMargin = 10;

public:
	enum { IDD = IDD_CONFIG_PANEL };

	CConfigPanel(UINT idd = IDD, CWnd* pParent = NULL, uint initialY = 0);   // standard constructor
	virtual ~CConfigPanel();

	// Create edit controls for given configuration group
	void DisplayGroup(Config::CConfigGroup* pGroup, tukk szGroupName);

	// Called after value of given config variable was changed
	virtual void OnConfigValueChanged(Config::IConfigVar* pVar) {};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnOK()     {};
	virtual void OnCancel() {};
	afx_msg void OnOptionChanged(UINT nID);
	afx_msg void OnTextChanged(UINT nID);
	afx_msg void OnSize(UINT nType, i32 cx, i32 cy);

protected:
	// IEditorNotifyListener interface implementation
	virtual void OnEditorNotifyEvent(EEditorNotifyEvent event);

	struct CItem
	{
		CWnd*               m_pWnd;
		Config::IConfigVar* m_pVar;
		DWORD               m_top;
		DWORD               m_bottom;

		CItem(Config::IConfigVar* pVar, CWnd* pWnd, const RECT& rect);
		~CItem();
	};

	struct CGroup
	{
		CWnd*               m_pGroup;
		DWORD               m_top;
		DWORD               m_bottom;
		std::vector<CItem*> m_items;

		CGroup(CWnd* pWnd, const RECT& rect);
		~CGroup();
	};

	typedef std::vector<CGroup*> TGroups;
	TGroups m_groups;

	typedef std::map<DWORD, CItem*> TItemMap;
	TItemMap m_items;

	uint     m_initialY;
	uint     m_currentPosition;
	uint     m_currentId;
};

