// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

typedef u16 EntityClassId;

#include <drx3D/Entity/Stream.h>
#include <drx3D/Entity/IBitStream.h>         // <> required for Interfuscator

struct IEntityContainer;

template<i32 _max_size> class SafeString
{
public:
	SafeString()
	{
		memset(m_s, 0, _max_size);
	}
	SafeString& operator=(const string& s)
	{
		assert(s.length() < _max_size);
		drx_strcpy(m_s, s.c_str());
		return *this;
	}
	SafeString& operator=(tukk s)
	{
		assert(s);
		assert(strlen(s) < _max_size);
		drx_strcpy(m_s, s);
		return *this;
	}
	operator tukk () const
	{
		return m_s;
	}
	operator tukk ()
	{
		return m_s;
	}
	tukk c_str() const { return m_s; }
	tukk c_str()       { return m_s; }
	i32         length()      { return strlen(m_s); }
private:
	char m_s[_max_size];
};

class CStream;
struct IScriptTable;

/*!
   CEntityDecs class is an entity description.
   This class describes what kind of entity needs to be spawned, and is passed to entity system when an entity is spawned. This
   class is kept in the entity and can be later retrieved in order to (for example) clone an entity.
   @see IEntitySystem::SpawnEntity(CEntityDesc &)
   @see IEntity::GetEntityDesc()
 */
class CEntityDesc
{
public:

	i32           id;                //!< The net unique identifier (EntityId).
	SafeString<256> name;              //!< The name of the player. does not need to be unique.
	EntityClassId   ClassId;           //!< Player, weapon, or something else - the class id of this entity.
	SafeString<256> sModel;            //!< Specify a model for the player container.
	Vec3            vColor;            //!< Used for team coloring (0xffffff=default, coloring not used).
	bool            netPresence;       //!< This is filled out by container, defaults to ANY.
	SafeString<256> className;         //!< The name of the lua table corresponding to this entity.
	Vec3            pos;
	Ang3            angles;
	float           scale;
	uk           pUserData;         //!< Used during loading from XML.

	IScriptTable*   pProperties;
	IScriptTable*   pPropertiesInstance;
	~CEntityDesc(){};
	CEntityDesc();
	CEntityDesc(i32 id, const EntityClassId ClassId);
	CEntityDesc(const CEntityDesc& d) { *this = d; };
	CEntityDesc& operator=(const CEntityDesc& d);

	bool         Write(IBitStream* pIBitStream, CStream& stm);
	bool         Read(IBitStream* pIBitStream, CStream& stm);

	bool         IsDirty();
};

///////////////////////////////////////////////
inline CEntityDesc::CEntityDesc()
{
	className = "";
	id = 0;
	netPresence = true;
	ClassId = 0;
	sModel = "";
	pUserData = 0;
	pProperties = NULL;
	pPropertiesInstance = NULL;
	angles(0, 0, 0);
	pos(0, 0, 0);
	scale = 1;
	vColor = Vec3(1, 1, 1); //!< Default, colour not used.
}

///////////////////////////////////////////////
inline CEntityDesc::CEntityDesc(i32 _id, const EntityClassId _ClassId)
{
	className = "";
	id = _id;
	netPresence = true;
	ClassId = _ClassId;
	sModel = "";
	pUserData = 0;
	pProperties = NULL;
	pPropertiesInstance = NULL;
	angles(0, 0, 0);
	pos(0, 0, 0);
	scale = 1;
	vColor = Vec3(1, 1, 1); //!< Default, colour not used.
}

inline CEntityDesc& CEntityDesc::operator=(const CEntityDesc& d)
{
	className = d.className;
	id = d.id;
	netPresence = d.netPresence;
	ClassId = d.ClassId;
	sModel = d.sModel;
	pos = d.pos;
	angles = d.angles;
	pProperties = d.pProperties;
	pPropertiesInstance = d.pPropertiesInstance;
	vColor = d.vColor;
	scale = d.scale;
	return *this;
}

///////////////////////////////////////////////
inline bool CEntityDesc::Write(IBitStream* pIBitStream, CStream& stm)
{
	WRITE_COOKIE(stm);

	if (!pIBitStream->WriteBitStream(stm, id, eEntityId))
		return false;

	if (name.length())
	{
		stm.Write(true);
		stm.Write(name.c_str());
	}
	else
		stm.Write(false);

	if (!pIBitStream->WriteBitStream(stm, ClassId, eEntityClassId))
		return false;

	if (sModel.length())
	{
		stm.Write(true);
		stm.Write(sModel);
	}
	else
	{
		stm.Write(false);
	}
	if ((*((u32*)(&pos.x)) == 0)
	    && (*((u32*)(&pos.y)) == 0)
	    && (*((u32*)(&pos.z)) == 0))
	{
		stm.Write(false);
	}
	else
	{
		stm.Write(true);
		stm.Write(pos);
	}

	if (vColor != Vec3(1, 1, 1))
	{
		stm.Write(true);
		stm.Write((u8)(vColor.x * 255.0f));
		stm.Write((u8)(vColor.y * 255.0f));
		stm.Write((u8)(vColor.z * 255.0f));
	}
	else
		stm.Write(false);

	WRITE_COOKIE(stm);
	return true;
}

///////////////////////////////////////////////
inline bool CEntityDesc::Read(IBitStream* pIBitStream, CStream& stm)
{
	bool bModel, bName, bPos, bTeamColor;
	static char sTemp[250];
	VERIFY_COOKIE(stm);

	if (!pIBitStream->ReadBitStream(stm, id, eEntityId))
		return false;

	stm.Read(bName);
	if (bName)
	{
		stm.Read(sTemp, 250);
		name = sTemp;
	}

	if (!pIBitStream->ReadBitStream(stm, ClassId, eEntityClassId))
		return false;

	stm.Read(bModel);
	if (bModel)
	{
		stm.Read(sTemp, 250);
		sModel = sTemp;
	}

	stm.Read(bPos);
	if (bPos)
		stm.Read(pos);
	else
		pos = Vec3(0, 0, 0);

	stm.Read(bTeamColor);
	if (bTeamColor)
	{
		u8 x, y, z;
		stm.Read(x);
		stm.Read(y);
		stm.Read(z);

		vColor = Vec3(x / 255.0f, y / 255.0f, z / 255.0f);
	}
	else
		vColor = Vec3(1, 1, 1);

	VERIFY_COOKIE(stm);
	return true;
}

///////////////////////////////////////////////
inline bool CEntityDesc::IsDirty()
{
	return true;
}

//! \endcond