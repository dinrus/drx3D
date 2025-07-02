// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptElement.h>
#include <drx3D/Schema/IScriptExtension.h>
#include <drx3D/Schema/ScriptDependencyEnumerator.h>
#include <drx3D/Schema/EnumFlags.h>
#include <drx3D/Schema/GUID.h>
#include <drx3D/Schema/Signal.h>
#include <drx3D/Schema/UniqueId.h>
#include <drx3D/Schema/Validator.h>

namespace sxema
{

// Forward declare interfaces.
struct IGraphNodeCompiler;
struct IScriptGraph;
struct IScriptGraphNodeCreationMenu;
// Forward declare structures.
struct SCompilerContext;
struct SElementId;
// Forward declare classes.
class CAnyConstPtr;

enum class EScriptGraphNodeFlags
{
	NotCopyable  = BIT(0),
	NotRemovable = BIT(1)
};

typedef CEnumFlags<EScriptGraphNodeFlags> ScriptGraphNodeFlags;

enum class EScriptGraphPortFlags
{
	None = 0,

	// Category.
	Signal = BIT(0),        // Port is signal.
	Flow   = BIT(1),        // Port controls internal flow.
	Data   = BIT(2),        // Port has data.

	// Connection modifiers.
	MultiLink = BIT(3),     // Multiple links can be connected to port.

	// Flow modifiers.
	Begin = BIT(4),         // Port initiates flow.
	End   = BIT(5),         // Port terminates flow.

	// Data modifiers.
	Ptr        = BIT(6),  // Data is pointer.
	Array      = BIT(7),  // Data is array.
	Persistent = BIT(8),  // Data will be saved to file.
	Editable   = BIT(9),  // Data can be modified in editor.
	Pull       = BIT(10), // Data can be pulled from node without need to trigger an input.

	// Layout modifiers.
	SpacerAbove = BIT(11),   // Draw spacer above port.
	SpacerBelow = BIT(12)    // Draw spacer below port.
};

typedef CEnumFlags<EScriptGraphPortFlags> ScriptGraphPortFlags;

struct IScriptGraphNode // #SchematycTODO : Move to separate header?
{
	virtual ~IScriptGraphNode() {}

	virtual void                Attach(IScriptGraph& graph) = 0;
	virtual IScriptGraph&       GetGraph() = 0;
	virtual const IScriptGraph& GetGraph() const = 0;
	virtual DrxGUID             GetTypeGUID() const = 0;
	virtual DrxGUID             GetGUID() const = 0;
	virtual tukk         GetName() const = 0;
	//virtual tukk          GetBehavior() const = 0;
	//virtual tukk          GetSubject() const = 0;
	//virtual tukk          GetDescription() const = 0;
	virtual tukk          GetStyleId() const = 0;
	virtual ScriptGraphNodeFlags GetFlags() const = 0;
	virtual void                 SetPos(Vec2 pos) = 0;
	virtual Vec2                 GetPos() const = 0;
	virtual u32               GetInputCount() const = 0;
	virtual u32               FindInputById(const CUniqueId& id) const = 0;
	virtual CUniqueId            GetInputId(u32 inputIdx) const = 0;
	virtual tukk          GetInputName(u32 inputIdx) const = 0;
	virtual DrxGUID              GetInputTypeGUID(u32 inputIdx) const = 0;
	virtual ScriptGraphPortFlags GetInputFlags(u32 inputIdx) const = 0;
	virtual CAnyConstPtr         GetInputData(u32 inputIdx) const = 0;
	virtual ColorB               GetInputColor(u32 inputIdx) const = 0;
	virtual u32               GetOutputCount() const = 0;
	virtual u32               FindOutputById(const CUniqueId& id) const = 0;
	virtual CUniqueId            GetOutputId(u32 outputIdx) const = 0;
	virtual tukk          GetOutputName(u32 outputIdx) const = 0;
	virtual DrxGUID              GetOutputTypeGUID(u32 outputIdx) const = 0;
	virtual ScriptGraphPortFlags GetOutputFlags(u32 outputIdx) const = 0;
	virtual CAnyConstPtr         GetOutputData(u32 outputIdx) const = 0;
	virtual ColorB               GetOutputColor(u32 outputIdx) const = 0;
	virtual void                 EnumerateDependencies(const ScriptDependencyEnumerator& enumerator, EScriptDependencyType type) const = 0;
	virtual void                 ProcessEvent(const SScriptEvent& event) = 0;
	virtual void                 Serialize(Serialization::IArchive& archive) = 0;
	virtual void                 Copy(Serialization::IArchive& archive) = 0;  // #SchematycTODO : Rather than having an explicit copy function consider using standard save pass with a copy flag set in the context.
	virtual void                 Paste(Serialization::IArchive& archive) = 0; // #SchematycTODO : Rather than having an explicit paste function consider using standard load passes with a paste flag set in the context.
	virtual void                 Validate(const Validator& validator) const = 0;
	virtual void                 RemapDependencies(IGUIDRemapper& guidRemapper) = 0;
	virtual void                 Compile(SCompilerContext& context, IGraphNodeCompiler& compiler) const {}
};

DECLARE_SHARED_POINTERS(IScriptGraphNode)

typedef std::function<EVisitStatus(IScriptGraphNode&)>       ScriptGraphNodeVisitor;
typedef std::function<EVisitStatus(const IScriptGraphNode&)> ScriptGraphNodeConstVisitor;

struct IScriptGraphLink // #SchematycTODO : Once all ports are referenced by id we can replace this with a simple SSCriptGraphLink structure.
{
	virtual ~IScriptGraphLink() {}

