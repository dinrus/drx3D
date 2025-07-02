// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QTextEdit.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>

struct ITextEventListener
{
	virtual ~ITextEventListener() {}
	virtual void OnMessageDoubleClicked(QMouseEvent* event) = 0;
	virtual bool OnMouseMoved(QMouseEvent* event) = 0;
};

typedef CListenerSet<ITextEventListener*> TextEventListeners;

class CCSharpOutputTextEdit : public QTextEdit
{
public:
	CCSharpOutputTextEdit() = default;
	virtual ~CCSharpOutputTextEdit() = default;

	void AddTextEventListener(ITextEventListener* pTextEventListener)
	{
		if (pTextEventListener)
		{
			m_textEventListeners.Add(pTextEventListener);
			setMouseTracking(true);
		}
	};

	void RemoveTextEventListener(ITextEventListener* pTextEventListener)
	{
		if (pTextEventListener)
		{
			m_textEventListeners.Remove(pTextEventListener);
			if (m_textEventListeners.Empty())
			{
				setMouseTracking(false);
			}
		}
	};

protected:
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override
	{
		for (TextEventListeners::Notifier notifier(m_textEventListeners); notifier.IsValid(); notifier.Next())
		{
			notifier->OnMessageDoubleClicked(event);
		}
	};

	virtual void mouseMoveEvent(QMouseEvent* event) override
	{
		bool accept = false;
		for (TextEventListeners::Notifier notifier(m_textEventListeners); notifier.IsValid(); notifier.Next())
		{
			accept = notifier->OnMouseMoved(event) || accept;
		}
		if (accept)
		{
			event->accept();
		}
		else
		{
			QTextEdit::mouseMoveEvent(event);
		}
	};

private:
	TextEventListeners m_textEventListeners = TextEventListeners(1);
};
