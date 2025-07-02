// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// This is the header file for importing background objects into Mannequin.
// The recomended way to call this dialog is through DoModal() method.

#ifndef MannImportBackgroundDialogDialog_h__
#define MannImportBackgroundDialogDialog_h__

class CMannImportBackgroundDialog : public CDialog
{
	//////////////////////////////////////////////////////////////////////////
	// Methods
public:
	CMannImportBackgroundDialog(std::vector<CString>& loadedObjects);

	void DoDataExchange(CDataExchange* pDX);
	BOOL OnInitDialog();
	void OnOK();

	i32  GetCurrentRoot() const
	{
		return m_selectedRoot;
	}

protected:
	const std::vector<CString>& m_loadedObjects;
	CComboBox                   m_comboRoot;
	i32                         m_selectedRoot;
};

#endif // StringInputDialog_h__