	virtual void      SetSrcNodeGUID(const DrxGUID& guid) = 0;
	virtual DrxGUID   GetSrcNodeGUID() const = 0;
	virtual CUniqueId GetSrcOutputId() const = 0;
	virtual void      SetDstNodeGUID(const DrxGUID& guid) = 0;
	virtual DrxGUID   GetDstNodeGUID() const = 0;
	virtual CUniqueId GetDstInputId() const = 0;
	virtual void      Serialize(Serialization::IArchive& archive) = 0;
};

DECLARE_SHARED_POINTERS(IScriptGraphLink)

typedef std::function<EVisitStatus(IScriptGraphLink&)>       ScriptGraphLinkVisitor;
typedef std::function<EVisitStatus(const IScriptGraphLink&)> ScriptGraphLinkConstVisitor;

enum class EScriptGraphType // #SchematycTODO : Do we really need this enumeration or can we get the information we need from the element that owns the graph?
{
	Construction,
	Signal,
	Function,
	Transition
};

struct SScriptGraphParams
{
	inline SScriptGraphParams(const DrxGUID& _scopeGUID, tukk _szName, EScriptGraphType _type, const DrxGUID& _contextGUID)
		: scopeGUID(_scopeGUID)
		, szName(_szName)
		, type(_type)
		, contextGUID(_contextGUID)
	{}

	const DrxGUID&   scopeGUID;
	tukk      szName;
	EScriptGraphType type;
	const DrxGUID&   contextGUID;
};

typedef CSignal<void (const IScriptGraphLink&)> ScriptGraphLinkRemovedSignal;

struct IScriptGraphNodeCreationCommand
{
	virtual ~IScriptGraphNodeCreationCommand() {}

	virtual tukk         GetBehavior() const = 0;      // What is the node's responsibility?
	virtual tukk         GetSubject() const = 0;       // What does the node operate on?
	virtual tukk         GetDescription() const = 0;   // Get detailed node description.
	virtual tukk         GetStyleId() const = 0;       // Get unique id of node's style.
	virtual IScriptGraphNodePtr Execute(const Vec2& pos) = 0; // Create node.
};

DECLARE_SHARED_POINTERS(IScriptGraphNodeCreationCommand) // #SchematycTODO : Shouldn't these be unique pointers?

struct IScriptGraphNodeCreationMenu
{
	virtual ~IScriptGraphNodeCreationMenu() {}

	virtual bool AddCommand(const IScriptGraphNodeCreationCommandPtr& pCommand) = 0;
};

struct IScriptGraph : public IScriptExtensionBase<EScriptExtensionType::Graph>
{
	virtual ~IScriptGraph() {}

