// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "IDrxGraphEditor.h"
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>

#include "NodeGraphViewGraphicsWidget.h"

#include <drx3D/CoreX/Sandbox/DrxSignal.h>
#include "IUndoObject.h"

#include <QPointF>
#include <QVariant>

namespace DrxGraphEditor {

class CNodeGraphViewModel;
class CAbstractNodeItem;
class CAbstractConnectionItem;
class CAbstractPinItem;
class CAbstractConnectionItem;

class EDITOR_COMMON_API CUndoObject : public IUndoObject
{
public:
	CUndoObject(CNodeGraphViewModel* pModel);
	~CUndoObject();

	// IUndoObject
	virtual tukk GetDescription() final { return m_description.c_str(); }
	// ~IUndoObject

	const string& GetDescriptionString() const              { return m_description; }
	void          SetDescription(const string& description) { m_description = description; }

private:
	void OnModelDestruction() { m_pModel = nullptr; }

protected:
	CNodeGraphViewModel* m_pModel;
	string               m_description;
};

class EDITOR_COMMON_API CUndoNodeMove : public CUndoObject
{
public:
	CUndoNodeMove(CAbstractNodeItem& nodeItem, const string& description = string());

protected:
	// IUndoObject
	virtual void Undo(bool bUndo) override;
	virtual void Redo() override;
	// ~UndoObject

private:
	QVariant m_nodeId;
	QPointF  m_position;
};

class EDITOR_COMMON_API CUndoNodePropertiesChange : public CUndoObject
{
public:
	CUndoNodePropertiesChange(CAbstractNodeItem& nodeItem, const string& description = string());

protected:
	// IUndoObject
	virtual void Undo(bool bUndo) override;
	virtual void Redo() override;
	// ~UndoObject

private:
	QVariant       m_nodeId;
	DynArray<char> m_nodeData;
};

class EDITOR_COMMON_API CUndoNodeNameChange : public CUndoObject
{
public:
	CUndoNodeNameChange(CAbstractNodeItem& nodeItem, tukk szNewName, const string& description = string());

protected:
	// IUndoObject
	virtual void Undo(bool bUndo) override;
	virtual void Redo() override;
	// ~UndoObject

private:
	QVariant m_nodeId;
	QString  m_name;
};

class EDITOR_COMMON_API CUndoConnectionCreate : public CUndoObject
{
public:
	CUndoConnectionCreate(CAbstractConnectionItem& connection, const string& description = string());

protected:
	// IUndoObject
	virtual void Undo(bool bUndo) override;
	virtual void Redo() override;
	// ~UndoObject

private:
	QVariant       m_sourcePinId;
	QVariant       m_targetPinId;
	QVariant       m_sourceNodeId;
	QVariant       m_targetNodeId;
	DynArray<char> m_connectionData;
	QVariant       m_connectionId;
};

class EDITOR_COMMON_API CUndoConnectionRemove : public CUndoObject
{
public:
	CUndoConnectionRemove(CAbstractConnectionItem& connection, const string& description = string());

protected:
	// IUndoObject
	virtual void Undo(bool bUndo) override;
	virtual void Redo() override;
	// ~UndoObject

private:
	QVariant       m_sourcePinId;
	QVariant       m_targetPinId;
	QVariant       m_sourceNodeId;
	QVariant       m_targetNodeId;
	DynArray<char> m_connectionData;
	QVariant       m_connectionId;
};

class EDITOR_COMMON_API CUndoNodeCreate : public CUndoObject
{
public:
	CUndoNodeCreate(CAbstractNodeItem& nodeItem, const string& description = string());

protected:
	// IUndoObject
	virtual void Undo(bool bUndo) override;
	virtual void Redo() override;
	// ~UndoObject

private:
	QVariant       m_nodeId;
	QVariant       m_typdeId;
	DynArray<char> m_nodeData;
	QPointF        m_position;
};

class EDITOR_COMMON_API CUndoNodeRemove : public CUndoObject
{
public:
	CUndoNodeRemove(CAbstractNodeItem& nodeItem, const string& description = string());

protected:
	// IUndoObject
	virtual void Undo(bool bUndo) override;
	virtual void Redo() override;
	// ~UndoObject

private:
	QVariant       m_nodeId;
	QVariant       m_typdeId;
	DynArray<char> m_nodeData;
	QPointF        m_position;
};

}

