// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ILOADGAME_H__
#define __ILOADGAME_H__

#pragma once

struct ILoadGame
{
	virtual ~ILoadGame(){}
	// initialize - set name of game
	virtual bool                Init(tukk name) = 0;

	virtual IGeneralMemoryHeap* GetHeap() = 0;

	// get some basic meta-data
	virtual tukk                 GetMetadata(tukk tag) = 0;
	virtual bool                        GetMetadata(tukk tag, i32& value) = 0;
	virtual bool                        HaveMetadata(tukk tag) = 0;
	// create a serializer for some data section
	virtual std::unique_ptr<TSerialize> GetSection(tukk section) = 0;
	virtual bool                        HaveSection(tukk section) = 0;

	// finish - indicate success (negative success *must* remove file)
	// also calls delete this;
	virtual void Complete() = 0;

	// returns the filename of this savegame
	virtual tukk GetFileName() const = 0;
};

#endif
