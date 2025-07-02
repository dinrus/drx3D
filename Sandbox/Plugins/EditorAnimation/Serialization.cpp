// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "stdafx.h"

#include "Serialization.h"
#include <drx3D/CoreX/Serialization/IArchiveHost.h>

namespace CharacterTool
{

void SerializeToMemory(std::vector<char>* buffer, const Serialization::SStruct& obj)
{
	DynArray<char> temp;
	gEnv->pSystem->GetArchiveHost()->SaveBinaryBuffer(temp, obj);
	buffer->assign(temp.begin(), temp.end());
}

void SerializeToMemory(DynArray<char>* buffer, const Serialization::SStruct& obj)
{
	gEnv->pSystem->GetArchiveHost()->SaveBinaryBuffer(*buffer, obj);
}

void SerializeFromMemory(const Serialization::SStruct& obj, const std::vector<char>& buffer)
{
	gEnv->pSystem->GetArchiveHost()->LoadBinaryBuffer(obj, buffer.data(), buffer.size());
}

void SerializeFromMemory(const Serialization::SStruct& obj, const DynArray<char>& buffer)
{
	gEnv->pSystem->GetArchiveHost()->LoadBinaryBuffer(obj, buffer.data(), buffer.size());
}

}

