// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// These are helper classes for containing the data from the generic overwrite dialog.

#ifndef UserOptions_h__
#define UserOptions_h__

#pragma once

// Small helper class.
// Hint: have one for files and other for directories.
// Hint: used a CUserOptionsReferenceCountHelper to automatically control the reference counts
// of any CUserOptions variable: usefull for recursion when you don't want to use
// only static variables. See example in FileUtill.cpp, function CopyTree.
class CUserOptions
{
	//////////////////////////////////////////////////////////////////////////
	// Types & typedefs
public:
	enum EOption
	{
		ENotSet,
		EYes    = IDYES,
		ENo     = IDNO,
		ECancel = IDCANCEL,
	};

	class CUserOptionsReferenceCountHelper
	{
	public:
		CUserOptionsReferenceCountHelper(CUserOptions& roUserOptions);
		virtual ~CUserOptionsReferenceCountHelper();
	protected:
		CUserOptions& m_roReferencedUserOptionsObject;
	};
protected:
private:
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Methods
public:
	CUserOptions();

	bool IsOptionValid();

	i32  GetOption();

	bool IsOptionToAll();

	void SetOption(i32 nNewOption, bool boToAll);

	i32  DecRef();
	i32  IncRef();
protected:
private:
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Data
public:
protected:
	i32  m_nCurrentOption;
	bool m_boToAll;
	i32  m_nNumberOfReferences;
private:
	//////////////////////////////////////////////////////////////////////////
};
#endif // UserOptions_h__

