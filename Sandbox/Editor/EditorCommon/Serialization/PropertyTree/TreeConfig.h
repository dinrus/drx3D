// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct TreeConfig
{
	TreeConfig();

	static TreeConfig defaultConfig;

	bool fullRowContainers;
	bool immediateUpdate;
	bool showContainerIndices;
	bool filterWhenType;
	float valueColumnWidth;
	i32 filter;
	i32 tabSize;
	i32 sliderUpdateDelay;
	i32 defaultRowHeight;

	i32 expandLevels;
	bool undoEnabled;
	bool fullUndo;
	bool multiSelection;
	// Whether we'll allow actions / buttons in the tree to be used
	bool enableActions;
};

