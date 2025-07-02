// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 04:05:2012: Created by Stan Fichele

*************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GamePhysicsSettings.h>
#include <drx3D/CoreX/BitFiddling.h>
#ifdef GAME_PHYS_DEBUG
#include <drx3D/Game/Utility/DrxWatch.h>
#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#endif //GAME_PHYS_DEBUG

AUTOENUM_BUILDNAMEARRAY(s_collision_class_names, COLLISION_CLASSES);
AUTOENUM_BUILDNAMEARRAY(s_game_collision_class_names, GAME_COLLISION_CLASSES);

i32k k_num_collision_classes = DRX_ARRAY_COUNT(s_collision_class_names);
i32k k_num_game_collision_classes = DRX_ARRAY_COUNT(s_game_collision_class_names);;

tukk CGamePhysicsSettings::GetCollisionClassName(u32 bitIndex)
{
	return (bitIndex<MAX_COLLISION_CLASSES) ? m_names[bitIndex] : "";
}


i32 CGamePhysicsSettings::GetBit(u32 a)
{
	i32 bit = IntegerLog2(a);
	bool valid = a!=0 && ((a-1)&a)==0;
	return valid ? bit : MAX_COLLISION_CLASSES;
}

void CGamePhysicsSettings::Init()
{
	COMPILE_TIME_ASSERT((k_num_collision_classes+k_num_game_collision_classes)<=MAX_COLLISION_CLASSES);

	// Automatically construct a list of string names for the collision clas enums

	for (i32 i=0; i<MAX_COLLISION_CLASSES; i++)
	{
		m_names[i] = "";
		m_classIgnoreMap[i] = 0;
	}

	#define GP_ASSIGN_NAME(a,...) m_names[GetBit(a)] = #a;
	#define GP_ASSIGN_NAMES(list) list(GP_ASSIGN_NAME)
	GP_ASSIGN_NAMES(COLLISION_CLASSES);

	#undef GP_ASSIGN_NAME
	#define GP_ASSIGN_NAME(a,b,...) m_names[GetBit(b)] = #a;
	GP_ASSIGN_NAMES(GAME_COLLISION_CLASSES);

	// Set up the default ignore flags.
	SetIgnoreMap(gcc_player_all, gcc_ragdoll);
	SetIgnoreMap(gcc_vtol, collision_class_terrain);
}

void CGamePhysicsSettings::ExportToLua()
{
	// Export enums to lua and construct a global table g_PhysicsCollisionClass
	
	IScriptSystem * pScriptSystem = gEnv->pScriptSystem;
	IScriptTable* physicsCollisionClassTable = pScriptSystem->CreateTable();
	physicsCollisionClassTable->AddRef();
	physicsCollisionClassTable->BeginSetGetChain();	
	for (i32 i=0; i<MAX_COLLISION_CLASSES; i++)
	{
		if (m_names[i][0])
		{
			COMPILE_TIME_ASSERT(MAX_COLLISION_CLASSES <= 23); //LUA can't support flags beyond bit 23 due to using floats

			stack_string name;
			name.Format("bT_%s", m_names[i]);  // Annoyingly we need to prefix with a b to make it a bool in lua
			physicsCollisionClassTable->SetValueChain(name.c_str(), 1<<i);
		}
	}
	physicsCollisionClassTable->EndSetGetChain();
	pScriptSystem->SetGlobalValue("g_PhysicsCollisionClass", physicsCollisionClassTable);
	physicsCollisionClassTable->Release();
	
	for (i32 i=0; i<MAX_COLLISION_CLASSES; i++)
	{
		if (m_names[i][0])
			pScriptSystem->SetGlobalValue(m_names[i], 1<<i);
	}

#undef GP_ASSIGN_NAME
#undef GP_ASSIGN_NAMES

#define GP_ASSIGN_NAMES(list) list(GP_ASSIGN_NAME)
#define GP_ASSIGN_NAME(a,b,...) pScriptSystem->SetGlobalValue(#a, b);
	GP_ASSIGN_NAMES(GAME_COLLISION_CLASS_COMBOS);

#undef GP_ASSIGN_NAME
#undef GP_ASSIGN_NAMES

}

