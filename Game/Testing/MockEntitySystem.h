// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
*************************************************************************/

#ifndef MOCKENTITYSYSTEM_H
#define MOCKENTITYSYSTEM_H


#include <EngineFacade/EngineFacade.h>


class CMockEnginePhysicalEntity: public EngineFacade::CNullEnginePhysicalEntity
{
public:
	CMockEnginePhysicalEntity();

	virtual void SetParamsResult(i32 result);
	virtual i32 SetParams(pe_params* params);

	virtual void SetReturnParams(pe_player_dynamics params);
	virtual i32 GetParams(pe_params* params);

	virtual void Action(pe_action* action);
	i32 GetActionCount();

private:
	i32 setParamsResult;
	pe_player_dynamics setReturnParams;
	i32 m_actionCounter;
};


class CMockEngineEntity: public EngineFacade::CNullEngineEntity
{
public:
	CMockEngineEntity();

	virtual EngineFacade::IEnginePhysicalEntity& GetPhysicalEntity();

	virtual const Matrix34& GetSlotWorldTM( i32 nSlot ) const;
	virtual const Matrix34& GetWorldTM() const;
	virtual Ang3 GetWorldAngles() const;
	virtual	Vec3 GetWorldPos() const;
	virtual void SetWorldPos(Vec3 worldPos);

	virtual Quat GetWorldRotation() const;
	virtual void SetWorldRotation(Quat rotation);

private:
	CMockEnginePhysicalEntity m_mockedPhysicalEntity;

	Vec3 m_worldPos;
	Quat m_worldRotation;
	Ang3 m_worldAngles;
	Matrix34 m_WorldMatrix;
};


class CMockEntitySystem: public EngineFacade::CEngineEntitySystem
{
public:
	CMockEntitySystem();

	virtual EngineFacade::IEngineEntity::Ptr GetEntityByID(EntityId id);

    void Use(EngineFacade::IEngineEntity::Ptr entity);

private:
	EngineFacade::IEngineEntity::Ptr mockedEntity;
};


#endif 
