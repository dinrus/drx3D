/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include <array>
#include <vector>

#include "Config.h"
#include "Assert.h"
#include "ClassFactoryBase.h"
#include "TypeID.h"
#if DRX_PLATFORM_WINAPI
#include <drx3D/CoreX/Platform/DrxLibrary.h>
#endif

namespace yasli{

class Archive;

class ClassFactoryUpr{
public:
	static ClassFactoryUpr& the()
	{
#if DRX_PLATFORM_WINAPI
		// As this code is part of DrxCommon, using a normal Singleton would result in one instance inside each DLL.
		// To prevent this, we need to load the exported variable from the executable and store a pointer to it.
		// The declaration of GetYasliClassFactoryUpr can be found in platform_impl.h
		typedef yasli::ClassFactoryUpr*(*TGetFactoryUpr)();
		static const TGetFactoryUpr getYasliClassFactoryUpr = reinterpret_cast<TGetFactoryUpr>(DrxGetProcAddress(DrxGetCurrentModule(), "GetYasliClassFactoryUpr"));
		static ClassFactoryUpr* const s_pUpr = getYasliClassFactoryUpr();
		return *s_pUpr;
#else
		static ClassFactoryUpr manager;
		return manager;
#endif
	}

	ClassFactoryUpr()
		: m_factoryCount(0)
	{}
	ClassFactoryUpr(const ClassFactoryUpr&) = delete;
	ClassFactoryUpr& operator=(const ClassFactoryUpr& other) = delete;
	ClassFactoryUpr(const ClassFactoryUpr&&) = delete;
	ClassFactoryUpr& operator=(const ClassFactoryUpr&& other) = delete;

	// The functions below are marked virtual even though there is no inheriting class in order to force a
	// vtable lookup which makes sure that all modules use the function that was compiled into the executable.
	// Otherwise we might run into crashes when using mixed builds due to iterator size differences.

	virtual ClassFactoryBase* find(TypeID baseType) const
	{
		const auto end = m_factories.cbegin() + m_factoryCount;
		const auto it = std::find_if(m_factories.cbegin(), end, [baseType](const Factories::value_type& lhs) { return lhs.first == baseType; });
		return it != end ? it->second : nullptr;
	}

	virtual void registerFactory(TypeID type, ClassFactoryBase* const pFactory)
	{
		const auto end = m_factories.begin() + m_factoryCount;
		const auto it = std::find_if(m_factories.begin(), end, [type](const Factories::value_type& lhs) { return lhs.first == type; });
		if (it == end)
		{
			DRX_ASSERT_MESSAGE(m_factoryCount < m_factories.size(), "Too many factories registered. Increase MAX_FACTORIES to fix this.");
			if (m_factoryCount < m_factories.size())
			{
				m_factories[m_factoryCount] = std::make_pair(type, pFactory);
				++m_factoryCount;
			}
		}
		else
		{
			it->second = pFactory;
		}
	}

protected:
	static constexpr size_t MAX_FACTORIES = 1024;
	typedef std::array<std::pair<TypeID, ClassFactoryBase*>, MAX_FACTORIES> Factories; // Needs to be a fixed size array as the memorymanager might not be available yet
	Factories m_factories;
	size_t m_factoryCount;
};

template<class BaseType>
class ClassFactory : public ClassFactoryBase{
public:
	static ClassFactory& the()
	{
		// As this code is part of DrxCommon, using a normal Singleton would result in one instance inside each DLL.
		// This problem is already fixed in ClassFactoryUpr so we just get the instance pointer from there.
		ClassFactoryBase* pFactory = ClassFactoryUpr::the().find(TypeID::get<BaseType>());
		if (pFactory == nullptr)
		{
			pFactory = new ClassFactory(); // The constructor takes care of registering the instance with the ClassFactoryUpr
		}
		return static_cast<ClassFactory&>(*pFactory);
	}

	class CreatorBase{
	public:
		CreatorBase() {
			if (head() == this) {
				YASLI_ASSERT(0);
			}
			if (head()) {
				next = head();
				head()->previous = this;
			}
			else {
				next = 0;
				previous = 0;
			}
			head() = this;
		}

		virtual ~CreatorBase() {
			if (previous) {
				previous->next = next;
			}
			if (this == head()) {
				head() = next;
				if (next)
					next->previous = 0;
			}
		}
		virtual BaseType* create() const = 0;
		virtual TypeID typeID() const = 0;
		const TypeDescription& description() const{ return *description_; }
#if YASLI_NO_RTTI
		uk vptr() const{ return vptr_; }
#endif
		static CreatorBase*& head() { static CreatorBase* head; return head; }
		CreatorBase* next;
		CreatorBase* previous;
	protected:
		const TypeDescription* description_;
#if YASLI_NO_RTTI
		uk vptr_;
#endif
	};

