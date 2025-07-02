// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __IPOSTRENDERER_H__
#define __IPOSTRENDERER_H__

#pragma once

class IPostRenderer
{
public:
	IPostRenderer() : m_refCount(0){}

	virtual void OnPostRender() const = 0;

	void         AddRef()  { m_refCount++; }
	void         Release() { if (--m_refCount <= 0) delete this; }

protected:
	virtual ~IPostRenderer(){}

	i32 m_refCount;
};

#endif//__IPOSTRENDERER_H__

