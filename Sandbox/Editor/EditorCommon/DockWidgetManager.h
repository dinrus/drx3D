// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/TypeID.h>
#include <QObject>
#include <QScopedPointer>
#include <QWidget>
#include <QDockWidget>

#include "Serialization.h"
#include <vector>
#include <memory>

struct IDockWidgetType
{
	virtual tukk            Name() const = 0;
	virtual tukk            Title() const = 0;
	virtual QWidget*               Create(QDockWidget* dw) = 0;
	virtual i32                    DockArea() const = 0;
	virtual Serialization::SStruct Serializer(QWidget* widget) const = 0;
};

template<class T, class C>
struct DockWidgetType : IDockWidgetType
{
	typedef T* (C::* CreateFunc)();
	C*         m_creator;
	CreateFunc m_createFunc;
	string     m_title;
	i32        m_dockArea;

	DockWidgetType(C* creator, CreateFunc createFunc, tukk title, i32 dockArea)
		: m_creator(creator)
		, m_createFunc(createFunc)
		, m_title(title)
		, m_dockArea(dockArea)
	{}

	tukk Name() const override    { return Serialization::TypeID::get<T>().name(); }
	tukk Title() const override   { return m_title; }
	QWidget*    Create(QDockWidget* dw) override
	{
		T* result = (m_creator->*m_createFunc)();
		result->SetDockWidget(dw);
		return result;
	}
	i32                    DockArea() const override { return m_dockArea; }
	Serialization::SStruct Serializer(QWidget* widget) const override
	{
		return Serialization::SStruct(*static_cast<T*>(widget));
	}
};

// Controls dock-windows that can be splitted.
class EDITOR_COMMON_API DockWidgetManager : public QObject
{
	Q_OBJECT
public:
	DockWidgetManager(QMainWindow* mainWindow);
	~DockWidgetManager();

	template<class T, class C>
	void AddDockWidgetType(C* creator, typename DockWidgetType<T, C>::CreateFunc createFunc, tukk title, Qt::DockWidgetArea dockArea)
	{
		DockWidgetType<T, C>* dockWidgetType = new DockWidgetType<T, C>(creator, createFunc, title, (i32)dockArea);
		m_types.push_back(std::unique_ptr<IDockWidgetType>(dockWidgetType));
	}

	QWidget*               CreateByTypeName(tukk typeName, QDockWidget* dockWidget) const;

	void                   RemoveOrHideDockWidget(QDockWidget* dockWidget);
	void                   SplitDockWidget(QDockWidget* dockWidget);

	void                   ResetToDefault();

	void                   Clear();

	void                   Serialize(IArchive& ar);
signals:
	void                   SignalChanged();
private:
	void                   CreateDefaultWidget(IDockWidgetType* type);
	string                 MakeUniqueDockWidgetName(tukk type) const;
	IDockWidgetType*       FindType(tukk name, i32* typeIndex = 0);
	void                   SplitOpenWidget(i32 index);
	QDockWidget*           CreateDockWidget(IDockWidgetType* type);
	Serialization::SStruct WidgetSerializer(QWidget* widget, tukk dockTypeName);

	struct OpenWidget
	{
		QWidget*     widget;
		QDockWidget* dockWidget;
		string       type;
		string       createdType;

		OpenWidget() : widget(), dockWidget() {}
		void Serialize(IArchive& ar);
		void Destroy(QMainWindow* window);
		string GetName() const;
	};

	QMainWindow*                                  m_mainWindow;
	std::vector<OpenWidget>                       m_openWidgets;
	std::vector<std::unique_ptr<IDockWidgetType>> m_types;
};

