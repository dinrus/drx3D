// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

class IObjectEnumerator
{
public:
	virtual void AddEntry(tukk path, tukk id, tukk sortStr = "") = 0;
	virtual void RemoveEntry(tukk path, tukk id) = 0;
	virtual void ChangeEntry(tukk path, tukk id, tukk sortStr = "") = 0;
};

