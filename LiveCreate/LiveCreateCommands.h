// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _H_LIVECREATECOMMANDS_H_
#define _H_LIVECREATECOMMANDS_H_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#if !defined(NO_LIVECREATE)

	#include <drx3D/Network/IServiceNetwork.h>
	#include <drx3D/Network/IRemoteCommand.h>
	#include <drx3D/CoreX/Game/IGameFramework.h>
	#include <drx3D/LiveCreate/IViewSystem.h>
	#include <drx3D/Entity/IEntitySystem.h>
	#include <drx3D/Act/IActorSystem.h>
	#include <drx3D/LiveCreate/IGameRulesSystem.h>
	#include <drx3D/Act/ILevelSystem.h>
	#include <drx3D/LiveCreate/LiveCreateHost.h>

//------------------------------------------------------------------------------------

	#define DECLARE_LIVECREATE_COMMAND(x) \
	  DECLARE_REMOTE_COMMAND(x)

	#define IMPLEMENT_LIVECREATE_COMMAND(x) \
	  IRemoteCommandClass & GetCommandClass ## x() { return x::GetStaticClass(); }

	#define REGISTER_LIVECREATE_COMMAND(x)                \
	  extern IRemoteCommandClass &GetCommandClass ## x(); \
	  gEnv->pRemoteCommandUpr->RegisterCommandClass(GetCommandClass ## x());

//------------------------------------------------------------------------------------

// serialization helper for vector
template<class T>
ILINE T& operator<<(T& stream, Vec3& v)
{
	stream << v.x;
	stream << v.y;
	stream << v.z;
	return stream;
}

// serialization helper for vector
template<class T>
ILINE T& operator<<(T& stream, Vec4& v)
{
	stream << v.x;
	stream << v.y;
	stream << v.z;
	stream << v.w;
	return stream;
}

// serialization helper for quaternion
template<class T>
ILINE T& operator<<(T& stream, Quat& q)
{
	stream << q.v.x;
	stream << q.v.y;
	stream << q.v.z;
	stream << q.w;
	return stream;
}

// serialization helper for color
template<class T>
ILINE T& operator<<(T& stream, ColorF& c)
{
	stream << c.r;
	stream << c.g;
	stream << c.b;
	stream << c.a;
	return stream;
}

// serialization helper for matrix
template<class T>
ILINE T& operator<<(T& stream, Matrix33& m)
{
	stream << m.m00 << m.m01 << m.m02;
	stream << m.m10 << m.m11 << m.m12;
	stream << m.m20 << m.m21 << m.m22;
	return stream;
}

// serialization helper for matrix
template<class T>
ILINE T& operator<<(T& stream, Matrix34& m)
{
	stream << m.m00 << m.m01 << m.m02 << m.m03;
	stream << m.m10 << m.m11 << m.m12 << m.m13;
	stream << m.m20 << m.m21 << m.m22 << m.m23;
	return stream;
}

// serialization helper for matrix
template<class T>
ILINE T& operator<<(T& stream, Matrix44& m)
{
	stream << m.m00 << m.m01 << m.m02 << m.m03;
	stream << m.m10 << m.m11 << m.m12 << m.m13;
	stream << m.m20 << m.m21 << m.m22 << m.m23;
	stream << m.m30 << m.m31 << m.m32 << m.m33;
	return stream;
}

//------------------------------------------------------------------------------------

namespace LiveCreate
{

struct SFullSyncEntityHeader
{
	u32 m_id;
	u32 m_crc;
	u32 m_layerId;
	string m_layerName;
	Vec3   m_position;
	Quat   m_rotation;
	Vec3   m_scale;

	template<class T>
	friend T& operator<<(T& stream, SFullSyncEntityHeader& entity)
	{
		stream << entity.m_id;
		stream << entity.m_crc;
		stream << entity.m_layerId;
		stream << entity.m_layerName;
		stream << entity.m_position;
		stream << entity.m_rotation;
		stream << entity.m_scale;
		return stream;
	}
};

// Base class for all LiveCreate commands
// Contains static methods for accessing various engine interfaces
struct ILiveCreateCommand : public IRemoteCommand
{
public:
	enum EFlags
	{
		eFlag_HasCRC = 1 << 0, // Valid CRC information is passed
		eFlag_Light  = 1 << 1, // Object is a light
	};

	ILINE static IView* GetActiveView()
	{
		IViewSystem* pViewSystem = GetViewSystem();
		if (NULL != pViewSystem)
		{
			return pViewSystem->GetActiveView();
		}

		return NULL;
	}

	ILINE static IViewSystem* GetViewSystem()
	{
		if (gEnv->pGameFramework)
		{
			return gEnv->pGameFramework->GetIViewSystem();
		}

		return NULL;
	}

	ILINE static IEntity* GetCameraEntity()
	{
		IView* pView = GetActiveView();
		if (NULL != pView)
		{
			return gEnv->pEntitySystem->GetEntity(pView->GetLinkedId());
		}

		return NULL;
	}

	ILINE static IEntity* GetPlayerEntity()
	{
		if (gEnv->pGameFramework)
		{
			IActor* pPlayer = gEnv->pGameFramework->GetClientActor();
			if (NULL != pPlayer)
			{
				return pPlayer->GetEntity();
			}
		}

		return NULL;
	}

	ILINE static ILightSource* FindLightSource(EntityGUID entityId)
	{
		// slow, linear search
		const PodArray<ILightSource*>* allLights = gEnv->p3DEngine->GetLightEntities();
		if (allLights != NULL)
		{
			for (u32 i = 0; i < allLights->Size(); ++i)
			{
				ILightSource* pLight = allLights->GetAt(i);
				if (pLight->GetLightProperties().m_nEntityId == entityId)
				{
					return pLight;
				}
			}
		}

		// not found
		return NULL;
	}
};

//------------------------------------------------------------------------------------

	#ifdef NO_LIVECREATE_COMMAND_IMPLEMENTATION
		#define LC_COMMAND {}
	#else
		#define LC_COMMAND
	#endif

//------------------------------------------------------------------------------------

/// Inplace memory buffer reader
class CLiveCreateInplaceReader : public IDataReadStream
{
private:
	u32k m_size;
	u8k* m_pData;
	u32       m_offset;

public:
	CLiveCreateInplaceReader(ukk pData, u32k size)
		: m_size(size)
		, m_offset(0)
		, m_pData(static_cast<u8k*>(pData))
	{
	}

	virtual void        Delete()                { /* this object is created on stack*/ }
	virtual void        Skip(u32k size) { m_offset += size; }
	virtual void        Read8(uk pData)      { Read(pData, 8); SwapEndian(*reinterpret_cast<uint64*>(pData)); }
	virtual void        Read4(uk pData)      { Read(pData, 4); SwapEndian(*reinterpret_cast<u32*>(pData)); }
	virtual void        Read2(uk pData)      { Read(pData, 2); SwapEndian(*reinterpret_cast<u16*>(pData)); }
	virtual void        Read1(uk pData)      { return Read(pData, 1); }
	virtual ukk GetPointer()            { return m_pData + m_offset; };

	virtual void        Read(uk pData, u32k size)
	{
		memcpy(pData, m_pData + m_offset, size);
		m_offset += size;
	}
};

//------------------------------------------------------------------------------------

}

#endif
#endif
