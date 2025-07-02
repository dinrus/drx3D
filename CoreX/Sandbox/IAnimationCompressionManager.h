// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct IAnimationCompressionManager
{
	virtual bool IsEnabled() const = 0;
	virtual void UpdateLocalAnimations() = 0;

	// animationName is defined as a path withing game data folder, e.g.
	// "animations/alien/cerberus/cerberus_activation.caf"
	virtual void QueueAnimationCompression(tukk animationName) = 0;
};