	virtual EScriptGraphType                     GetType() const = 0;

	virtual void                                 SetPos(Vec2 pos) = 0;
	virtual Vec2                                 GetPos() const = 0;

	virtual void                                 PopulateNodeCreationMenu(IScriptGraphNodeCreationMenu& nodeCreationMenu) = 0;
	virtual bool                                 AddNode(const IScriptGraphNodePtr& pNode) = 0;
	virtual IScriptGraphNodePtr                  AddNode(const DrxGUID& typeGUID) = 0;
	virtual void                                 RemoveNode(const DrxGUID& guid) = 0;
	virtual u32                               GetNodeCount() const = 0;
	virtual IScriptGraphNode*                    GetNode(const DrxGUID& guid) = 0;
	virtual const IScriptGraphNode*              GetNode(const DrxGUID& guid) const = 0;
	virtual void                                 VisitNodes(const ScriptGraphNodeVisitor& visitor) = 0;
	virtual void                                 VisitNodes(const ScriptGraphNodeConstVisitor& visitor) const = 0;

	virtual bool                                 CanAddLink(const DrxGUID& srcNodeGUID, const CUniqueId& srcOutputId, const DrxGUID& dstNodeGUID, const CUniqueId& dstInputId) const = 0;
	virtual IScriptGraphLink*                    AddLink(const DrxGUID& srcNodeGUID, const CUniqueId& srcOutputId, const DrxGUID& dstNodeGUID, const CUniqueId& dstInputId) = 0;
	virtual void                                 RemoveLink(u32 iLink) = 0;
	virtual void                                 RemoveLinks(const DrxGUID& nodeGUID) = 0;
	virtual u32                               GetLinkCount() const = 0;
	virtual IScriptGraphLink*                    GetLink(u32 linkIdx) = 0;
	virtual const IScriptGraphLink*              GetLink(u32 linkIdx) const = 0;
	virtual u32                               FindLink(const DrxGUID& srcNodeGUID, const CUniqueId& srcOutputId, const DrxGUID& dstNodeGUID, const CUniqueId& dstInputId) const = 0;
	virtual EVisitResult                         VisitLinks(const ScriptGraphLinkVisitor& visitor) = 0;
	virtual EVisitResult                         VisitLinks(const ScriptGraphLinkConstVisitor& visitor) const = 0;
	virtual EVisitResult                         VisitInputLinks(const ScriptGraphLinkVisitor& visitor, const DrxGUID& dstNodeGUID, const CUniqueId& dstInputId) = 0;
	virtual EVisitResult                         VisitInputLinks(const ScriptGraphLinkConstVisitor& visitor, const DrxGUID& dstNodeGUID, const CUniqueId& dstInputId) const = 0;
	virtual EVisitResult                         VisitOutputLinks(const ScriptGraphLinkVisitor& visitor, const DrxGUID& srcNodeGUID, const CUniqueId& srcOutputId) = 0;
	virtual EVisitResult                         VisitOutputLinks(const ScriptGraphLinkConstVisitor& visitor, const DrxGUID& srcNodeGUID, const CUniqueId& srcOutputId) const = 0;
	virtual bool                                 GetLinkSrc(const IScriptGraphLink& link, IScriptGraphNode*& pNode, u32& outputIdx) = 0;
	virtual bool                                 GetLinkSrc(const IScriptGraphLink& link, const IScriptGraphNode*& pNode, u32& outputIdx) const = 0;
	virtual bool                                 GetLinkDst(const IScriptGraphLink& link, IScriptGraphNode*& pNode, u32& inputIdx) = 0;
	virtual bool                                 GetLinkDst(const IScriptGraphLink& link, const IScriptGraphNode*& pNode, u32& inputIdx) const = 0;

	virtual void                                 RemoveBrokenLinks() = 0;
	virtual ScriptGraphLinkRemovedSignal::Slots& GetLinkRemovedSignalSlots() = 0;

	virtual void                                 FixMapping(IScriptGraphNode& node) = 0;
};

} // sxema
