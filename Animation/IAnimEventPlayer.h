// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/DrxExtension.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

struct AnimEventInstance;
struct ICharacterInstance;
struct SRendParams;
struct SRenderingPassInfo;

//! Defines parameters that are used by a custom event type.
enum EAnimEventParameters
{
	ANIM_EVENT_USES_BONE                 = 1 << 0,
	ANIM_EVENT_USES_BONE_INLINE          = 1 << 1,
	ANIM_EVENT_USES_OFFSET_AND_DIRECTION = 1 << 2,
	ANIM_EVENT_USES_ALL_PARAMETERS       = ANIM_EVENT_USES_BONE | ANIM_EVENT_USES_OFFSET_AND_DIRECTION
};

struct SCustomAnimEventType
{
	tukk name;
	i32         parameters;   //!< EAnimEventParameters.
	tukk description;  //!< Used for tooltips in Character Tool.
};

//! Allows to preview different kinds of animevents within Character Tool.
struct IAnimEventPlayer : public IDrxUnknown
{
	DRXINTERFACE_DECLARE_GUID(IAnimEventPlayer, "2e2f7475-5424-47f3-b672-9edb4a3af495"_drx_guid);

	//! Can be used to customize parameter type for editing.
	virtual bool Play(ICharacterInstance* character, const AnimEventInstance& animEvent) = 0;
	virtual void Initialize() {}

	//! Used when animation is being rewound.
	virtual void Reset()                                                                                          {}
	virtual void Update(ICharacterInstance* character, const QuatT& playerPosition, const Matrix34& cameraMatrix) {}
	virtual void Render(const QuatT& playerPosition, SRendParams& params, const SRenderingPassInfo& passInfo)     {}
	virtual void EnableAudio(bool enable)                                                                         {}
	virtual void Serialize(Serialization::IArchive& ar)                                                           {}

	//! Allows to customize editing of custom parameter of AnimEvent.
	virtual tukk                 SerializeCustomParameter(tukk parameterValue, Serialization::IArchive& ar, i32 customTypeIndex) { return ""; }
	virtual const SCustomAnimEventType* GetCustomType(i32 customTypeIndex) const                                                               { return 0; }
	virtual i32                         GetCustomTypeCount() const                                                                             { return 0; }
};

DECLARE_SHARED_POINTERS(IAnimEventPlayer);

inline bool Serialize(Serialization::IArchive& ar, IAnimEventPlayerPtr& pointer, tukk name, tukk label)
{
	Serialization::DrxExtensionPointer<IAnimEventPlayer, IAnimEventPlayer> serializer(pointer);
	return ar(serializer, name, label);
}
//! \endcond

// Game-specific anim event player, will be automatically found by the character tool
struct IAnimEventPlayerGame : public IAnimEventPlayer
{
	DRXINTERFACE_DECLARE_GUID(IAnimEventPlayerGame, "3218ad9c-8237-4c5f-8b64-87bdedaf1c4a"_drx_guid);
};