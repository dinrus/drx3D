// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/StringList.h>
#include <drx3D/CoreX/Serialization/ClassFactory.h>
#include <drx3D/CoreX/Extension/IDrxFactoryRegistry.h>
#include <drx3D/CoreX/Extension/DrxTypeID.h>
#include <drx3D/Sys/ISystem.h>

namespace Serialization {

	//! Generate user-friendly class name, e.g. convert "AnimationPoseModifier_FootStore" -> "Foot Store"
	inline string MakePrettyClassName(tukk className)
	{
		tukk firstSep = strchr(className, '_');
		if (!firstSep)
		{
			// name doesn't follow expected convention, return as is
			return className;
		}

		tukk start = firstSep + 1;
		string result;
		result.reserve(strlen(start) + 4);

		tukk p = start;
		while (*p != '\0')
		{
			if (*p >= 'A' && *p <= 'Z' &&
				*(p - 1) >= 'a' && *(p - 1) <= 'z')
			{
				result += ' ';
			}
			if (*p == '_')
				result += ' ';
			else
				result += *p;
			++p;
		}

		return result;
	}

	//! Provides Serialization::IClassFactory interface for classes registered with DrxExtension to IArchive.
	//! TSerializable can be used to expose Serialize method through a separate interface, rathern than TBase.
	//! Safe to missing as QueryInterface is used to check its presence.
	template<class TBase, class TSerializable = TBase>
	class DrxExtensionClassFactory : public Serialization::IClassFactory
	{
	public:
		size_t size() const override
		{
			return m_types.size();
		}

		static DrxExtensionClassFactory& the()
		{
			static DrxExtensionClassFactory instance;
			return instance;
		}

		DrxExtensionClassFactory()
			: IClassFactory(Serialization::TypeID::get<TBase>())
		{
			setNullLabel("[ None ]");
			IDrxFactoryRegistry* factoryRegistry = gEnv->pSystem->GetDrxFactoryRegistry();

			size_t factoryCount = 0;
			factoryRegistry->IterateFactories(drxiidof<TBase>(), 0, factoryCount);
			std::vector<IDrxFactory*> factories(factoryCount, nullptr);
			if (factoryCount)
				factoryRegistry->IterateFactories(drxiidof<TBase>(), &factories[0], factoryCount);

			string sharedPrefix;
			bool hasSharedPrefix = true;
			for (size_t i = 0; i < factoryCount; ++i)
			{
				IDrxFactory* factory = factories[i];
				if (factory->ClassSupports(drxiidof<TSerializable>()))
				{
					m_factories.push_back(factory);
					if (hasSharedPrefix)
					{
						// make sure that shared prefix is the same for all the names
						tukk name = factory->GetName();
						tukk lastPrefixCharacter = strchr(name, '_');
						if (lastPrefixCharacter == 0)
							hasSharedPrefix = false;
						else
						{
							if (!sharedPrefix.empty())
							{
								if (strncmp(name, sharedPrefix.c_str(), sharedPrefix.size()) != 0)
									hasSharedPrefix = false;
							}
							else
								sharedPrefix.assign(name, lastPrefixCharacter + 1);
						}
					}
				}
			}

			size_t usableFactoriesCount = m_factories.size();
			m_types.reserve(usableFactoriesCount);
			m_labels.reserve(usableFactoriesCount);

			for (size_t i = 0; i < usableFactoriesCount; ++i)
			{
				IDrxFactory* factory = m_factories[i];
				m_classIds.push_back(factory->GetClassID());
				tukk name = factory->GetName();
				m_labels.push_back(MakePrettyClassName(name));
				if (hasSharedPrefix)
					name += sharedPrefix.size();
				m_types.push_back(Serialization::TypeDescription(Serialization::TypeID(), name, m_labels.back().c_str()));
			}
		}

		const Serialization::TypeDescription* descriptionByIndex(i32 index) const override
		{
			if (size_t(index) >= m_types.size())
				return 0;
			return &m_types[index];
		}

		const Serialization::TypeDescription* descriptionByRegisteredName(tukk registeredName) const override
		{
			size_t count = m_types.size();
			for (size_t i = 0; i < m_types.size(); ++i)
				if (strcmp(m_types[i].name(), registeredName) == 0)
					return &m_types[i];
			return 0;
		}

		tukk findAnnotation(tukk typeName, tukk name) const override { return ""; }

		void        serializeNewByIndex(IArchive& ar, i32 index, tukk name, tukk label) override
		{
			if (size_t(index) >= m_types.size())
				return;
			std::shared_ptr<TBase> ptr(create(m_types[index].name()));
			if (TSerializable* ser = drxinterface_cast<TSerializable>(ptr.get()))
			{
				ar(*ser, name, label);
			}
		}

		std::shared_ptr<TBase> create(tukk registeredName)
		{
			size_t count = m_types.size();
			for (size_t i = 0; i < count; ++i)
				if (strcmp(m_types[i].name(), registeredName) == 0)
					return std::static_pointer_cast<TBase>(m_factories[i]->CreateClassInstance());
			return std::shared_ptr<TBase>();
		}

		tukk getRegisteredTypeName(const std::shared_ptr<TBase>& ptr) const
		{
			if (!ptr.get())
				return "";
			DrxInterfaceID id = std::static_pointer_cast<TBase>(ptr)->GetFactory()->GetClassID();
			size_t count = m_classIds.size();
			for (size_t i = 0; i < count; ++i)
				if (m_classIds[i] == id)
					return m_types[i].name();
			return "";
		}

	private:
		std::vector<Serialization::TypeDescription> m_types;
		std::vector<string>                         m_labels;
		std::vector<IDrxFactory*>                   m_factories;
		std::vector<DrxInterfaceID>                 m_classIds;
	};

	//! Exposes DrxExtension shared_ptr<> as serializeable type for Serialization::IArchive.
	template<class T, class TSerializable>
	class DrxExtensionSharedPtr : public Serialization::IPointer
	{
	public:
		DrxExtensionSharedPtr(std::shared_ptr<T>& ptr)
			: m_ptr(ptr)
		{}

		tukk registeredTypeName() const override
		{
			if (m_ptr)
				return factory()->getRegisteredTypeName(m_ptr);
			else
				return "";
		}

		void create(tukk registeredTypeName) const override
		{
			if (registeredTypeName[0] != '\0')
				m_ptr = factory()->create(registeredTypeName);
			else
				m_ptr.reset();
		}

		Serialization::TypeID          baseType() const override { return Serialization::TypeID::get<T>(); }
		virtual Serialization::SStruct serializer() const override
		{
			if (TSerializable* ser = drxinterface_cast<TSerializable>(m_ptr.get()))
				return Serialization::SStruct(*ser);
			else
				return Serialization::SStruct();
		}
		uk                                       get() const override { return reinterpret_cast<uk>(m_ptr.get()); }
		ukk                                 handle() const override { return &m_ptr; }
		Serialization::TypeID                       pointerType() const override { return Serialization::TypeID::get<std::shared_ptr<T>>(); }
		DrxExtensionClassFactory<T, TSerializable>* factory() const override { return &DrxExtensionClassFactory<T, TSerializable>::the(); }
	protected:
		std::shared_ptr<T>& m_ptr;
	};

	template<class TPointer, class TSerializable>
	bool Serialize(Serialization::IArchive& ar, Serialization::DrxExtensionPointer<TPointer, TSerializable>& ptr, tukk name, tukk label)
	{
		Serialization::DrxExtensionSharedPtr<TPointer, TSerializable> serializer(ptr.ptr);
		return ar(static_cast<Serialization::IPointer&>(serializer), name, label);
	}
}