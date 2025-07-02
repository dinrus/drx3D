// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ISAVEGAME_H__
#define __ISAVEGAME_H__

#pragma once

#include <drx3D/Network/SerializeFwd.h>

struct ISaveGame
{
	virtual ~ISaveGame(){}
	// initialize - set output path
	virtual bool Init(tukk name) = 0;

	// set some basic meta-data
	virtual void       AddMetadata(tukk tag, tukk value) = 0;
	virtual void       AddMetadata(tukk tag, i32 value) = 0;
	// create a serializer for some data section
	virtual TSerialize AddSection(tukk section) = 0;
	// set a thumbnail.
	// if imageData == 0: only reserves memory and returns ptr to local data
	// if imageData != 0: copies data from imageData into local buffer
	// imageData is in BGR or BGRA
	// returns ptr to internal data storage (size=width*height*depth) if Thumbnail supported,
	// 0 otherwise
	virtual u8* SetThumbnail(u8k* imageData, i32 width, i32 height, i32 depth) = 0;

	// set a thumbnail from an already present bmp file
	// file will be read on function call
	// returns true if successful, false otherwise
	virtual bool SetThumbnailFromBMP(tukk filename) = 0;

	// finish - indicate success (negative success *must* remove file)
	// also calls delete this;
	virtual bool Complete(bool successfulSoFar) = 0;

	// returns the filename of this savegame
	virtual tukk GetFileName() const = 0;

	// save game reason
	virtual void            SetSaveGameReason(ESaveGameReason reason) = 0;
	virtual ESaveGameReason GetSaveGameReason() const = 0;

	virtual void            GetMemoryUsage(IDrxSizer* pSizer) const = 0;
};

#endif
