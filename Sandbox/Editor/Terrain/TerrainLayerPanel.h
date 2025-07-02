// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <QWidget>
#include "EditToolPanel.h"

class CLayer;

struct STexturePainterSerializer
{
public:
	STexturePainterSerializer()
		: pEditTool(nullptr)
		, pLayer(nullptr)
		, textureLayerUpdate(false)
	{}

	void YASLI_SERIALIZE_METHOD(Serialization::IArchive& ar);

	CEditTool* pEditTool;
	CLayer*    pLayer;
	bool       textureLayerUpdate;
};

class QTerrainLayerButtons : public QWidget
{
public:
	QTerrainLayerButtons(QWidget* parent = nullptr);

private:
	void AddTool(CRuntimeClass* pRuntimeClass, tukk text);
};

class QTerrainLayerPanel : public QEditToolPanel
{
public:
	QTerrainLayerPanel(QWidget* parent = nullptr);
	virtual ~QTerrainLayerPanel() override;

	void SetLayer(CLayer* pLayer);
	void UndoPush();

protected:
	virtual bool CanEditTool(CEditTool* pTool) override;
	virtual void SetTool(CEditTool* pTool) override;
	void         LayerChanged(CLayer* pLayer);
	void         BrushChanged(CEditTool* pTool);
	void         AttachProperties();

	STexturePainterSerializer m_layerSerializer;
};