	template<typename T, typename std::enable_if<std::is_polymorphic<T>::value, i32>::type = 0>
	static uk extractVPtr(T* ptr)
	{
		return *((uk *)ptr);
	}

	template<typename T, typename std::enable_if<!std::is_polymorphic<T>::value, i32>::type = 0>
	static uk extractVPtr(T* ptr)
	{
		return nullptr;
	}

	template<class Derived>
	struct Annotation
	{
		Annotation(ClassFactoryBase* factory, tukk name, tukk value) { static_cast<ClassFactory<BaseType>*>(factory)->addAnnotation<Derived>(name, value); }
	};

	template<class Derived>
	class Creator : public CreatorBase{
	public:
		Creator(const TypeDescription* description, ClassFactory* factory = 0){
			this->description_ = description;
#if YASLI_NO_RTTI
			// TODO: remove unnecessary static initialisation
			Derived vptrProbe;
            CreatorBase::vptr_ = extractVPtr(&vptrProbe);
#endif
			if (!factory)
				factory = &ClassFactory::the();
			factory->registerCreator(this);
		}
		BaseType* create() const override { return new Derived(); }
		TypeID typeID() const override{ return yasli::TypeID::get<Derived>(); }
	};

	ClassFactory()
	: ClassFactoryBase(TypeID::get<BaseType>())
	{
		ClassFactoryUpr::the().registerFactory(baseType_, this);
	}

	typedef std::map<string, CreatorBase*> TypeToCreatorMap;

	BaseType* create(tukk registeredName) const
	{
		if (!registeredName)
			return 0;
		if (registeredName[0] == '\0')
			return 0;
		typename TypeToCreatorMap::const_iterator it = typeToCreatorMap_.find(registeredName);
		if(it != typeToCreatorMap_.end())
			return it->second->create();
		else
			return 0;
	}

	virtual tukk getRegisteredTypeName(BaseType* ptr) const
	{
		if (ptr == 0)
			return "";
		uk vptr = extractVPtr(ptr);
		if (!vptr)
			return "";
		typename VPtrToCreatorMap::const_iterator it = vptrToCreatorMap_.find(vptr);
		if (it == vptrToCreatorMap_.end())
			return "";
		return it->second->description().name();
	}

	BaseType* createByIndex(i32 index) const
	{
		YASLI_ASSERT(size_t(index) < creators_.size());
		return creators_[index]->create();
	}

	void serializeNewByIndex(Archive& ar, i32 index, tukk name, tukk label) override
	{
		YASLI_ESCAPE(size_t(index) < creators_.size(), return);
		BaseType* ptr = creators_[index]->create();
		ar(*ptr, name, label);
		delete ptr;
	}
	// from ClassFactoryInterface:
	size_t size() const override{ return creators_.size(); }
	const TypeDescription* descriptionByIndex(i32 index) const override{
		if(size_t(index) >= i32(creators_.size()))
			return 0;
		return &creators_[index]->description();
	}

	const TypeDescription* descriptionByRegisteredName(tukk name) const override{
		const size_t numCreators = creators_.size();
		for (size_t i = 0; i < numCreators; ++i) {
			if (strcmp(creators_[i]->description().name(), name) == 0)
				return &creators_[i]->description();
		}
		return 0;
	}
	// ^^^

	TypeID typeIDByRegisteredName(tukk registeredTypeName) const {
		RegisteredNameToTypeID::const_iterator it = registeredNameToTypeID_.find(registeredTypeName);
		if (it == registeredNameToTypeID_.end())
			return TypeID();
		return it->second;
	}
	// ^^^
	
	CreatorBase* creatorChain() { return CreatorBase::head(); }
	
	void registerChain(CreatorBase* head)
	{
		CreatorBase* current = head;
		while (current) {
			registerCreator(current);
			current = current->next;
		}
	}

	void unregisterChain(CreatorBase* head)
	{
		CreatorBase* current = head;
		while (current) {
			typeToCreatorMap_.erase(current->description().typeID());
#if YASLI_NO_RTTI
			if(current->vptr())
				vptrToCreatorMap_.erase(current->vptr());
#endif
			for (size_t i = 0; i < creators_.size(); ++i) {
				if (creators_[i] == current) {
					creators_.erase(creators_.begin() + i);
					--i;
				}
			}
			current = current->next;
		}
	}

