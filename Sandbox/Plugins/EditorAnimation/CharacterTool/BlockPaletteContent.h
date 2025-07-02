// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <vector>
#include "Serialization.h"

struct BlockPaletteItem
{
	string name;
	ColorB color;
	union
	{
		long long userId;
		uk     userPointer;
	};
	std::vector<char> userPayload;

	BlockPaletteItem()
		: color(255, 255, 255, 255)
		, userId(0)
	{
	}

	void Serialize(Serialization::IArchive& ar)
	{
		ar(name, "name", "Name");
		ar(color, "color", "Color");
		ar(userId, "userId", "UserID");
	}
};
typedef std::vector<BlockPaletteItem> BlockPaletteItems;

struct BlockPaletteContent
{
	BlockPaletteItems items;

	i32               lineHeight;
	i32               padding;

	BlockPaletteContent()
		: lineHeight(24)
		, padding(2)
	{
	}

	void Serialize(Serialization::IArchive& ar)
	{
		ar(lineHeight, "lineHeight", "Line Height");
		ar(padding, "padding", "Padding");
		ar(items, "items", "Items");
	}
};

