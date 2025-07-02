// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/BlackBox.h>
#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/Schema/IScriptElement.h>
#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/Delegate.h>

#include <drx3D/Schema/Script.h>

namespace sxema
{
// Forward declare structures.
struct SScriptInputElement;
// Forward declare classes.
class CScript;

//////////////////////////////////////////////////////////////////////////
// FILE FORMAT
//////////////////////////////////////////////////////////////////////////
// <DrxSchematyc>
//  <version value="..."/>
//  <guid value="..."/>
//  <scope value="..."/>
//  <root>
//   <elementType value="..."/>
//   <guid value="..."/>
//   <name value="..."/>
//   <extensions />
//   <children>
//    <Element>
//     <elementType value="..."/>
//     <guid value="..."/>
//     <name value="..."/>
//     <inputs />
//     <userDocumentation>
//      <author value="..."/>
//     </userDocumentation>
//     <extensions />
//     <children />
//    </Element>
//    <Element>
//			...
//    </Element>
//   </children>
//  </root>
// </schematyc>
//////////////////////////////////////////////////////////////////////////
typedef std::function<void (IScriptElement&)> ScriptElementSerializeCallback;


class CScriptInputElementSerializer
{
public:
	CScriptInputElementSerializer(IScriptElement& element, ESerializationPass serializationPass, const ScriptElementSerializeCallback& callback = ScriptElementSerializeCallback());

	void Serialize(Serialization::IArchive& archive);

private:
	IScriptElement&                m_element;
	ESerializationPass             m_serializationPass;
	ScriptElementSerializeCallback m_callback;
};

typedef std::vector<SScriptInputElement>  ScriptInputElements;
typedef std::vector<SScriptInputElement*> ScriptInputElementPtrs;

struct SScriptInputElement
{
	SScriptInputElement();

	void Serialize(Serialization::IArchive& archive);

	Serialization::SBlackBox blackBox;
	IScriptElementPtr        instance;
	ScriptInputElements      children;
	ScriptInputElementPtrs   dependencies;
	u32                   sortPriority;
};

struct SScriptInputBlock
{
	DrxGUID               guid;
	DrxGUID               scopeGUID;
	SScriptInputElement rootElement;
};
typedef std::vector<SScriptInputBlock> ScriptInputBlocks;

class CScriptLoadSerializer
{
public:

	CScriptLoadSerializer(SScriptInputBlock& inputBlock, const ScriptElementSerializeCallback& callback = ScriptElementSerializeCallback());

	void Serialize(Serialization::IArchive& archive);

private:

	SScriptInputBlock&             m_inputBlock;
	ScriptElementSerializeCallback m_callback;
};

class CScriptSaveSerializer
{
public:

	CScriptSaveSerializer(CScript& script, const ScriptElementSerializeCallback& callback = ScriptElementSerializeCallback());

	void Serialize(Serialization::IArchive& archive);

private:

	CScript&                       m_script;
	ScriptElementSerializeCallback m_callback;
};

class CScriptCopySerializer
{
public:

	CScriptCopySerializer(IScriptElement& root, const ScriptElementSerializeCallback& callback = ScriptElementSerializeCallback());

	void Serialize(Serialization::IArchive& archive);

private:

	IScriptElement&                m_root;
	ScriptElementSerializeCallback m_callback;
};

class CScriptPasteSerializer
{
public:

	CScriptPasteSerializer(SScriptInputBlock& inputBlock, const ScriptElementSerializeCallback& callback = ScriptElementSerializeCallback());

	void Serialize(Serialization::IArchive& archive);

private:

	SScriptInputBlock&             m_inputBlock;
	ScriptElementSerializeCallback m_callback;
};

void UnrollScriptInputElementsRecursive(ScriptInputElementPtrs& output, SScriptInputElement& element);
bool SortScriptInputElementsByDependency(ScriptInputElementPtrs& elements);
} // sxema