	tukk findAnnotation(tukk registeredTypeName, tukk name) const override {
		TypeID typeID = typeIDByRegisteredName(registeredTypeName);
		AnnotationMap::const_iterator it = annotations_.find(typeID);
		if (it == annotations_.end())
			return "";
		for (size_t i = 0; i < it->second.size(); ++i) {
			if (strcmp(it->second[i].first, name) == 0)
				return it->second[i].second;
		}
		return "";
	}

protected:
	virtual void registerCreator(CreatorBase* creator){
		if (!typeToCreatorMap_.insert(std::make_pair(creator->description().name(), creator)).second) {
			YASLI_ASSERT(0 && "Type registered twice in the same factory. Was YASLI_CLASS_NAME put into header file by mistake");
		}
		creators_.push_back(creator);
		registeredNameToTypeID_[creator->description().name()] = creator->typeID();
		if(creator->vptr())
			vptrToCreatorMap_[creator->vptr()] =  creator;
	}

	template<class T>
	void addAnnotation(tukk name, tukk value) {
		addAnnotation(yasli::TypeID::get<T>(), name, value);
	}

	virtual void addAnnotation(const yasli::TypeID& id, tukk name, tukk value) {
		annotations_[id].push_back(std::make_pair(name, value));
	}

	TypeToCreatorMap typeToCreatorMap_;
	std::vector<CreatorBase*> creators_;

	typedef std::map<uk , CreatorBase*> VPtrToCreatorMap;
	VPtrToCreatorMap vptrToCreatorMap_;

	typedef std::map<string, TypeID> RegisteredNameToTypeID;
	RegisteredNameToTypeID registeredNameToTypeID_;

	typedef std::map<TypeID, std::vector<std::pair<tukk , tukk > > > AnnotationMap;
	AnnotationMap annotations_;
};

}

#define YASLI_JOIN_HELPER(x,y,z,w) x ## y ## z ## w
#define YASLI_JOIN(x,y,z,w) YASLI_JOIN_HELPER(x,y,z,w)

#define YASLI_CLASS_NULL_HELPER(Counter, BaseType, name) \
	static bool YASLI_JOIN(baseType, _NullRegistered_, _,Counter) = yasli::ClassFactory<BaseType>::the().setNullLabel(name); \

#define YASLI_CLASS_NULL(BaseType, name) \
	YASLI_CLASS_NULL_HELPER(__COUNTER__, BaseType, name)

#define YASLI_CLASS(BaseType, Type, label) \
	static const yasli::TypeDescription Type##BaseType##_DerivedDescription(yasli::TypeID::get<Type>(), #Type, label); \
	static yasli::ClassFactory<BaseType>::Creator<Type> Type##BaseType##_Creator(&Type##BaseType##_DerivedDescription); \
	i32 dummyForType_##Type##BaseType;

#define YASLI_CLASS_NAME_HELPER(Counter, BaseType, Type, name, label) \
	static yasli::TypeDescription YASLI_JOIN(globalDerivedDescription,__LINE__,_,Counter)(yasli::TypeID::get<Type>(), name, label); \
	static const yasli::ClassFactory<BaseType>::Creator<Type> YASLI_JOIN(globalCreator,__LINE__,_,Counter)(&YASLI_JOIN(globalDerivedDescription,__LINE__,_,Counter)); \

#define YASLI_CLASS_NAME(BaseType, Type, name, label) \
	YASLI_CLASS_NAME_HELPER(__COUNTER__, BaseType, Type, name, label)

#define YASLI_CLASS_NAME_FOR_FACTORY(Factory, BaseType, Type, name, label) \
	static const yasli::TypeDescription Type##BaseType##_DerivedDescription(yasli::TypeID::get<Type>(), name, label); \
	static yasli::ClassFactory<BaseType>::Creator<Type> Type##BaseType##_Creator(&Type##BaseType##_DerivedDescription, &(Factory));

#define YASLI_CLASS_ANNOTATION(BaseType, Type, attributeName, attributeValue) \
	static yasli::ClassFactory<BaseType>::Annotation<Type> Type##BaseType##_Annotation(&yasli::ClassFactory<BaseType>::the(), attributeName, attributeValue);

#define YASLI_CLASS_ANNOTATION_FOR_FACTORY(factory, BaseType, Type, attributeName, attributeValue) \
	static yasli::ClassFactory<BaseType>::Annotation<Type> Type##BaseType##_Annotation(&factory, attributeName, attributeValue);

#define YASLI_FORCE_CLASS(BaseType, Type) \
	extern i32 dummyForType_##Type##BaseType; \
	i32* dummyForTypePtr_##Type##BaseType = &dummyForType_##Type##BaseType + 1;

#if YASLI_INLINE_IMPLEMENTATION
#include "ClassFactoryImpl.h"
#endif

