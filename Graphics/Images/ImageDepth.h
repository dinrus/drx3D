#pragma once

#include <drx3D/Graphics/Images/Image.h>

namespace drx3d {
/**
 * @brief Resource that represents a depth stencil image.
 */
class DRX3D_EXPORT ImageDepth : public Image {
public:
	explicit ImageDepth(const Vector2ui &extent, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
};
}
