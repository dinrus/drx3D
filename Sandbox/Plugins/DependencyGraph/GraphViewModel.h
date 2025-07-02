// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <NodeGraph/AbstractNodeGraphViewModel.h>
#include <NodeGraph/AbstractNodeItem.h>

class CModel;
class CAsset;
class CAssetType;
class CNodeGraphViewModel;
class CAbstractDictionaryEntry;

class CGraphViewModel : public DrxGraphEditor::CNodeGraphViewModel
{
	Q_OBJECT
public:
	CGraphViewModel(CModel* pModel);

	virtual DrxGraphEditor::INodeGraphRuntimeContext& GetRuntimeContext() override;
	virtual QString                                   GetGraphName() override;
	virtual u32                                    GetNodeItemCount() const override;
	virtual DrxGraphEditor::CAbstractNodeItem*        GetNodeItemByIndex(u32 index) const override;
	virtual DrxGraphEditor::CAbstractNodeItem*        GetNodeItemById(QVariant id) const override;

	virtual u32                                    GetConnectionItemCount() const override;
	virtual DrxGraphEditor::CAbstractConnectionItem*  GetConnectionItemByIndex(u32 index) const override;
	virtual DrxGraphEditor::CAbstractConnectionItem*  GetConnectionItemById(QVariant id) const override;

	const CModel*                                     GetModel() { return m_pModel; }

private:
	void OnBeginModelChange();
	void OnEndModelChange();
private:
	std::unique_ptr<DrxGraphEditor::INodeGraphRuntimeContext> m_pRuntimeContext;
	CModel* m_pModel;

	std::vector<std::unique_ptr<DrxGraphEditor::CAbstractNodeItem>>       m_nodes;
	std::vector<std::unique_ptr<DrxGraphEditor::CAbstractConnectionItem>> m_connections;
	bool m_bSuppressComplexityWarning;
};

//! Asset view model.
class CAssetNodeBase : public DrxGraphEditor::CAbstractNodeItem
{
public:
	CAssetNodeBase(DrxGraphEditor::CNodeGraphViewModel& viewModel, CAsset* pAsset, const CAssetType* pAssetType, const string& m_path);

	bool                                    CanBeEdited() const;
	void                                    EditAsset() const;

	bool                                    CanBeCreated() const;
	void                                    CreateAsset() const;

	bool                                    CanBeImported() const;
	std::vector<string>                     GetSourceFileExtensions() const;
	void                                    ImportAsset(const string& sourceFilename) const;

	string                                  GetPath() const { return m_path; }
	CAsset*									GetAsset() const { return m_pAsset; }

	virtual const CAbstractDictionaryEntry* GetNodeEntry() const = 0;

protected:
	CAsset* const           m_pAsset;
	const CAssetType* const m_pAssetType;
	string                  m_path;
};

