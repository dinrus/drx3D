// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Drx_Math.h>

#include <drx3D/CoreX/Serialization/Forward.h>

struct SkeletonAlias;

bool Serialize(Serialization::IArchive& ar, SkeletonAlias& value, tukk name, tukk label);

#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/CoreX/Serialization/Decorators/Range.h>
#include <drx3D/CoreX/Serialization/Decorators/OutputFilePath.h>
using Serialization::OutputFilePath;
#include <drx3D/CoreX/Serialization/Decorators/ResourceFilePath.h>
using Serialization::ResourceFilePath;
#include <drx3D/CoreX/Serialization/Decorators/JointName.h>
using Serialization::JointName;

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/SmartPtr.h>
#include <drx3D/CoreX/Serialization/ClassFactory.h>
#include <drx3D/CoreX/Serialization/Enum.h>
using Serialization::IArchive;

