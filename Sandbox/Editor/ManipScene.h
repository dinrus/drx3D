// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SandboxAPI.h"
#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/Math/Drx_Geo.h>
#include <drx3D/CoreX/Math/Drx_Color.h>
#include <QObject>
#include "QViewportConsumer.h"

#include <drx3D/CoreX/Serialization/Forward.h>

//#include "ManipSceneUtil.h"

class CAxisHelper;

// Manip is a set of reusable utilities for creating interactive 3d scene that
// can be manipulated with gizmos.
namespace Manip
{

using std::unique_ptr;
using std::set;
using std::vector;

enum EElementCaps
{
	CAP_SELECT = 1 << 0,
	CAP_HIDE   = 1 << 1,
	CAP_MOVE   = 1 << 2,
	CAP_ROTATE = 1 << 3,
	CAP_SCALE  = 1 << 4,
	CAP_DELETE = 1 << 5,
};

enum EElementShape
{
	SHAPE_AXES,
	SHAPE_BOX
};

enum EElementAction
{
	ACTION_NONE,
	ACTION_DELETE,
	ACTION_HIDE,
	ACTION_UNHIDE
};

enum EElementColorGroup
{
	ELEMENT_COLOR_PROXY,
	ELEMENT_COLOR_CLOTH
};

struct SElementPlacement
{
	QuatT transform;
	QuatT startTransform;
	Vec3  center;
	Vec3  size;

	SElementPlacement()
		: transform(IDENTITY)
		, startTransform(IDENTITY)
		, size(1.0f, 1.0f, 1.0f)
		, center(0.0f, 0.0f, 0.0f)
	{}

	void Serialize(Serialization::IArchive& ar);
};

typedef uint64 ElementId;

struct SSpaceAndIndex
{
	i32 m_space;
	i32 m_jointCRC32;
	i32 m_attachmentCRC32;
	SSpaceAndIndex()
	{
		m_space = -1;
		m_jointCRC32 = -1;
		m_attachmentCRC32 = -1;
	};
};

struct SElement
{
	ElementId id;
	union
	{
		i32         originalId;
		ukk originalHandle;
	};
	i32                layer;
	SElementPlacement  placement;
	i32                caps;
	EElementAction     action;
	EElementShape      shape;
	EElementColorGroup colorGroup;
	SSpaceAndIndex     parentSpaceIndex;
	SSpaceAndIndex     parentOrientationSpaceIndex;
	QuatT              parentSpaceConcatenation;
	i32                mousePickPriority;
	bool               hidden;
	bool               changed;
	bool               alwaysXRay;
	bool               poseModifier;

	SElement()
		: id()
		, layer(0)
		, originalHandle(0)
		, action(ACTION_NONE)
		, shape(SHAPE_AXES)
		, hidden(false)
		, changed(false)
		, colorGroup(ELEMENT_COLOR_PROXY)
		, mousePickPriority(0)
		, alwaysXRay(false)
		, parentSpaceConcatenation(IDENTITY)
		, poseModifier(false)
	{
	}
};
typedef std::vector<SElement> SElements;

struct SElementData {};

struct IMouseDragHandler
{
	virtual bool Begin(const SMouseEvent& ev, Vec3 hitPoint) = 0;
	virtual void Update(const SMouseEvent& ev) = 0;
	virtual void Render(const SRenderContext& rc) {}
	virtual void End(const SMouseEvent& ev) = 0;
};

struct SSelectionSet
{
	SSelectionSet() {}
	SSelectionSet(ElementId id) { items.push_back(id); }
	void Add(ElementId elementId)
	{
		Remove(elementId);
		items.push_back(elementId);
		std::sort(items.begin(), items.end());
	}
	void Remove(ElementId elementId)  { items.erase(std::remove(items.begin(), items.end(), elementId), items.end()); }
	void Clear()                      { items.clear(); }
	bool IsEmpty() const              { return items.empty(); }
	bool Contains(ElementId id) const { return std::find(items.begin(), items.end(), id) != items.end(); }
	size_t Size() const               { return items.size(); }

	bool operator==(const SSelectionSet& rhs) const { return items == rhs.items; }
	bool operator!=(const SSelectionSet& rhs) const { return !operator==(rhs); }

	std::vector<ElementId> items;
};

struct ICommand {};

struct IElementTracer
{
	virtual bool HitRay(Vec3* intersectionPoint, const Ray& ray, const SElement& element) const = 0;
};

struct IElementDrawer
{
	virtual bool Draw(const SElement& element);
};

enum ETransformationSpace
{
	SPACE_WORLD,
	SPACE_LOCAL
};

enum ETransformationMode
{
	MODE_TRANSLATE,
	MODE_ROTATE,
	MODE_SCALE
};

struct SLookSettings
{
	ColorB proxyColor;
	ColorB proxySelectionColor;
	ColorB proxyHighlightColor;
	ColorB clothProxyColor;

	ColorB poseModifierColor;

	ColorB jointColor;
	ColorB jointHighlightColor;
	ColorB jointSelectionColor;