void CGamePhysicsSettings::AddIgnoreMap( u32 gcc_classTypes, u32k ignoreClassTypesOR, u32k ignoreClassTypesAND )
{
	for(i32 i=0; i<MAX_COLLISION_CLASSES && gcc_classTypes; i++,gcc_classTypes>>=1)
	{
		if(gcc_classTypes&0x1)
		{
			m_classIgnoreMap[i] |= ignoreClassTypesOR;
			m_classIgnoreMap[i] &= ignoreClassTypesAND;
		}
	}
}

void CGamePhysicsSettings::SetIgnoreMap( u32 gcc_classTypes, u32k ignoreClassTypes )
{
	AddIgnoreMap( gcc_classTypes, ignoreClassTypes, ignoreClassTypes );
}

void CGamePhysicsSettings::SetCollisionClassFlags( IPhysicalEntity& physEnt, u32 gcc_classTypes, u32k additionalIgnoreClassTypesOR /*= 0*/, u32k additionalIgnoreClassTypesAND /*= 0xFFFFFFFF */ )
{
	u32k defaultIgnores = GetIgnoreTypes(gcc_classTypes); 
	pe_params_collision_class gcc_params;
	gcc_params.collisionClassOR.type = gcc_params.collisionClassAND.type = gcc_classTypes;
	gcc_params.collisionClassOR.ignore = gcc_params.collisionClassAND.ignore = (defaultIgnores|additionalIgnoreClassTypesOR)&additionalIgnoreClassTypesAND;
	physEnt.SetParams(&gcc_params);
}

void CGamePhysicsSettings::AddCollisionClassFlags( IPhysicalEntity& physEnt, u32 gcc_classTypes, u32k additionalIgnoreClassTypesOR /*= 0*/, u32k additionalIgnoreClassTypesAND /*= 0xFFFFFFFF */ )
{
	u32k defaultIgnores = GetIgnoreTypes(gcc_classTypes); 
	pe_params_collision_class gcc_params;
	gcc_params.collisionClassOR.type = gcc_classTypes;
	gcc_params.collisionClassOR.ignore = defaultIgnores|additionalIgnoreClassTypesOR;
	gcc_params.collisionClassAND.ignore = additionalIgnoreClassTypesAND;
	physEnt.SetParams(&gcc_params);
}

u32 CGamePhysicsSettings::GetIgnoreTypes( u32 gcc_classTypes ) const
{
	u32 ignoreTypes = 0;
	for(i32 i=0; i<MAX_COLLISION_CLASSES && gcc_classTypes; i++,gcc_classTypes>>=1)
	{
		if(gcc_classTypes&0x1)
		{
			ignoreTypes |= m_classIgnoreMap[i];
		}
	}
	return ignoreTypes;
}

