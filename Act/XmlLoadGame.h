// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __XMLLOADGAME_H__
#define __XMLLOADGAME_H__

#pragma once

#include "ILoadGame.h"

class CXmlLoadGame : public ILoadGame
{
public:
	CXmlLoadGame();
	virtual ~CXmlLoadGame();

	// ILoadGame
	virtual bool                        Init(tukk name);
	virtual IGeneralMemoryHeap*         GetHeap() { return NULL; }
	virtual tukk                 GetMetadata(tukk tag);
	virtual bool                        GetMetadata(tukk tag, i32& value);
	virtual bool                        HaveMetadata(tukk tag);
	virtual std::unique_ptr<TSerialize> GetSection(tukk section);
	virtual bool                        HaveSection(tukk section);
	virtual void                        Complete();
	virtual tukk                 GetFileName() const;
	// ~ILoadGame

protected:
	bool Init(const XmlNodeRef& root, tukk fileName);

private:
	struct Impl;
	std::unique_ptr<Impl> m_pImpl;
};

#endif
