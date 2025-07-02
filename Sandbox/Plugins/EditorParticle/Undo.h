// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/NodeGraphUndo.h>

#include <QPointF>
#include <QObject>

namespace pfx2 {

struct SParticleFeatureParams;

}

namespace DrxParticleEditor {

class CFeatureItem;

class CUndoFeatureCreate : public DrxGraphEditor::CUndoObject
{
public:
	CUndoFeatureCreate(CFeatureItem& feature, const string& description = string());

protected:
	// IUndoObject
	virtual void Undo(bool bUndo) override;
	virtual void Redo() override;
	// ~UndoObject

private:
	QVariant                            m_nodeId;
	const pfx2::SParticleFeatureParams& m_params;
	DynArray<char>                      m_featureData;
	u32                              m_featureIndex;
};

class CUndoFeatureRemove : public DrxGraphEditor::CUndoObject
{
public:
	CUndoFeatureRemove(CFeatureItem& feature, const string& description = string());

protected:
	// IUndoObject
	virtual void Undo(bool bUndo) override;
	virtual void Redo() override;
	// ~UndoObject

private:
	QVariant                            m_nodeId;
	const pfx2::SParticleFeatureParams& m_params;
	DynArray<char>                      m_featureData;
	u32                              m_featureIndex;
};

class CUndoFeatureMove : public DrxGraphEditor::CUndoObject
{
public:
	CUndoFeatureMove(CFeatureItem& feature, u32 oldIndex, const string& description = string());

protected:
	// IUndoObject
	virtual void Undo(bool bUndo) override;
	virtual void Redo() override;
	// ~UndoObject

private:
	QVariant m_nodeId;
	u32   m_oldFeatureIndex;
	u32   m_newFeatureIndex;
};

}

