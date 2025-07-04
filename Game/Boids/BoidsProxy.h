// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   BoidsProxy.h
//  Version:     v1.00
//  Created:     2/10/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __BoidsProxy_h__
#define __BoidsProxy_h__
#pragma once

//#include <drx3D/Entity/IEntityProxy.h>

class CFlock;
class CBoidObject;

//////////////////////////////////////////////////////////////////////////
// Описание:
//    Handles sounds in the entity.
//////////////////////////////////////////////////////////////////////////
struct CBoidsProxy : public IEntityBoidsProxy
{
	CBoidsProxy();
	~CBoidsProxy();
	IEntity* GetEntity() const { return m_pEntity; };

	// IComponent interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void Initialize( const SComponentInitializer& init );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// IEntityProxy interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual EEntityProxy GetType() { return ENTITY_PROXY_BOIDS; }
	virtual void Release();
	virtual void Done() {};
	virtual	void Update( SEntityUpdateContext &ctx );
	virtual	void ProcessEvent( SEntityEvent &event );
	virtual bool Init( IEntity *pEntity,SEntitySpawnParams &params ) { return true; }
	virtual void Reload( IEntity *pEntity,SEntitySpawnParams &params );
	virtual void SerializeXML( XmlNodeRef &entityNode,bool bLoading ) {};
	virtual void Serialize( TSerialize ser );
	virtual bool NeedSerialize() { return false; };
	virtual bool GetSignature( TSerialize signature );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void SetFlock( CFlock *pFlock );
	CFlock* GetFlock() { return m_pFlock; }
	void OnTrigger( bool bEnter,SEntityEvent &event );

	virtual void GetMemoryUsage(IDrxSizer *pSizer )const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_pFlock);
	}
private:
	void OnMove();

private:
	//////////////////////////////////////////////////////////////////////////
	// Private member variables.
	//////////////////////////////////////////////////////////////////////////
	// Host entity.
	IEntity *m_pEntity;

	// Flock of items.
	CFlock *m_pFlock;

	i32 m_playersInCount;
};

DECLARE_COMPONENT_POINTERS( CBoidsProxy );

//////////////////////////////////////////////////////////////////////////
// Описание:
//    Handles sounds in the entity.
//////////////////////////////////////////////////////////////////////////
struct CBoidObjectProxy : public IEntityProxy
{
	CBoidObjectProxy();
	~CBoidObjectProxy();
	IEntity* GetEntity() const { return m_pEntity; };

	//////////////////////////////////////////////////////////////////////////
	// IEntityEvent interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void Initialize( const SComponentInitializer& init );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// IEntityProxy interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual EEntityProxy GetType() { return ENTITY_PROXY_BOID_OBJECT; }
	virtual void Release() { delete this; };
	virtual void Done() {};
	virtual	void Update( SEntityUpdateContext &ctx ){};
	virtual	void ProcessEvent( SEntityEvent &event );
	virtual bool Init( IEntity *pEntity,SEntitySpawnParams &params ) { return true; }
	virtual void Reload( IEntity *pEntity,SEntitySpawnParams &params ) {};
	virtual void SerializeXML( XmlNodeRef &entityNode,bool bLoading ) {};
	virtual void Serialize( TSerialize ser );
	virtual bool NeedSerialize() { return false; };
	virtual bool GetSignature( TSerialize signature );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void SetBoid( CBoidObject *pBoid ) { m_pBoid = pBoid; };
	CBoidObject* GetBoid() { return m_pBoid; }

	virtual void GetMemoryUsage(IDrxSizer *pSizer )const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_pBoid);
	}
private:
	//////////////////////////////////////////////////////////////////////////
	// Private member variables.
	//////////////////////////////////////////////////////////////////////////
	// Host entity.
	IEntity *m_pEntity;
	// Host Flock.
	CBoidObject *m_pBoid;
};

DECLARE_COMPONENT_POINTERS( CBoidObjectProxy );

#endif //__BoidsProxy_h__
