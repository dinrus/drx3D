// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/Node.h>
//Вначале не помешает заглянуть сюда!

namespace BehaviorTree
//Пространство имён ДеревоПоведение
{
class Action : public Node
//Класс Действие, наследующий от класса Узел
{
	typedef Node BaseClass;
	//Объявление типа КлассОснова = Узел

public:
	virtual void HandleEvent(const EventContext& context, const Event& event) override
	//Виртуальный метод ОбработайСобытие: принимает Контекст События и ссылку на Событие
	{
	}

	virtual LoadResult LoadFromXml(const XmlNodeRef& xml, const LoadContext& context) override
	//Виртуальная функция ЗагрузиИзРяр: принимает СсылкуНаУзелРяр, ссылку на КонтекстЗагрузки.
	//Возвращает результат загрузки.
	{
		IF_UNLIKELY (BaseClass::LoadFromXml(xml, context) == LoadFailure)
			return LoadFailure;

		if (xml->getChildCount() == 0)
		{
			return LoadSuccess;
		}
		else
		{
			ErrorReporter(*this, context).LogError("No children allowed but found %d.", xml->getChildCount());
			return LoadFailure;
		}
	}

	virtual void OnInitialize(const UpdateContext& context) override
	//Виртуальный метод ПриИнициализации: примимает КонтекстДляОбновления
	{
	}

	virtual void OnTerminate(const UpdateContext& context) override
	//Виртуальный метод ПриТерминации: примимает КонтекстДляОбновления
	{
	}
};
}
