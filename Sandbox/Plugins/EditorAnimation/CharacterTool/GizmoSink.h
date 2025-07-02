// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <vector>
#include <Serialization/Decorators/IGizmoSink.h>
#include <drx3D/CoreX/Serialization/Decorators/LocalFrame.h>
#include "ManipScene.h"
#include "Explorer/Explorer.h"

namespace CharacterTool
{
using std::vector;

using namespace Explorer;

class CharacterDocument;
class CharacterSpaceProvider : public Manip::ISpaceProvider
{
public:
	CharacterSpaceProvider(CharacterDocument* document) : m_document(document) {}
	Manip::SSpaceAndIndex FindSpaceIndexByName(i32 spaceType, tukk name, i32 parentsUp) const override;
	QuatT                 GetTransform(const Manip::SSpaceAndIndex& si) const override;
private:
	CharacterDocument* m_document;
};

enum GizmoLayer
{
	GIZMO_LAYER_NONE = -1,
	GIZMO_LAYER_SCENE,
	GIZMO_LAYER_CHARACTER,
	GIZMO_LAYER_ANIMATION,
	GIZMO_LAYER_COUNT
};

class GizmoSink : public Serialization::IGizmoSink
{
public:
	GizmoSink() : m_lastIndex(0), m_scene(0) {}
	void           SetScene(Manip::CScene* scene) { m_scene = scene; }
	ExplorerEntry* ActiveEntry()                  { return m_activeEntry.get(); }

	void           Clear(GizmoLayer layer);
	void           BeginWrite(ExplorerEntry* entry, GizmoLayer layer);
	void           BeginRead(GizmoLayer layer);
	void           EndRead();

	i32            CurrentGizmoIndex() const override;
	i32            Write(const Serialization::LocalPosition& position, const Serialization::GizmoFlags& gizmoFlags, ukk handle) override;
	i32            Write(const Serialization::LocalOrientation& position, const Serialization::GizmoFlags& gizmoFlags, ukk handle) override;
	i32            Write(const Serialization::LocalFrame& position, const Serialization::GizmoFlags& gizmoFlags, ukk handle) override;
	bool           Read(Serialization::LocalPosition* position, Serialization::GizmoFlags* gizmoFlags, ukk handle) override;
	bool           Read(Serialization::LocalOrientation* position, Serialization::GizmoFlags* gizmoFlags, ukk handle) override;
	bool           Read(Serialization::LocalFrame* position, Serialization::GizmoFlags* gizmoFlags, ukk handle) override;
	void           SkipRead() override;
	void           Reset(ukk handle);
private:
	i32                       m_lastIndex;
	i32                       m_currentLayer;
	Manip::CScene*            m_scene;
	_smart_ptr<ExplorerEntry> m_activeEntry;
};

}

