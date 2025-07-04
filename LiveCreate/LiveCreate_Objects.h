// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _H_LIVECREATECOMMANDS_OBJECTS_H_
#define _H_LIVECREATECOMMANDS_OBJECTS_H_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <drx3D/LiveCreate/LiveCreateCommands.h>

#if !defined(NO_LIVECREATE)

namespace LiveCreate
{

//-----------------------------------------------------------------------------

class CLiveCreateCmd_SetEntityTransform : public ILiveCreateCommand
{
	DECLARE_REMOTE_COMMAND(CLiveCreateCmd_SetEntityTransform);

public:
	EntityGUID m_guid;
	u32     m_flags;
	Vec3       m_position;
	Quat       m_rotation;
	Vec3       m_scale;

public:
	template<class T>
	void Serialize(T& stream)
	{
		stream << m_guid;
		stream << m_flags;
		stream << m_position;
		stream << m_rotation;
		stream << m_scale;
	}

	virtual void Execute() LC_COMMAND;
};

//-----------------------------------------------------------------------------

class CLiveCreateCmd_EntityUpdate : public ILiveCreateCommand
{
	DECLARE_REMOTE_COMMAND(CLiveCreateCmd_EntityUpdate);

public:
	EntityGUID m_guid;
	u32     m_flags;
	string     m_data;

public:
	template<class T>
	void Serialize(T& stream)
	{
		stream << m_guid;
		stream << m_flags;
		stream << m_data;
	}

	virtual void Execute() LC_COMMAND;
};

//-----------------------------------------------------------------------------

class CLiveCreateCmd_EntityDelete : public ILiveCreateCommand
{
	DECLARE_REMOTE_COMMAND(CLiveCreateCmd_EntityDelete);

public:
	EntityGUID m_guid;
	u32     m_flags;

public:
	template<class T>
	void Serialize(T& stream)
	{
		stream << m_guid;
		stream << m_flags;
	}

	virtual void Execute() LC_COMMAND;
};

//-----------------------------------------------------------------------------

class CLiveCreateCmd_ObjectAreaUpdate : public ILiveCreateCommand
{
	DECLARE_REMOTE_COMMAND(CLiveCreateCmd_ObjectAreaUpdate);

public:
	std::vector<u8>  m_data;
	std::vector<string> m_layerNames;
	std::vector<u16> m_layerIds;

public:
	template<class T>
	void Serialize(T& stream)
	{
		stream << m_data;
		stream << m_layerNames;
		stream << m_layerIds;
	}

	virtual void Execute() LC_COMMAND;
};

//-----------------------------------------------------------------------------

class CLiveCreateCmd_UpdateLightParams : public ILiveCreateCommand
{
	DECLARE_REMOTE_COMMAND(CLiveCreateCmd_UpdateLightParams);

public:
	EntityGUID m_guid;

	// Selection of SRenderLight parameters that are sent
	u32 m_Flags;
	float  m_fRadius;
	ColorF m_Color;
	float  m_SpecMult;
	float  m_fHDRDynamic;
	u8  m_nAttenFalloffMax;
	u8  m_nSortPriority;
	float  m_fShadowBias;
	float  m_fShadowSlopeBias;
	float  m_fShadowResolutionScale;
	float  m_fShadowUpdateMinRadius;
	u16 m_nShadowMinResolution;
	u16 m_nShadowUpdateRatio;
	float  m_fBaseRadius;
	ColorF m_BaseColor;
	float  m_BaseSpecMult;

public:
	template<class T>
	void Serialize(T& stream)
	{
		stream << m_guid;
		stream << m_Flags;
		stream << m_fRadius;
		stream << m_Color;
		stream << m_SpecMult;
		stream << m_fHDRDynamic;
		stream << m_nAttenFalloffMax;
		stream << m_nSortPriority;
		stream << m_fShadowBias;
		stream << m_fShadowSlopeBias;
		stream << m_fShadowResolutionScale;
		stream << m_fShadowUpdateMinRadius;
		stream << m_nShadowMinResolution;
		stream << m_nShadowUpdateRatio;
		stream << m_fBaseRadius;
		stream << m_BaseColor;
		stream << m_BaseSpecMult;
	}

	virtual void Execute() LC_COMMAND;
};

//-----------------------------------------------------------------------------

class CLiveCreateCmd_EntityPropertyChange : public ILiveCreateCommand
{
	DECLARE_REMOTE_COMMAND(CLiveCreateCmd_EntityPropertyChange);

public:
	// must match IVariable definitions
	enum EType
	{
		eType_UNKNOWN,
		eType_INT,
		eType_BOOL,
		eType_FLOAT,
		eType_VECTOR2,
		eType_VECTOR,
		eType_VECTOR4,
		eType_QUAT,
		eType_STRING,
	};

	EntityGUID m_guid;
	string     m_name;
	u8      m_type;
	Vec4       m_valueVec4; // all floats + vectors
	i32      m_valueInt;  // i32 + bool
	string     m_valueString;

public:
	template<class T>
	void Serialize(T& stream)
	{
		stream << m_guid;
		stream << m_name;
		stream << m_type;

		switch (m_type)
		{
		case eType_BOOL:
		case eType_INT:
			{
				stream << m_valueInt;
				break;
			}

		case eType_FLOAT:
			{
				stream << m_valueVec4.x;
				break;
			}

		case eType_QUAT:
		case eType_VECTOR2:
		case eType_VECTOR:
		case eType_VECTOR4:
			{
				stream << m_valueVec4;
				break;
			}

		case eType_STRING:
			{
				stream << m_valueString;
				break;
			}

		default:
			{
				// no value serialized
			}
		}
	}

	virtual void Execute() LC_COMMAND;
};

//-----------------------------------------------------------------------------

class CLiveCreateCmd_EntitySetMaterial : public ILiveCreateCommand
{
	DECLARE_REMOTE_COMMAND(CLiveCreateCmd_EntitySetMaterial);

public:
	EntityGUID m_guid;
	string     m_materialName;

public:
	template<class T>
	void Serialize(T& stream)
	{
		stream << m_guid;
		stream << m_materialName;
	}

	virtual void Execute() LC_COMMAND;
};

//-----------------------------------------------------------------------------
}

#endif
#endif
