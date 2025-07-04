// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "IDrxGraphEditor.h"
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include <drx3D/CoreX/Sandbox/DrxSignal.h>

#include <QObject>
#include <QGraphicsProxyWidget>

namespace DrxGraphEditor {

class CNodeGraphView;

class EDITOR_COMMON_API CNodeGraphViewGraphicsWidget : public QGraphicsWidget
{
	Q_OBJECT

public:
	enum { Type = UserType + 1 };
	virtual i32 type() const final { return Type; }

	CNodeGraphViewGraphicsWidget(CNodeGraphView& view);
	virtual ~CNodeGraphViewGraphicsWidget();

	// Never ever use QGraphicsQidget::deleteLater() or delete a widget direclty, always use this function instead.
	virtual void                             DeleteLater();

	virtual i32                            GetType() const { return eGraphViewWidgetType_Unset; }
	virtual CAbstractNodeGraphViewModelItem* GetAbstractItem() const = 0;

	virtual void                             OnItemInvalidated();

	CNodeGraphView& GetView() const { return m_view; }

	bool            IsSameView(CNodeGraphViewGraphicsWidget* pGrahicsItem) const;
	bool            IsView(QObject* pObject) const;

	bool            IsSelected() const    { return m_isSelected; }
	void            SetSelected(bool isSelected);
	bool            IsHighlighted() const { return m_isHighlighted; }
	void            SetHighlighted(bool isHighlighted);
	bool            IsDeactivated() const { return m_isDeactivated; }
	void            SetDeactivated(bool isDeactivated);

	qreal           GetDefaultZValue() const       { return m_defaultZValue; }
	void            SetDefaultZValue(qreal zValue) { m_defaultZValue = zValue; setZValue(zValue); }

	void            ResetZValue()                  { setZValue(m_defaultZValue); }

	//
	template<typename T>
	static T* Cast(QGraphicsItem* pGraphicsItem);

	template<typename T>
	static T* Cast(CNodeGraphViewGraphicsWidget* pViewWidget);

	template<typename T>
	T* Cast();

public:
	CDrxSignal<void(bool isSelected)>    SignalSelectionChanged;
	CDrxSignal<void(bool isHighlighted)> SignalHighlightedChanged;
	CDrxSignal<void(bool isDeactivated)> SignalDeactivatedChanged;

protected:
	void OnItemPositionChanged();
	void OnItemDeactivatedChanged();

private:
	CNodeGraphView& m_view;

	qreal           m_defaultZValue;

	bool            m_isSelected    : 1;
	bool            m_isHighlighted : 1;
	bool            m_isDeactivated : 1;
};

template<typename T>
inline T* CNodeGraphViewGraphicsWidget::Cast(QGraphicsItem* pGraphicsItem)
{
	CNodeGraphViewGraphicsWidget* pViewWidget = qgraphicsitem_cast<CNodeGraphViewGraphicsWidget*>(pGraphicsItem);
	return CNodeGraphViewGraphicsWidget::Cast<T>(pViewWidget);
}

template<typename T>
inline T* CNodeGraphViewGraphicsWidget::Cast(CNodeGraphViewGraphicsWidget* pViewWidget)
{
	if (pViewWidget && T::Type == pViewWidget->GetType())
	{
		return static_cast<T*>(pViewWidget);
	}
	return nullptr;
}

template<typename T>
inline T* CNodeGraphViewGraphicsWidget::Cast()
{
	return Cast<T>(this);
}

}

