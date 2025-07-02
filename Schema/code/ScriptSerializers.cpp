// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptSerializers.h>

#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/Schema/EnumFlags.h>

#include <drx3D/Schema/Script.h>
#include <drx3D/Schema/ScriptElementFactory.h>
#include <drx3D/Schema/ScriptActionInstance.h>
#include <drx3D/Schema/ScriptBase.h>
#include <drx3D/Schema/ScriptClass.h>
#include <drx3D/Schema/ScriptComponentInstance.h>
#include <drx3D/Schema/ScriptConstructor.h>
#include <drx3D/Schema/ScriptEnum.h>
#include <drx3D/Schema/ScriptFunction.h>
#include <drx3D/Schema/ScriptInterface.h>
#include <drx3D/Schema/ScriptInterfaceFunction.h>
#include <drx3D/Schema/ScriptInterfaceImpl.h>
#include <drx3D/Schema/ScriptInterfaceTask.h>
#include <drx3D/Schema/ScriptModule.h>
#include <drx3D/Schema/ScriptSignal.h>
#include <drx3D/Schema/ScriptSignalReceiver.h>
#include <drx3D/Schema/ScriptState.h>
#include <drx3D/Schema/ScriptStateMachine.h>
#include <drx3D/Schema/ScriptStruct.h>
#include <drx3D/Schema/ScriptTimer.h>
#include <drx3D/Schema/ScriptVariable.h>
#include <drx3D/Schema/SerializationContext.h>

namespace sxema
{

namespace {

CScriptElementFactory g_scriptElementFactory; // #SchematycTODO : Move to CScriptRegistry? Yes!

enum class EScriptElementOutputSerializerFlags
{
	None    = 0,
	Copying = BIT(0)
};

typedef CEnumFlags<EScriptElementOutputSerializerFlags> ScriptElementOutputSerializerFlags;

struct ScriptElementOutputSerializerParams
{
	inline ScriptElementOutputSerializerParams(const ScriptElementSerializeCallback& _callback, const ScriptElementOutputSerializerFlags& _flags)
		: callback(_callback)
		, flags(_flags)
	{}

	ScriptElementSerializeCallback     callback;
	ScriptElementOutputSerializerFlags flags;
};

class CScriptElementOutputSerializer
{
public:

	inline CScriptElementOutputSerializer()
		: m_pElement(nullptr)
		, m_pParams(nullptr)
	{}

	inline CScriptElementOutputSerializer(IScriptElement& element, const ScriptElementOutputSerializerParams& params)
		: m_pElement(&element)
		, m_pParams(&params)
	{}

	void Serialize(Serialization::IArchive& archive)
	{
		SXEMA_ENV_ASSERT_FATAL(m_pElement && m_pParams);

		{
			CSerializationContext serializationContext(SSerializationContextParams(archive, ESerializationPass::Save));
			m_pElement->Serialize(archive);
		}

		if (m_pParams->callback)
		{
			m_pParams->callback(*m_pElement);
		}

		std::vector<CScriptElementOutputSerializer> children;
		children.reserve(20);
		for (IScriptElement* pChildElement = m_pElement->GetFirstChild(); pChildElement; pChildElement = pChildElement->GetNextSibling())
		{
			if (m_pParams->flags.Check(EScriptElementOutputSerializerFlags::Copying) || !pChildElement->GetScript())
			{
				children.push_back(CScriptElementOutputSerializer(*pChildElement, *m_pParams));
			}
		}
		archive(children, "children");
	}

private:

