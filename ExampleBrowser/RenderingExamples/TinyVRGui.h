
#ifndef TINY_VR_GUI_H
#define TINY_VR_GUI_H

#include <drx3D/Common/b3Transform.h>

class TinyVRGui
{
	struct TinyVRGuiInternalData* m_data;

public:
	TinyVRGui(struct ComboBoxParams& params, struct CommonRenderInterface* renderer);
	virtual ~TinyVRGui();

	bool init();
	void tick(b3Scalar deltaTime, const b3Transform& guiWorldTransform);

	void clearTextArea();
	void grapicalPrintf(tukk str, i32 rasterposx, i32 rasterposy, u8 red, u8 green, u8 blue, u8 alpha);
};

#endif  //TINY_VR_GUI_H
