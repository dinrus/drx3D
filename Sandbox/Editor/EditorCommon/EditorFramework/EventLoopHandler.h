// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <QObject>
#include <QAbstractNativeEventFilter>
#include <vector>

class EDITOR_COMMON_API CEventLoopHandler : public QObject, public QAbstractNativeEventFilter
{
	Q_OBJECT
public:
	CEventLoopHandler();
	~CEventLoopHandler();

	void SetDefaultHandler(QWidget* pDefaultHandler) { m_pDefaultHandler = pDefaultHandler; }
	void AddNativeHandler(uintptr_t id, std::function <bool(uk , long*)>);
	void RemoveNativeHandler(uintptr_t id);

private:
	struct CallBack
	{
		CallBack(size_t id, std::function <bool(uk , long*)> cb)
			: m_id(id)
			, m_cb(cb)
		{}

		size_t m_id;
		std::function <bool(uk , long*)> m_cb;
	};

	virtual bool eventFilter(QObject* object, QEvent* event) override;
	virtual bool nativeEventFilter(const QByteArray &eventType, uk message, long *) override;

	QWidget* m_pDefaultHandler;
	std::vector <CallBack> m_nativeListeners;
};

