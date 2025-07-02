// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace CharacterTool
{

struct SAnimationReference
{
	SAnimationReference()
		: pathCRC(0)
		, animationName()
		, bIsRegularCaf(false)
	{}

	SAnimationReference(u32 pathCRC, tukk animationName, bool bIsCaf)
		: pathCRC(0)
	{
		reset(pathCRC, animationName, bIsCaf);
	}

	SAnimationReference(const SAnimationReference& rhs)
		: pathCRC(0)
	{
		reset(rhs.pathCRC, rhs.animationName.c_str(), rhs.bIsRegularCaf);
	}

	SAnimationReference& operator=(const SAnimationReference& rhs)
	{
		reset(rhs.pathCRC, rhs.animationName.c_str(), rhs.bIsRegularCaf);
		return *this;
	}

	~SAnimationReference()
	{
		reset(0, nullptr, false);
	}

	void         reset(u32 crc, tukk animationName, bool bIsRegularCaf);

	u32 PathCRC() const       { return pathCRC; }

	tukk  AnimationName() const { return animationName.c_str(); }

private:

	u32 pathCRC;
	string       animationName;
	bool         bIsRegularCaf;
};

}

