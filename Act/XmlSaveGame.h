// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// xml save game - primarily for debug purposes

#ifndef __XMLSAVEGAME_H__
#define __XMLSAVEGAME_H__

#pragma once

#include "ISaveGame.h"

class CXmlSaveGame : public ISaveGame
{
public:
	CXmlSaveGame();
	virtual ~CXmlSaveGame();

	// ISaveGame
	virtual bool            Init(tukk name);
	virtual void            AddMetadata(tukk tag, tukk value);
	virtual void            AddMetadata(tukk tag, i32 value);
	virtual u8*          SetThumbnail(u8k* imageData, i32 width, i32 height, i32 depth);
	virtual bool            SetThumbnailFromBMP(tukk filename);
	virtual TSerialize      AddSection(tukk section);
	virtual bool            Complete(bool successfulSoFar);
	virtual tukk     GetFileName() const;
	virtual void            SetSaveGameReason(ESaveGameReason reason) { m_eReason = reason; }
	virtual ESaveGameReason GetSaveGameReason() const                 { return m_eReason; }
	virtual void            GetMemoryUsage(IDrxSizer* pSizer) const;
	// ~ISaveGame

protected:
	virtual bool Write(tukk filename, XmlNodeRef data);
	XmlNodeRef   GetMetadataXmlNode() const;

private:
	struct Impl;
	std::unique_ptr<Impl> m_pImpl;
	ESaveGameReason       m_eReason;

};

#endif
