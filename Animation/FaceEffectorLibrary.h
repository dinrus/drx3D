// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/FaceEffector.h>

class CFacialAnimation;

//////////////////////////////////////////////////////////////////////////
class CFacialEffectorsLibrary : public IFacialEffectorsLibrary
{
public:
	CFacialEffectorsLibrary(CFacialAnimation* pFaceAnim);
	~CFacialEffectorsLibrary();

	//////////////////////////////////////////////////////////////////////////
	// IFacialEffectorsLibrary interface.
	//////////////////////////////////////////////////////////////////////////
	virtual void             AddRef()                  { ++m_nRefCounter; }
	virtual void             Release()                 { if (--m_nRefCounter <= 0) delete this; }

	virtual void             SetName(tukk name) { m_name = name; };
	virtual tukk      GetName()                 { return m_name.c_str(); };

	virtual IFacialEffector* Find(CFaceIdentifierHandle ident);
	virtual IFacialEffector* Find(tukk identStr);
	virtual IFacialEffector* GetRoot();

	virtual void             VisitEffectors(IFacialEffectorsLibraryEffectorVisitor* pVisitor);

	IFacialEffector*         CreateEffector(EFacialEffectorType nType, CFaceIdentifierHandle ident);
	IFacialEffector*         CreateEffector(EFacialEffectorType nType, tukk identStr);
	virtual void             RemoveEffector(IFacialEffector* pEffector);

	virtual void             Serialize(XmlNodeRef& node, bool bLoading);
	virtual void             SerializeEffector(IFacialEffector* pEffector, XmlNodeRef& node, bool bLoading);
	//////////////////////////////////////////////////////////////////////////

	i32              GetLastIndex() const { return m_nLastIndex; }
	void             RenameEffector(CFacialEffector* pEffector, CFaceIdentifierHandle ident);

	CFacialEffector* GetEffectorFromIndex(i32 nIndex);

	void             MergeLibrary(IFacialEffectorsLibrary* pMergeLibrary, const Functor1wRet<tukk , MergeCollisionAction>& collisionStrategy);

	i32              GetEffectorCount() const { return m_crcToEffectorMap.size(); }

	void             GetMemoryUsage(IDrxSizer* pSizer) const;
private:
	void             NewRoot();
	void             SerializeEffectorSelf(CFacialEffector* pEffector, XmlNodeRef& node, bool bLoading);
	void             SerializeEffectorSubCtrls(CFacialEffector* pEffector, XmlNodeRef& node, bool bLoading);
	void             SetEffectorIndex(CFacialEffector* pEffector);
	void             ReplaceEffector(CFacialEffector* pOriginalEffector, CFacialEffector* pNewEffector);

private:
	friend class CFacialModel;

	i32               m_nRefCounter;
	string            m_name;

	CFacialAnimation* m_pFacialAnim;

	typedef std::map<u32, _smart_ptr<CFacialEffector>> CrcToEffectorMap;
	CrcToEffectorMap                         m_crcToEffectorMap;

	_smart_ptr<CFacialEffector>              m_pRoot;

	std::vector<_smart_ptr<CFacialEffector>> m_indexToEffector;

	i32 m_nLastIndex;
};
