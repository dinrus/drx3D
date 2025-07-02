// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   FlashUIElement.h
//  Version:     v1.00
//  Created:     10/9/2010 by Paul Reindell.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#ifndef __FlashUIElement_H__
#define __FlashUIElement_H__

#include <drx3D/Sys/IFlashUI.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>
#include <drx3D/CoreX/Platform/IPlatformOS.h>

struct SUIElementSerializer;
class CFlashUI;

class CFlashUIElement
	: public IUIElement
	  , public IFSCommandHandler
	  , public IVirtualKeyboardEvents
{
public:
	CFlashUIElement(CFlashUI* pFlashUI, CFlashUIElement* pBaseInstance = NULL, uint instanceId = 0);
	virtual ~CFlashUIElement();

	virtual void                                     AddRef();
	virtual void                                     Release();

	virtual uint                                     GetInstanceID() const { return m_iInstanceID; }
	virtual IUIElement*                              GetInstance(uint instanceID);
	virtual IUIElementIteratorPtr                    GetInstances() const;
	virtual bool                                     DestroyInstance(uint instanceID);
	virtual bool                                     DestroyThis()                        { return DestroyInstance(m_iInstanceID); }

	virtual void                                     SetName(tukk sName)           { m_sName = sName; }
	virtual tukk                              GetName() const                      { return m_sName.c_str(); }

	virtual void                                     SetGroupName(tukk sGroupName) { m_sGroupName = sGroupName; }
	virtual tukk                              GetGroupName() const                 { return m_sGroupName.c_str(); }

	virtual void                                     SetFlashFile(tukk sFlashFile);
	virtual tukk                              GetFlashFile() const { return m_sFlashFile.c_str(); }

	virtual bool                                     Init(bool bLoadAsset = true);
	virtual void                                     Unload(bool bAllInstances = false);
	virtual void                                     Reload(bool bAllInstances = false);
	virtual bool                                     IsInit() const { return m_pFlashplayer != NULL; }

	virtual void                                     RequestUnload(bool bAllInstances = false);

	virtual bool                                     IsValid() const { return m_pBaseInstance ? m_pBaseInstance->m_bIsValid : m_bIsValid; }

	virtual void                                     UnloadBootStrapper();
	virtual void                                     ReloadBootStrapper();

	virtual void                                     Update(float fDeltaTime);
	virtual void                                     Render();
	virtual void                                     RenderLockless();

	virtual void                                     RequestHide();
	virtual bool                                     IsHiding() const { return m_bIsHideRequest; }

	virtual void                                     SetVisible(bool bVisible);
	virtual inline bool                              IsVisible() const { return m_bVisible; }

	virtual void                                     SetFlag(EFlashUIFlags flag, bool bSet);
	virtual bool                                     HasFlag(EFlashUIFlags flag) const;

	virtual float                                    GetAlpha() const { return m_fAlpha; }
	virtual void                                     SetAlpha(float fAlpha);

	virtual i32                                      GetLayer() const { return m_iLayer; }
	virtual void                                     SetLayer(i32 iLayer);

	virtual void                                     SetConstraints(const SUIConstraints& newConstraints);
	virtual inline const IUIElement::SUIConstraints& GetConstraints() const { return m_constraints; }

	virtual void                                     ForceLazyUpdate()      { ForceLazyUpdateInl(); }
	virtual void                                     LazyRendered()         { m_bNeedLazyRender = false; }
	virtual bool                                     NeedLazyRender() const { return (m_iFlags & (uint64) eFUI_LAZY_UPDATE) == 0 || m_bNeedLazyRender; }

	virtual std::shared_ptr<IFlashPlayer>            GetFlashPlayer();

	virtual const SUIParameterDesc*                  GetVariableDesc(i32 index) const                          { return index < m_variables.size() ? m_variables[index] : NULL; }
	virtual const SUIParameterDesc*                  GetVariableDesc(tukk sVarName) const               { return m_variables(sVarName); }
	virtual i32                                      GetVariableCount() const                                  { return m_variables.size(); }

	virtual const SUIParameterDesc*                  GetArrayDesc(i32 index) const                             { return index < m_arrays.size() ? m_arrays[index] : NULL; }
	virtual const SUIParameterDesc*                  GetArrayDesc(tukk sArrayName) const                { return m_arrays(sArrayName); }
	virtual i32                                      GetArrayCount() const                                     { return m_arrays.size(); }

	virtual const SUIMovieClipDesc*                  GetMovieClipDesc(i32 index) const                         { return index < m_displayObjects.size() ? m_displayObjects[index] : NULL; }
	virtual const SUIMovieClipDesc*                  GetMovieClipDesc(tukk sMovieClipName) const        { return m_displayObjects(sMovieClipName); }
	virtual i32                                      GetMovieClipCount() const                                 { return m_displayObjects.size(); }

	virtual const SUIMovieClipDesc*                  GetMovieClipTmplDesc(i32 index) const                     { return index < m_displayObjectsTmpl.size() ? m_displayObjectsTmpl[index] : NULL; }
	virtual const SUIMovieClipDesc*                  GetMovieClipTmplDesc(tukk movieClipTmplName) const { return m_displayObjectsTmpl(movieClipTmplName); }
	virtual i32                                      GetMovieClipTmplCount() const                             { return m_displayObjectsTmpl.size(); }

	virtual const SUIEventDesc*                      GetEventDesc(i32 index) const                             { return index < m_events.size() ? m_events[index] : NULL; }
	virtual const SUIEventDesc*                      GetEventDesc(tukk sEventName) const                { return m_events(sEventName); }
	virtual i32                                      GetEventCount() const                                     { return m_events.size(); }

	virtual const SUIEventDesc*                      GetFunctionDesc(i32 index) const                          { return index < m_functions.size() ? m_functions[index] : NULL; }
	virtual const SUIEventDesc*                      GetFunctionDesc(tukk sFunctionName) const          { return m_functions(sFunctionName); }
	virtual i32                                      GetFunctionCount() const                                  { return m_functions.size(); }

	virtual void                                     UpdateViewPort();
	virtual void                                     GetViewPort(i32& x, i32& y, i32& width, i32& height, float& aspectRatio);

	virtual bool                                     Serialize(XmlNodeRef& xmlNode, bool bIsLoading);

	virtual void                                     AddEventListener(IUIElementEventListener* pListener, tukk name);
	virtual void                                     RemoveEventListener(IUIElementEventListener* pListener);

	virtual bool                                     CallFunction(tukk fctName, const SUIArguments& args = SUIArguments(), TUIData* pDataRes = NULL, tukk pTmplName = NULL);
	virtual bool                                     CallFunction(const SUIEventDesc* pFctDesc, const SUIArguments& args = SUIArguments(), TUIData* pDataRes = NULL, const SUIMovieClipDesc* pTmplDesc = NULL);

	virtual IFlashVariableObject*                    GetMovieClip(tukk movieClipName, tukk pTmplName = NULL);
	virtual IFlashVariableObject*                    GetMovieClip(const SUIMovieClipDesc* pMovieClipDesc, const SUIMovieClipDesc* pTmplDesc = NULL);
	virtual IFlashVariableObject*                    CreateMovieClip(const SUIMovieClipDesc*& pNewInstanceDesc, tukk movieClipTemplate, tukk mcParentName = NULL, tukk mcInstanceName = NULL);
	virtual IFlashVariableObject*                    CreateMovieClip(const SUIMovieClipDesc*& pNewInstanceDesc, const SUIMovieClipDesc* pMovieClipTemplateDesc, const SUIMovieClipDesc* pParentMC = NULL, tukk mcInstanceName = NULL);
	virtual void                                     RemoveMovieClip(tukk movieClipName);
	virtual void                                     RemoveMovieClip(const SUIParameterDesc* pMovieClipDesc);
	virtual void                                     RemoveMovieClip(IFlashVariableObject* pVarObject);

	virtual bool                                     SetVariable(tukk varName, const TUIData& value, tukk pTmplName = NULL);
	virtual bool                                     SetVariable(const SUIParameterDesc* pVarDesc, const TUIData& value, const SUIMovieClipDesc* pTmplDesc = NULL);
	virtual bool                                     GetVariable(tukk varName, TUIData& valueOut, tukk pTmplName = NULL);
	virtual bool                                     GetVariable(const SUIParameterDesc* pVarDesc, TUIData& valueOut, const SUIMovieClipDesc* pTmplDesc = NULL);
	virtual bool                                     CreateVariable(const SUIParameterDesc*& pNewDesc, tukk varName, const TUIData& value, tukk pTmplName = NULL);
	virtual bool                                     CreateVariable(const SUIParameterDesc*& pNewDesc, tukk varName, const TUIData& value, const SUIMovieClipDesc* pTmplDesc = NULL);

	virtual bool                                     SetArray(tukk arrayName, const SUIArguments& values, tukk pTmplName = NULL);
	virtual bool                                     SetArray(const SUIParameterDesc* pArrayDesc, const SUIArguments& values, const SUIMovieClipDesc* pTmplDesc = NULL);
	virtual bool                                     GetArray(tukk arrayName, SUIArguments& valuesOut, tukk pTmplName = NULL);
	virtual bool                                     GetArray(const SUIParameterDesc* pArrayDesc, SUIArguments& valuesOut, const SUIMovieClipDesc* pTmplDesc = NULL);
	virtual bool                                     CreateArray(const SUIParameterDesc*& pNewDesc, tukk arrayName, const SUIArguments& values, tukk pTmplName = NULL);
	virtual bool                                     CreateArray(const SUIParameterDesc*& pNewDesc, tukk arrayName, const SUIArguments& values, const SUIMovieClipDesc* pTmplDesc = NULL);

	virtual void                                     ScreenToFlash(const float& px, const float& py, float& rx, float& ry, bool bStageScaleMode = false) const;
	virtual void                                     WorldToFlash(const Matrix34& camMat, const Vec3& worldpos, Vec3& flashpos, Vec2& borders, float& scale, bool bStageScaleMode = false) const;

	virtual void                                     LoadTexIntoMc(tukk movieClip, ITexture* pTexture, tukk pTmplName = NULL);
	virtual void                                     LoadTexIntoMc(const SUIParameterDesc* pMovieClipDesc, ITexture* pTexture, const SUIMovieClipDesc* pTmplDesc = NULL);

	virtual void                                     UnloadTexFromMc(tukk movieClip, ITexture* pTexture, tukk pTmplName = NULL);
	virtual void                                     UnloadTexFromMc(const SUIParameterDesc* pMovieClipDesc, ITexture* pTexture, const SUIMovieClipDesc* pTmplDesc = NULL);

	virtual void                                     AddTexture(IDynTextureSource* pDynTexture);
	virtual void                                     RemoveTexture(IDynTextureSource* pDynTexture);
	virtual i32                                      GetNumExtTextures() const                    { return m_textures.GetCount(); }
	virtual bool                                     GetDynTexSize(i32& width, i32& height) const { width = m_constraints.iWidth; height = m_constraints.iHeight; return m_constraints.eType == SUIConstraints::ePT_FixedDynTexSize; }

	virtual void                                     SendCursorEvent(SFlashCursorEvent::ECursorState evt, i32 iX, i32 iY, i32 iButton = 0, float fWheel = 0.f);
	virtual void                                     SendKeyEvent(const SFlashKeyEvent& evt);
	virtual void                                     SendCharEvent(const SFlashCharEvent& charEvent);
	virtual void                                     SendControllerEvent(EControllerInputEvent event, EControllerInputState state, float value);
	virtual void                                     GetMemoryUsage(IDrxSizer* s) const;

	// IFSCommandHandler
	void HandleFSCommand(tukk pCommand, tukk pArgs, uk pUserData = 0);
	// ~IFSCommandHandler

	// IVirtualKeyboardEvents
	virtual void KeyboardCancelled();
	virtual void KeyboardFinished(tukk pInString);
	// ~IVirtualKeyboardEvents

	void SetValid(bool bValid) { if (m_pBaseInstance) { m_pBaseInstance->SetValid(bValid); return; } if (!bValid) DestroyBootStrapper(); m_bIsValid = bValid; }

private:
	IFlashPlayerBootStrapper* InitBootStrapper();
	void                      DestroyBootStrapper();

	const SUIParameterDesc*   GetOrCreateVariableDesc(tukk pVarName, bool* bExist = NULL);
	const SUIParameterDesc*   GetOrCreateArrayDesc(tukk pArrayName, bool* bExist = NULL);
	const SUIMovieClipDesc*   GetOrCreateMovieClipDesc(tukk pMovieClipName, bool* bExist = NULL);
	const SUIMovieClipDesc*   GetOrCreateMovieClipTmplDesc(tukk pMovieClipTmplName);
	const SUIEventDesc*       GetOrCreateFunctionDesc(tukk pFunctionName);

	bool                      SetVariableInt(const SUIParameterDesc* pVarDesc, const TUIData& value, const SUIMovieClipDesc* pTmplDesc, bool bCreate = false);

	inline void               SetFlagInt(EFlashUIFlags flag, bool bSet);

	inline bool               HasExtTexture() const { return !m_textures.IsEmpty(); }
	void                      UpdateFlags();
	bool                      HandleInternalCommand(tukk pCommand, const SUIArguments& args);

	struct SFlashObjectInfo
	{
		IFlashVariableObject* pObj;
		IFlashVariableObject* pParent;
		string                sMember;
	};

	bool              DefaultInfoCheck(SFlashObjectInfo*& pInfo, const SUIParameterDesc* pDesc, const SUIMovieClipDesc* pTmplDesc);

	SFlashObjectInfo* GetFlashVarObj(const SUIParameterDesc* pDesc, const SUIMovieClipDesc* pTmplDesc = NULL);
	void              RemoveFlashVarObj(const SUIParameterDesc* pDesc);
	void              FreeVarObjects();

	typedef std::vector<IUIElement*>           TUIElements;
	typedef std::set<IUIElementEventListener*> TUIEventListenerUnique;
	inline TUIElements::iterator GetAllListeners(TUIEventListenerUnique& listeners, uint instanceID = 0);

	inline bool                  LazyInit();
	inline void                  ShowCursor();
	inline void                  HideCursor();

	inline tukk           GetStringBuffer(tukk str);
	inline Vec3                  MatMulVec3(const Matrix44& m, const Vec3& v) const;

	inline void                  ForceLazyUpdateInl() { m_bNeedLazyUpdate = true; m_bNeedLazyRender = false; }

	const SUIParameterDesc*      GetDescForInfo(SFlashObjectInfo* pInfo, const SUIParameterDesc** pParent = NULL) const;
private:
	 i32              m_refCount;
	CFlashUI*                 m_pFlashUI;
	string                    m_sName;
	string                    m_sGroupName;
	string                    m_sFlashFile;
	float                     m_fAlpha;
	i32                       m_iLayer;
	bool                      m_bIsValid;
	bool                      m_bIsHideRequest;
	bool                      m_bUnloadRequest;
	bool                      m_bUnloadAll;

	std::shared_ptr<IFlashPlayer> m_pFlashplayer;

	IFlashPlayerBootStrapper* m_pBootStrapper;
	const SUIMovieClipDesc*   m_pRoot;

	bool                      m_bVisible;
	uint64                    m_iFlags;
	bool                      m_bCursorVisible;
	bool                      m_bNeedLazyUpdate;
	bool                      m_bNeedLazyRender;
	SUIConstraints            m_constraints;
	XmlNodeRef                m_baseInfo;
	typedef std::set<string> TStringBuffer;
	TStringBuffer             m_StringBufferSet;

	CFlashUIElement*          m_pBaseInstance;
	uint                      m_iInstanceID;
	TUIElements               m_instances;

	TUIParamsLookup           m_variables;
	TUIParamsLookup           m_arrays;
	TUIMovieClipLookup        m_displayObjects;
	TUIMovieClipLookup        m_displayObjectsTmpl;
	TUIEventsLookup           m_events;
	TUIEventsLookup           m_functions;
	i32                       m_firstDynamicDisplObjIndex;

	typedef CListenerSet<IUIElementEventListener*> TUIEventListener;
	TUIEventListener m_eventListener;

	typedef std::map<CDrxName, SFlashObjectInfo>                 TVarMap;
	typedef std::map<const SUIParameterDesc*, SFlashObjectInfo*> TVarMapLookup;
	typedef std::map<const SUIParameterDesc*, TVarMapLookup>     TTmplMapLookup;
	TVarMap        m_variableObjects;
	TTmplMapLookup m_variableObjectsLookup;

	template<class T>
	struct SThreadSafeVec
	{
		typedef std::vector<T> TVec;

		TVec GetCopy() const
		{
			DrxAutoCriticalSection lock(m_lock);
			return m_vec;
		}

		bool PushBackUnique(const T& v)
		{
			DrxAutoCriticalSection lock(m_lock);
			return stl::push_back_unique(m_vec, v);
		}

		bool FindAndErase(const T& v)
		{
			DrxAutoCriticalSection lock(m_lock);
			return stl::find_and_erase(m_vec, v);
		}

		size_t GetCount() const
		{
			DrxAutoCriticalSection lock(m_lock);
			return m_vec.size();
		}

		bool IsEmpty() const
		{
			DrxAutoCriticalSection lock(m_lock);
			return m_vec.empty();
		}

	private:
		TVec                       m_vec;
		mutable DrxCriticalSection m_lock;
	};

	typedef SThreadSafeVec<IDynTextureSource*> TDynTextures;
	TDynTextures m_textures;

	friend struct SUIElementSerializer;
	friend struct CUIElementIterator;

#if !defined (_RELEASE)
	friend void RenderDebugInfo();
	typedef std::vector<CFlashUIElement*> TElementInstanceList;
	static TElementInstanceList s_ElementDebugList;
#endif
};

struct CUIElementIterator : public IUIElementIterator
{
	CUIElementIterator(const CFlashUIElement* pElement);
	virtual void        AddRef();
	virtual void        Release();
	virtual IUIElement* Next();
	virtual i32         GetCount() const { return m_pElement->m_instances.size(); }

private:
	i32                                          m_iRefs;
	const CFlashUIElement*                       m_pElement;
	CFlashUIElement::TUIElements::const_iterator m_currIter;
};

#endif // #ifndef __FlashUIElement_H__
