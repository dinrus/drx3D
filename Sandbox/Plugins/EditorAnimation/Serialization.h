// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "../EditorCommon/Serialization.h"

#include <drx3D/CoreX/Serialization/Forward.h>

struct SkeletonAlias;
bool Serialize(Serialization::IArchive& ar, SkeletonAlias& value, tukk name, tukk label);

#include <drx3D/CoreX/Serialization/Decorators/Resources.h>
#include <drx3D/CoreX/Serialization/Decorators/ResourceFilePath.h>
#include <drx3D/CoreX/Serialization/Decorators/ResourceFolderPath.h>
#include <drx3D/CoreX/Serialization/Decorators/JointName.h>
#include <drx3D/CoreX/Serialization/Decorators/TagList.h>
#include <Serialization/Decorators/INavigationProvider.h>
#include <drx3D/CoreX/Serialization/Decorators/LocalFrame.h>

#include "Serialization/Qt.h"

using Serialization::AttachmentName;
using Serialization::ResourceFilePath;
using Serialization::MaterialPicker;
using Serialization::SkeletonOrCgaPath;
using Serialization::SkeletonParamsPath;
using Serialization::AnimationAlias;
using Serialization::AnimationPath;
using Serialization::AnimationOrBlendSpacePath;
using Serialization::AnimationPathWithId;
using Serialization::CharacterPath;
using Serialization::CharacterPhysicsPath;
using Serialization::CharacterRigPath;
using Serialization::ResourceFolderPath;
using Serialization::JointName;
using Serialization::BitFlags;
using Serialization::LocalToJoint;
using Serialization::ToggleButton;
using Serialization::SerializeToMemory;
using Serialization::SerializeToMemory;
using Serialization::SerializeFromMemory;
using Serialization::SerializeFromMemory;

