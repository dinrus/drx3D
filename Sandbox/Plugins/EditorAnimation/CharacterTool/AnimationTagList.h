// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Decorators/TagList.h>
#include <QObject>

namespace CharacterTool
{

class EditorCompressionPresetTable;
class EditorDBATable;
class AnimationTagList : public QObject, public ITagSource
{
	Q_OBJECT
public:
	AnimationTagList(EditorCompressionPresetTable* presets, EditorDBATable* dbaTable);

protected:
	void         AddRef() override  {}
	void         Release() override {}

	u32 TagCount(u32 group) const override;
	tukk  TagValue(u32 group, u32 index) const override;
	tukk  TagDescription(u32 group, u32 index) const override;
	tukk  GroupName(u32 group) const override;
	u32 GroupCount() const override;
protected slots:
	void         OnCompressionPresetsChanged();
	void         OnDBATableChanged();
private:
	EditorCompressionPresetTable* m_presets;
	EditorDBATable*               m_dbaTable;

	struct SEntry
	{
		string tag;
		string description;
	};

	struct SGroup
	{
		string              name;
		std::vector<SEntry> entries;
	};

	std::vector<SGroup> m_groups;
};

}

