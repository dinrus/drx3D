// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2011.
// -------------------------------------------------------------------------
//  File name:   LensFlareUndo.h
//  Created:     12/Dec/2012 by Jaesik.
////////////////////////////////////////////////////////////////////////////

class CLensFlareItem;

class CUndoLensFlareItem : public IUndoObject
{
public:
	CUndoLensFlareItem(CLensFlareItem* pGroupItem, tukk undoDescription = "Undo Lens Flare Tree");
	~CUndoLensFlareItem();

protected:
	tukk GetDescription() { return m_undoDescription; };
	void        Undo(bool bUndo);
	void        Redo();

private:

	string m_undoDescription;

	struct SData
	{
		SData()
		{
			m_pOptics = NULL;
		}

		string               m_selectedFlareItemName;
		bool                  m_bRestoreSelectInfo;
		IOpticsElementBasePtr m_pOptics;
	};

	string m_flarePathName;
	SData   m_Undo;
	SData   m_Redo;

	void Restore(const SData& data);
};

class CUndoRenameLensFlareItem : public IUndoObject
{
public:

	CUndoRenameLensFlareItem(tukk oldFullName, tukk newFullName, bool bRefreshItemTreeWhenUndo = false, bool bRefreshItemTreeWhenRedo = false);

protected:
	tukk GetDescription() { return m_undoDescription; };
	void        Undo(bool bUndo);
	void        Redo();

private:

	string m_undoDescription;

	struct SUndoDataStruct
	{
		string m_oldFullItemName;
		string m_newFullItemName;
		bool    m_bRefreshItemTreeWhenUndo;
		bool    m_bRefreshItemTreeWhenRedo;
	};

	void Rename(const SUndoDataStruct& data, bool bRefreshItemTree);

	SUndoDataStruct m_undo;
	SUndoDataStruct m_redo;
};

class CUndoLensFlareElementSelection : public IUndoObject
{
public:
	CUndoLensFlareElementSelection(CLensFlareItem* pLensFlareItem, tukk flareTreeItemFullName, tukk undoDescription = "Undo Lens Flare Element Tree");
	~CUndoLensFlareElementSelection(){}

protected:
	tukk GetDescription() { return m_undoDescription; };
	void        Undo(bool bUndo);
	void        Redo();

private:

	string m_undoDescription;

	string m_flarePathNameForUndo;
	string m_flareTreeItemFullNameForUndo;

	string m_flarePathNameForRedo;
	string m_flareTreeItemFullNameForRedo;
};

class CUndoLensFlareItemSelectionChange : public IUndoObject
{
public:
	CUndoLensFlareItemSelectionChange(tukk fullLensFlareItemName, tukk undoDescription = "Undo Lens Flare element selection");
	~CUndoLensFlareItemSelectionChange(){}

protected:
	tukk GetDescription() { return m_undoDescription; };

	void        Undo(bool bUndo);
	void        Redo();

private:
	string m_undoDescription;
	string m_FullLensFlareItemNameForUndo;
	string m_FullLensFlareItemNameForRedo;
};

