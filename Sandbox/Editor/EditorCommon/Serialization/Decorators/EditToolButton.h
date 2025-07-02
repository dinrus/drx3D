// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//#include "stdafx.h"
#include <drx3D/CoreX/Serialization/IArchive.h>
#include "Serialization/PropertyTree/IDrawContext.h"
#include "Serialization/PropertyTree/PropertyTree.h"

#include "IEditor.h"
#include "IEditorClassFactory.h"
#include "EditTool.h"

namespace Serialization
{
struct SEditToolButton;

DECLARE_SHARED_POINTERS(SEditToolButton)

// Serialization for edit tool buttons. This supports standard edit tool button features such as current tool highlight
struct SEditToolButton
{
	string         icon;
	// Tool associated with this button.
	CRuntimeClass* m_toolClass;
	string         m_userDataKey;
	bool           m_bNeedDocument;
	i32            buttonflags;
	uk          m_userData;
	PropertyTree*  tree;
	CEditTool*     m_activeTool;

	explicit SEditToolButton(tukk icon)
		: icon(icon)
		, m_bNeedDocument(false)
		, m_toolClass(nullptr)
		, buttonflags(0)
		, tree(nullptr)
		, m_activeTool(nullptr)
	{
	}

	virtual tukk Icon() const
	{
		return icon.c_str();
	}

	virtual SEditToolButtonPtr Clone() const
	{
		SEditToolButton* editToolButton = new SEditToolButton(icon.c_str());
		// this could be trouble later if data are not meant to be shared
		* editToolButton = *this;
		return SEditToolButtonPtr(editToolButton);
	}

	void SetToolName(const string& sEditToolName, tukk userDataKey, uk userData)
	{
		IClassDesc* pClass = GetIEditor()->GetClassFactory()->FindClass(sEditToolName);
		if (!pClass)
		{
			return;
		}
		if (pClass->SystemClassID() != ESYSTEM_CLASS_EDITTOOL)
		{
			return;
		}
		CRuntimeClass* pRtClass = pClass->GetRuntimeClass();
		if (!pRtClass || !pRtClass->IsDerivedFrom(RUNTIME_CLASS(CEditTool)))
		{
			return;
		}
		m_toolClass = pRtClass;

		m_userData = userData;
		if (userDataKey)
			m_userDataKey = userDataKey;

		// Now we can determine if we can be checked or not, since we have a valid tool class
		DetermineCheckedState();
	}

	void SetToolClass(CRuntimeClass* toolClass, tukk userDataKey = nullptr, uk userData = nullptr)
	{
		m_toolClass = toolClass;

		m_userData = userData;
		if (userDataKey)
			m_userDataKey = userDataKey;

		// Now we can determine if we can be checked or not, since we have a valid tool class
		DetermineCheckedState();
	}

	void DetermineCheckedState()
	{
		if (!m_toolClass)
		{
			buttonflags &= ~BUTTON_PRESSED;
			buttonflags |= BUTTON_DISABLED;
			return;
		}

		// Check tool state.
		CEditTool* tool = GetIEditor()->GetEditTool();
		CRuntimeClass* toolClass = 0;
		if (tool)
			toolClass = tool->GetRuntimeClass();

		buttonflags &= ~BUTTON_DISABLED;

		if (toolClass != m_toolClass)
		{
			if (buttonflags & BUTTON_PRESSED)
			{
				buttonflags &= ~BUTTON_PRESSED;
				if (tree)
					tree->repaint();
			}
		}
		else
		{
			if (!(buttonflags & BUTTON_PRESSED))
			{
				buttonflags |= BUTTON_PRESSED;
				if (tree)
					tree->repaint();
			}
		}
	}

	// workaround for propertytree not being available at creation time
	void SetPropertyTree(PropertyTree* ptree)
	{
		tree = ptree;
	}

	void OnClicked()
	{
		if (!m_toolClass || (m_bNeedDocument && !GetIEditor()->IsDocumentReady()))
		{
			buttonflags &= ~BUTTON_PRESSED;
			return;
		}

		if (buttonflags & BUTTON_PRESSED)
		{
			CEditTool* tool = GetIEditor()->GetEditTool();
			if (m_activeTool && tool == m_activeTool)
			{
				m_activeTool = nullptr;
				GetIEditor()->SetEditTool(m_activeTool);
			}
			return;
		}
		else
		{
			m_activeTool = (CEditTool*)m_toolClass->CreateObject();
			if (!m_activeTool)
				return;

			if (m_userData)
				m_activeTool->SetUserData(m_userDataKey, m_userData);

			// Must be last function, can delete this.
			GetIEditor()->SetEditTool(m_activeTool);
		}
	}

	i32 GetButtonFlags()
	{
		return buttonflags;
	}
};

inline bool Serialize(Serialization::IArchive& ar, SEditToolButton& button, tukk name, tukk label)
{
	if (ar.isEdit())
		return ar(Serialization::SStruct::forEdit(button), name, label);
	else
		return false;
}

inline SEditToolButton EditToolButton(tukk icon = "")
{
	return SEditToolButton(icon);
}

}