	IScriptElement*                            m_pElement;
	const ScriptElementOutputSerializerParams* m_pParams;
};

} // Anonymous

CScriptInputElementSerializer::CScriptInputElementSerializer(IScriptElement& element, ESerializationPass serializationPass, const ScriptElementSerializeCallback& callback)
	: m_element(element)
	, m_serializationPass(serializationPass)
{}

void CScriptInputElementSerializer::Serialize(Serialization::IArchive& archive)
{
	CSerializationContext serializationContext(SSerializationContextParams(archive, m_serializationPass));
	m_element.Serialize(archive);
}

SScriptInputElement::SScriptInputElement()
	: sortPriority(0)
{}

void SScriptInputElement::Serialize(Serialization::IArchive& archive)
{
	if (!instance)
	{
		EScriptElementType elementType = EScriptElementType::None;
		archive(elementType, "elementType");

		instance = g_scriptElementFactory.CreateElement(elementType);
		if (instance)
		{
			children.reserve(20);
			archive(children, "children");
			for (SScriptInputElement& child : children)
			{
				if (child.instance)
				{
					instance->AttachChild(*child.instance);
				}
			}
		}
	}
}

bool Serialize(Serialization::IArchive& archive, SScriptInputElement& value, tukk szName, tukk )
{
	archive(value.blackBox, szName);
	Serialization::LoadBlackBox(value, value.blackBox);
	return true;
}

CScriptLoadSerializer::CScriptLoadSerializer(SScriptInputBlock& inputBlock, const ScriptElementSerializeCallback& callback)
	: m_inputBlock(inputBlock)
	, m_callback(callback)
{}

void CScriptLoadSerializer::Serialize(Serialization::IArchive& archive)
{
	u32 versionNumber = 0;
	archive(versionNumber, "version");
	// #SchematycTODO : Check version number!
	{
		archive(m_inputBlock.guid, "guid");
		archive(m_inputBlock.scopeGUID, "scope");
		archive(m_inputBlock.rootElement, "root");
	}
}

CScriptSaveSerializer::CScriptSaveSerializer(CScript& script, const ScriptElementSerializeCallback& callback)
	: m_script(script)
	, m_callback(callback)
{}

void CScriptSaveSerializer::Serialize(Serialization::IArchive& archive)
{
	u32 versionNumber = 105;
	archive(versionNumber, "version");

	DrxGUID guid = m_script.GetGUID();
	archive(guid, "guid");

	IScriptElement* pRoot = m_script.GetRoot();
	if (pRoot)
	{
		IScriptElement* pScope = pRoot->GetParent();
		if (pScope)
		{
			DrxGUID scopeGUID = pScope->GetGUID();
			archive(scopeGUID, "scope");
		}

		archive(CScriptElementOutputSerializer(*pRoot, ScriptElementOutputSerializerParams(m_callback, EScriptElementOutputSerializerFlags::None)), "root");
	}
}

CScriptCopySerializer::CScriptCopySerializer(IScriptElement& root, const ScriptElementSerializeCallback& callback)
	: m_root(root)
	, m_callback(callback)
{}

void CScriptCopySerializer::Serialize(Serialization::IArchive& archive)
{
	archive(CScriptElementOutputSerializer(m_root, ScriptElementOutputSerializerParams(ScriptElementSerializeCallback(), EScriptElementOutputSerializerFlags::Copying)), "root");
}

CScriptPasteSerializer::CScriptPasteSerializer(SScriptInputBlock& inputBlock, const ScriptElementSerializeCallback& callback)
	: m_inputBlock(inputBlock)
	, m_callback(callback)
{}

void CScriptPasteSerializer::Serialize(Serialization::IArchive& archive)
{
	archive(m_inputBlock.rootElement, "root");
}

void UnrollScriptInputElementsRecursive(ScriptInputElementPtrs& output, SScriptInputElement& element)
{
	if (element.instance)
	{
		output.push_back(&element);
	}
	for (SScriptInputElement& childElement : element.children)
	{
		UnrollScriptInputElementsRecursive(output, childElement);
	}
}

bool SortScriptInputElementsByDependency(ScriptInputElementPtrs& elements)
{
	typedef std::unordered_map<DrxGUID, SScriptInputElement*> ElementsByGUID;

	ElementsByGUID elementsByGUID;
	u32k elementCount = elements.size();
	elementsByGUID.reserve(elementCount);
	for (SScriptInputElement* pElement : elements)
	{
		elementsByGUID.insert(ElementsByGUID::value_type(pElement->instance->GetGUID(), pElement));
	}

	for (SScriptInputElement* pElement : elements)
	{
		auto visitElementDependency = [&elementsByGUID, pElement](const DrxGUID& guid)
		{
			ElementsByGUID::const_iterator itDependency = elementsByGUID.find(guid);
			if (itDependency != elementsByGUID.end())
			{
				pElement->dependencies.push_back(itDependency->second);
			}
		};
		pElement->instance->EnumerateDependencies(visitElementDependency, EScriptDependencyType::Load);
	}

	u32 recursiveDependencyCount = 0;
	for (SScriptInputElement* pElement : elements)
	{
		for (SScriptInputElement* pDependencyElement : pElement->dependencies)
		{
			if (std::find(pDependencyElement->dependencies.begin(), pDependencyElement->dependencies.end(), pElement) != pDependencyElement->dependencies.end())
			{
				++recursiveDependencyCount;
			}
		}
	}
	if (recursiveDependencyCount)
	{
		SXEMA_CORE_CRITICAL_ERROR("%d recursive dependencies detected!", recursiveDependencyCount);
		return false;
	}

	bool bShuffle;
	do
	{
		bShuffle = false;
		for (SScriptInputElement* pElement : elements)
		{
			for (SScriptInputElement* pDependencyElement : pElement->dependencies)
			{
				if (pDependencyElement->sortPriority <= pElement->sortPriority)
				{
					pDependencyElement->sortPriority = pElement->sortPriority + 1;
					bShuffle = true;
				}
			}
		}
	}
	while (bShuffle);

	auto compareElements = [](const SScriptInputElement* lhs, const SScriptInputElement* rhs) -> bool
	{
		return lhs->sortPriority > rhs->sortPriority;
	};
	std::sort(elements.begin(), elements.end(), compareElements);
	return true;
}
} // sxema
