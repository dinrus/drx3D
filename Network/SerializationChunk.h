// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SERIALIZATIONCHUNK_H__
#define __SERIALIZATIONCHUNK_H__

#pragma once

#include <drx3D/Network/Config.h>
#include <drx3D/Network/INetwork.h>

class CNetOutputSerializeImpl;
class CNetInputSerializeImpl;
class CByteInputStream;
class CByteOutputStream;
struct ICompressionPolicy;
typedef ICompressionPolicy* ICompressionPolicyPtr;

enum
{
	eO_NoOp = -1,
#define SERIALIZATION_TYPE(T) eO_ ## T,
#include <drx3D/Network/SerializationTypes.h>
#undef SERIALIZATION_TYPE
	eO_String,
	eO_OptionalGroup
};

typedef i16 EOps;

template<class T> struct TypeToId;
#define SERIALIZATION_TYPE(T) template<> struct TypeToId<T> { static const EOps type = eO_ ## T; };
#include <drx3D/Network/SerializationTypes.h>
#undef SERIALIZATION_TYPE

typedef u16 ChunkID;
static const ChunkID InvalidChunkID = ~ChunkID(0);

class CSerializationChunk : public CMultiThreadRefCount
{
public:
	CSerializationChunk();
	~CSerializationChunk();

	void Reset();

	// Описание:
	//     initialize the serialization chunks for some aspect in some profile for some entity
	//     works by requesting that aspect/profile to be serialized and remembering what operations were performed
	//     => we cannot change the format of serialization after this point
	bool Init(IGameContext* pCtx, EntityId id, NetworkAspectType nAspect, u8 nProfile, NetworkAspectType noCompression);

	// pull some data from the game into a byte buffer
	bool               FetchFromGame(IGameContext* pCtx, CByteOutputStream& output, EntityId id, NetworkAspectType nAspect, ChunkID chunkID, u8 nProfile) const;
	// push some data from a byte buffer into the game
	ESynchObjectResult UpdateGame(IGameContext* pCtx, CByteInputStream& input, EntityId id, NetworkAspectType nAspect, ChunkID chunkID, u8 nProfile, bool& wasPartialUpdate) const;
	// byte stream -> compressed stream
	void               EncodeToStream(CByteInputStream& input, CNetOutputSerializeImpl& output, ChunkID chunkID, u8 nProfile) const;
	// compressed stream -> byte stream (to be pumped into the game later)
	void               DecodeFromStream(CNetInputSerializeImpl& input, CByteOutputStream& output, ChunkID chunkID, u8 nProfile) const;

	bool               IsEmpty() const;

	bool               operator<(const CSerializationChunk& rhs) const;

	void               GetMemoryStatistics(IDrxSizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "CSerializationChunk");

		pSizer->Add(*this);
		pSizer->AddContainer(m_ops);
	}

#if ENABLE_DEBUG_KIT
	void Dump(EntityId eid, u32 id);
#endif

private:
	class CToBufferImpl;
	class CFromBufferImpl;
	class CBuildImpl;

	// a single serialization 'op-code'
	struct SOp
	{
		EOps                  type;
		i16                 skipFalse;
		ICompressionPolicyPtr pPolicy;
#if TRACK_ENCODING // only store name of attribute when we're debugging (it's not used for anything but display purposes)
		string                name;
#endif
	};
	typedef std::vector<SOp> TOps;
	TOps  m_ops;
#if CRC8_ASPECT_FORMAT
	u8 m_crc;
#endif
	bool  m_overrideCompression;
};

typedef _smart_ptr<CSerializationChunk> CSerializationChunkPtr;

#endif