	SLookSettings()
		: proxyColor(126, 159, 243, 128)
		, proxySelectionColor(255, 255, 255, 128)
		, proxyHighlightColor(233, 255, 122, 128)
		, clothProxyColor(243, 159, 126, 128)
		, poseModifierColor(255, 0, 0, 255)
		, jointColor(0, 249, 48, 255)
		, jointHighlightColor(255, 248, 0, 128)
		, jointSelectionColor(255, 255, 255, 128)
	{
	}
};

struct ISpaceProvider
{
	virtual SSpaceAndIndex FindSpaceIndexByName(i32 spaceType, tukk name, i32 parentsUp) const = 0;
	virtual QuatT          GetTransform(const SSpaceAndIndex& index) const = 0;
};

class SANDBOX_API CScene : public QObject, public QViewportConsumer
{
	Q_OBJECT

public:
	enum EGizmoVisibilityFlags
	{
		PoseModifierGizmo = BIT(0),
		AttachmentGizmo   = BIT(1),
	};

public:
	CScene();
	~CScene();

	void                 OnViewportKey(const SKeyEvent& ev) override;
	void                 OnViewportMouse(const SMouseEvent& ev) override;
	void                 OnViewportRender(const SRenderContext& rc) override;

	void                 SetTransformationMode(ETransformationMode mode);
	ETransformationMode  TransformationMode() const;
	void                 SetTransformationSpace(ETransformationSpace space);
	ETransformationSpace TransformationSpace() const { return m_transformationSpace; }
	void                 SetVisibleLayerMask(u32 layerMask);
	u32         VisibleLayerMask() const    { return m_visibleLayerMask; }
	bool                 IsLayerVisible(i32 layer) const;

	void                 SetShowAttachementGizmos(const bool enabled)
	{
		if (enabled)
			m_gizmoVisibilityFlags |= AttachmentGizmo;
		else
			m_gizmoVisibilityFlags &= ~AttachmentGizmo;
	}
	void                 SetShowPoseModifierGizmos(const bool enabled)
	{
		if (enabled)
			m_gizmoVisibilityFlags |= PoseModifierGizmo;
		else
			m_gizmoVisibilityFlags &= ~PoseModifierGizmo;
	}

	void                 Clear();
	void                 ClearLayer(i32 layer);
	void                 AddElement(const SElement& element);
	void                 AddElement(const SElement& element, ElementId id);
	void                 ApplyToAll(EElementAction);
	void                 ApplyToSelection(EElementAction);
	void                 RefreshAllElements(); //!< Emit signal 'changed' to all elements in all layers


	void                 SetSpaceProvider(ISpaceProvider* spaceProvider);
	ISpaceProvider*      SpaceProvider() const { return m_spaceProvider; }
	QuatT                GetParentSpace(const SElement& e) const;
	QuatT                ElementToWorldSpace(const SElement& e) const;
	void                 WorldSpaceToElement(SElement* e, const QuatT& worldSpaceTransform);

	void                 SetCustomTracer(IElementTracer* tracer);

	const SElements&     Elements() const  { return m_elements; }
	SElements&           Elements()        { return m_elements; }
	void                 GetSelectedElements(SElements* elements) const;
	const SSelectionSet& Selection() const { return m_selection; }
	void                 SetSelection(const SSelectionSet& selection);
	void                 AddToSelection(const SSelectionSet& selection);
	void                 AddToSelection(ElementId elementId);

	bool                 SelectionCanBeMoved() const;
	bool                 SelectionCanBeRotated() const;
	QuatT                GetSelectionTransform(ETransformationSpace space) const;
	bool                 SetSelectionTransform(ETransformationSpace space, const QuatT& newTransform);

	void                 Serialize(Serialization::IArchive& ar);
signals:
	void                 SignalUndo();
	void                 SignalRedo();
	void                 SignalPushUndo(tukk description, ICommand* cause);
	void                 SignalElementsChanged(u32 layerBits);
	void                 SignalElementContinuousChange(u32 layerBits);
	void                 SignalPropertiesChanged();
	void                 SignalRenderElements(const SElements& elements, const SRenderContext& rc);
	void                 SignalSelectionChanged();
	void                 SignalManipulationModeChanged();

private:
	void UpdateElements(const SElements& elements);
	i32  GetSelectionCaps() const;
	Vec3 GetSelectionSize() const;
	bool SetSelectionSize(const Vec3& size);
	void OnMouseMove(const SMouseEvent& ev);

	struct SBlockSelectHandler;
	struct SMoveHandler;
	struct SRotationHandler;
	struct SScalingHandler;
	struct STransformBox;

	IElementTracer*             m_customTracer;
	IElementDrawer*             m_customDrawer;

	SSelectionSet               m_selection;
	unique_ptr<IMouseDragHandler> m_mouseDragHandler;
	ISpaceProvider*             m_spaceProvider;
	unique_ptr<CAxisHelper>       m_axisHelper;
	SElements                   m_elements;
	std::vector<ElementId>      m_lastIdByLayer;
	ETransformationMode         m_transformationMode;
	ETransformationSpace        m_transformationSpace;
	i32                         m_highlightItem;
	u32                m_visibleLayerMask;
	bool                        m_showGizmo;
	u32                m_gizmoVisibilityFlags;
	SLookSettings               m_lookSettings;
	i32                         m_highlightedItem;
	QuatT                       m_temporaryLocalDelta;
};

}