void CGamePhysicsSettings::Debug( const IPhysicalEntity& physEnt, const bool drawAABB ) const
{
#ifdef GAME_PHYS_DEBUG
	IRenderAuxGeom* pRender = gEnv->pRenderer->GetIRenderAuxGeom();
	if(drawAABB && !pRender)
		return;

	pe_params_collision_class gcc;
	if(physEnt.GetParams(&gcc))
	{
		// NAME:
		i32k iForeign = physEnt.GetiForeignData();
		uk  const pForeign = physEnt.GetForeignData(iForeign);
		static i32k bufLen = 256;
		char buf[bufLen] = "Unknown";
		switch(iForeign)
		{
		case PHYS_FOREIGN_ID_ENTITY:
			{
				if(IEntity* pEntity=(IEntity*)physEnt.GetForeignData(PHYS_FOREIGN_ID_ENTITY))
				{
					drx_strcpy(buf, pEntity->GetName());
				}
			}
			break;
		default:
			break;
		}

		DrxWatch("===============");
		DrxWatch("PhysEnt[%s]", buf);
		
		ToString(gcc.collisionClassOR.type,&buf[0],bufLen);
		DrxWatch("CollisionClass[ %s ]", buf);

		ToString(gcc.collisionClassOR.ignore,&buf[0],bufLen);
		DrxWatch("Ignoring[ %s ]", buf);

		DrxWatch("===============");

		pe_status_pos entpos;
		if(physEnt.GetStatus(&entpos))
		{
			pe_status_nparts np;
			i32k numParts = physEnt.GetStatus(&np);
			for(i32 p=0; p<numParts; p++)
			{
				if(drawAABB)
				{
					pe_status_pos sp;
					sp.ipart = p;
					sp.flags |= status_local;
					if(physEnt.GetStatus(&sp))
					{
						if(IGeometry* pGeom = sp.pGeomProxy ? sp.pGeomProxy : sp.pGeom)
						{
							OBB obb;
							primitives::box lbbox;
							pGeom->GetBBox(&lbbox);
							const Vec3 center = entpos.pos + (entpos.q * (sp.pos + (sp.q * (lbbox.center*sp.scale))));
							obb.c.zero();
							obb.h = lbbox.size * sp.scale * entpos.scale;
							obb.m33 = Matrix33(entpos.q*sp.q) * lbbox.Basis.GetTransposed();

							pRender->DrawOBB( obb, center, false, ColorB(40, 200, 40), eBBD_Faceted);

							const float distSqr = gEnv->pRenderer->GetCamera().GetPosition().GetSquaredDistance(center);
							const float drawColor[4] = {0.15f, 0.8f, 0.15f, clamp_tpl(1.f-((distSqr-100.f)/(10000.f-100.f)), 0.f, 1.f)};
							drx_sprintf(buf,"%d",p);
							gEnv->pRenderer->DrawLabelEx(center, 1.2f, drawColor, true, true, "%s", &buf[0]);
						}
					}
				}

				pe_params_part pp;
				pp.ipart = p;
				if(physEnt.GetParams(&pp))
				{
					DrxWatch("PART %d", p);
					u32k flagsSelf = pp.flagsOR;
					buf[0]=0;
					for(u32 i=0, first=1; i<32; i++)
					{
						if((1<<i)&flagsSelf)
						{
							if(first) { buf[0]=0; }
							drx_sprintf(buf, "%s%s%d", buf, first?"":" + ", i );
							first=0;
						}
					}
					DrxWatch(" Flags[ %s ]", buf);

					u32k flagsColl = pp.flagsColliderOR;
					buf[0]=0;
					for(u32 i=0, first=1; i<32; i++)
					{
						if((1<<i)&flagsColl)
						{
							if(first) { buf[0]=0; }
							drx_sprintf(buf, "%s%s%d", buf, first?"":"+", i );
							first=0;
						}
					}
					DrxWatch(" FlagsColl[ %s ]", buf);
				}
			}
		}
	}
	if(drawAABB)
	{
		pe_params_bbox bbox;
		if(physEnt.GetParams(&bbox))
		{
			pRender->DrawAABB( AABB(bbox.BBox[0],bbox.BBox[1]), Matrix34(IDENTITY), false, ColorB(255, 0, 0), eBBD_Faceted);
		}
	}
#endif //GAME_PHYS_DEBUG
}

i32 CGamePhysicsSettings::ToString( u32 gcc_classTypes, tuk buf, i32k len, const bool trim /*= true*/ ) const
{
	i32 count = 0;
#ifdef GAME_PHYS_DEBUG
	drx_strcpy(buf, len, "None");
	for(i32 i=0; i<MAX_COLLISION_CLASSES && gcc_classTypes; i++,gcc_classTypes>>=1)
	{
		if(gcc_classTypes&0x1)
		{
			count++;
			tukk const name = m_names[i];
			size_t trimOffset = 0;
			if(trim)
			{
				tukk find;
				if(find=strstr(name,"gcc_"))
				{
					trimOffset = 4;
				}
				else if(find=strstr(name,"collision_class_"))
				{
					trimOffset = 16;
				}
			}
			drx_sprintf(buf, len, "%s%s%s", count>1?buf:"", count>1?" + ":"", &name[trimOffset] );
		}
	}
#endif //GAME_PHYS_DEBUG
	return count;
}
