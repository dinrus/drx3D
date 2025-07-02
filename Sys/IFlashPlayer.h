// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Drx_Color.h>
#include <drx3D/CoreX/Math/Drx_Math.h>

#include <memory>

struct IFlashVariableObject;
struct IFlashPlayerBootStrapper;
struct IFSCommandHandler;
struct IExternalInterfaceHandler;
struct IActionScriptFunction;
struct IScaleformPlayback;

struct SFlashVarValue;
struct SFlashCxform;
struct SFlashDisplayInfo;
struct SFlashCursorEvent;
struct SFlashKeyEvent;
struct SFlashCharEvent;

//! Currently arrays of SFlashVarValue are not supported as it would require runtime conversion from / to Scaleform internal variant type.
//! GFxValue unless we enforce binary compatibility!.
enum EFlashVariableArrayType
{
	FVAT_Int,
	FVAT_Double,
	FVAT_Float,
	FVAT_ConstStrPtr,
	FVAT_ConstWstrPtr
};

struct IFlashPlayer
{
	enum EOptions
	{
		//LOG_FLASH_LOADING   = 0x01, // Logs loading of flash file.
		//LOG_ACTION_SCRIPT   = 0x02, // Logs action script.
		RENDER_EDGE_AA       = 0x04,  //!< Enables edge anti-aliased flash rendering.
		INIT_FIRST_FRAME     = 0x08,  //!< Init objects of first frame when creating instance of flash file.
		ENABLE_MOUSE_SUPPORT = 0x10,  //!< Enable mouse input support.

		DEFAULT              = RENDER_EDGE_AA | INIT_FIRST_FRAME | ENABLE_MOUSE_SUPPORT,
		DEFAULT_NO_MOUSE     = RENDER_EDGE_AA | INIT_FIRST_FRAME
	};

	enum ECategory
	{
		eCat_RequestMeshCacheResetBit = 0x80000000,

		eCat_Default                  = 0,
		eCat_Temp                     = 1,
		eCat_Temp_TessHeavy           = 1 | eCat_RequestMeshCacheResetBit,

		eCat_User                     = 3
	};

	enum EScaleModeType
	{
		eSM_NoScale,
		eSM_ShowAll,
		eSM_ExactFit,
		eSM_NoBorder
	};

	enum EAlignType
	{
		eAT_Center,
		eAT_TopCenter,
		eAT_BottomCenter,
		eAT_CenterLeft,
		eAT_CenterRight,
		eAT_TopLeft,
		eAT_TopRight,
		eAT_BottomLeft,
		eAT_BottomRight
	};
	
	//! Initialization.
	virtual bool Load(tukk pFilePath, u32 options = DEFAULT, u32 cat = eCat_Default) = 0;

	// Rendering
	virtual void           SetBackgroundColor(const ColorB& color) = 0;
	virtual void           SetBackgroundAlpha(float alpha) = 0;
	virtual float          GetBackgroundAlpha() const = 0;
	virtual void           SetViewport(i32 x0, i32 y0, i32 width, i32 height, float aspectRatio = 1.0f) = 0;
	virtual void           GetViewport(i32& x0, i32& y0, i32& width, i32& height, float& aspectRatio) const = 0;
	virtual void           SetViewScaleMode(EScaleModeType scaleMode) = 0;
	virtual EScaleModeType GetViewScaleMode() const = 0;
	virtual void           SetViewAlignment(EAlignType viewAlignment) = 0;
	virtual EAlignType     GetViewAlignment() const = 0;
	virtual void           SetScissorRect(i32 x0, i32 y0, i32 width, i32 height) = 0;
	virtual void           GetScissorRect(i32& x0, i32& y0, i32& width, i32& height) const = 0;
	virtual void           Advance(float deltaTime) = 0;
	virtual void           Render() = 0;
	virtual void           SetClearFlags(u32 clearFlags, ColorF clearColor = Clr_Transparent) = 0;
	virtual void           SetCompositingDepth(float depth) = 0;
	virtual void           StereoEnforceFixedProjectionDepth(bool enforce) = 0;
	virtual void           StereoSetCustomMaxParallax(float maxParallax = -1.0f) = 0;
	virtual void           AvoidStencilClear(bool avoid) = 0;
	virtual void           EnableMaskedRendering(bool enable) = 0; // special render mode for Drxsis 2 HUD markers (in stereo)
	virtual void           ExtendCanvasToViewport(bool extend) = 0;

	// Execution State
	virtual void         Restart() = 0;
	virtual bool         IsPaused() const = 0;
	virtual void         Pause(bool pause) = 0;
	virtual void         GotoFrame(u32 frameNumber) = 0;
	virtual bool         GotoLabeledFrame(tukk pLabel, i32 offset = 0) = 0;
	virtual u32 GetCurrentFrame() const = 0;
	virtual bool         HasLooped() const = 0;

	//! Callbacks & Events
	//! ##@{
	virtual void SetFSCommandHandler(IFSCommandHandler* pHandler, uk pUserData = 0) = 0;
	virtual void SetExternalInterfaceHandler(IExternalInterfaceHandler* pHandler, uk pUserData = 0) = 0;
	virtual void SendCursorEvent(const SFlashCursorEvent& cursorEvent) = 0;
	virtual void SendKeyEvent(const SFlashKeyEvent& keyEvent) = 0;
	virtual void SendCharEvent(const SFlashCharEvent& charEvent) = 0;
	//! ##@}

	virtual bool HitTest(float x, float y) const = 0;

	virtual void SetVisible(bool visible) = 0;
	virtual bool GetVisible() const = 0;
	virtual void SetImeFocus() = 0;

	// Action Script
	virtual bool         SetVariable(tukk pPathToVar, const SFlashVarValue& value) = 0;
	virtual bool         SetVariable(tukk pPathToVar, const IFlashVariableObject* pVarObj) = 0;
	virtual bool         GetVariable(tukk pPathToVar, SFlashVarValue& value) const = 0;
	virtual bool         GetVariable(tukk pPathToVar, IFlashVariableObject*& pVarObj) const = 0;
	virtual bool         IsAvailable(tukk pPathToVar) const = 0;
	virtual bool         SetVariableArray(EFlashVariableArrayType type, tukk pPathToVar, u32 index, ukk pData, u32 count) = 0;
	virtual u32 GetVariableArraySize(tukk pPathToVar) const = 0;
	virtual bool         GetVariableArray(EFlashVariableArrayType type, tukk pPathToVar, u32 index, uk pData, u32 count) const = 0;
	virtual bool         Invoke(tukk pMethodName, const SFlashVarValue* pArgs, u32 numArgs, SFlashVarValue* pResult = 0) = 0;

	virtual bool         CreateString(tukk pString, IFlashVariableObject*& pVarObj) = 0;
	virtual bool         CreateStringW(const wchar_t* pString, IFlashVariableObject*& pVarObj) = 0;
	virtual bool         CreateObject(tukk pClassName, const SFlashVarValue* pArgs, u32 numArgs, IFlashVariableObject*& pVarObj) = 0;
	virtual bool         CreateArray(IFlashVariableObject*& pVarObj) = 0;
	virtual bool         CreateFunction(IFlashVariableObject*& pFuncVarObj, IActionScriptFunction* pFunc, uk pUserData = 0) = 0;

	//! General property queries.
	//! ##@{
	virtual u32 GetFrameCount() const = 0;
	virtual float        GetFrameRate() const = 0;
	virtual i32          GetWidth() const = 0;
	virtual i32          GetHeight() const = 0;
	virtual size_t       GetMetadata(tuk pBuff, u32 buffSize) const = 0;
	virtual bool         HasMetadata(tukk pTag) const = 0;
	virtual tukk  GetFilePath() const = 0;
	//! ##@}

	//! Coordinate Translation.
	virtual void ResetDirtyFlags() = 0;

	//! Translates the screen coordinates to the client coordinates
	virtual void ScreenToClient(i32& x, i32& y) const = 0;

	//! Translates the client coordinates to the screen coordinates
	virtual void ClientToScreen(i32& x, i32& y) const = 0;
	// </interfuscator:shuffle>

	bool Invoke0(tukk pMethodName, SFlashVarValue* pResult = 0)
	{
		return Invoke(pMethodName, 0, 0, pResult);
	}
	bool Invoke1(tukk pMethodName, const SFlashVarValue& arg, SFlashVarValue* pResult = 0)
	{
		return Invoke(pMethodName, &arg, 1, pResult);
	}

#if defined(ENABLE_DYNTEXSRC_PROFILING)
	virtual void LinkDynTextureSource(const struct IDynTextureSource* pDynTexSrc) = 0;
#endif

	virtual IScaleformPlayback* GetPlayback() = 0;

protected:
	IFlashPlayer() {}
	virtual ~IFlashPlayer() {}
};

struct IFlashPlayer_RenderProxy
{
	enum EFrameType
	{
		EFT_Mono,
		EFT_StereoLeft,
		EFT_StereoRight
	};

	// <interfuscator:shuffle>
	virtual void RenderCallback(EFrameType ft) = 0;
	virtual void RenderPlaybackLocklessCallback(i32 cbIdx, EFrameType ft, bool finalPlayback = true) = 0;
	virtual void DummyRenderCallback(EFrameType ft) = 0;
	// </interfuscator:shuffle>

	virtual IScaleformPlayback* GetPlayback() = 0;

protected:
	virtual ~IFlashPlayer_RenderProxy() noexcept {}
};

struct IFlashVariableObject
{
	struct ObjectVisitor
	{
	public:
		virtual void Visit(tukk pName) = 0;

	protected:
		virtual ~ObjectVisitor() {}
	};
	// <interfuscator:shuffle>

	//! Lifetime.
	virtual void                  Release() = 0;
	virtual IFlashVariableObject* Clone() const = 0;

	//! Type check.
	virtual bool           IsObject() const = 0;
	virtual bool           IsArray() const = 0;
	virtual bool           IsDisplayObject() const = 0;

	virtual SFlashVarValue ToVarValue() const = 0;

	// AS Object support. These methods are only valid for Object type (which includes Array and DisplayObject types).
	virtual bool HasMember(tukk pMemberName) const = 0;
	virtual bool SetMember(tukk pMemberName, const SFlashVarValue& value) = 0;
	virtual bool SetMember(tukk pMemberName, const IFlashVariableObject* pVarObj) = 0;
	virtual bool GetMember(tukk pMemberName, SFlashVarValue& value) const = 0;
	virtual bool GetMember(tukk pMemberName, IFlashVariableObject*& pVarObj) const = 0;
	virtual void VisitMembers(ObjectVisitor* pVisitor) const = 0;
	virtual bool DeleteMember(tukk pMemberName) = 0;
	virtual bool Invoke(tukk pMethodName, const SFlashVarValue* pArgs, u32 numArgs, SFlashVarValue* pResult = 0) = 0;
	virtual bool Invoke(tukk pMethodName, const IFlashVariableObject** pArgs, u32 numArgs, SFlashVarValue* pResult = 0) = 0;

	//! AS Array support. These methods are only valid for Array type.
	virtual u32 GetArraySize() const = 0;
	virtual bool         SetArraySize(u32 size) = 0;
	virtual bool         SetElement(u32 idx, const SFlashVarValue& value) = 0;
	virtual bool         SetElement(u32 idx, const IFlashVariableObject* pVarObj) = 0;
	virtual bool         GetElement(u32 idx, SFlashVarValue& value) const = 0;
	virtual bool         GetElement(u32 idx, IFlashVariableObject*& pVarObj) const = 0;
	virtual bool         PushBack(const SFlashVarValue& value) = 0;
	virtual bool         PushBack(const IFlashVariableObject* pVarObj) = 0;
	virtual bool         PopBack() = 0;
	virtual bool         RemoveElements(u32 idx, i32 count = -1) = 0;

	// AS display object (MovieClips, Buttons, TextFields) support. These methods are only valid for DisplayObject type.
	virtual bool SetDisplayInfo(const SFlashDisplayInfo& info) = 0;
	virtual bool GetDisplayInfo(SFlashDisplayInfo& info) const = 0;
	virtual bool SetDisplayMatrix(const Matrix33& mat) = 0;
	virtual bool GetDisplayMatrix(Matrix33& mat) const = 0;
	virtual bool Set3DMatrix(const Matrix44& mat) = 0;
	virtual bool Get3DMatrix(Matrix44& mat) const = 0;
	virtual bool SetColorTransform(const SFlashCxform& cx) = 0;
	virtual bool GetColorTransform(SFlashCxform& cx) const = 0;
	virtual bool SetVisible(bool visible) = 0;

	// AS TextField support
	virtual bool SetText(tukk pText) = 0;
	virtual bool SetText(const wchar_t* pText) = 0;
	virtual bool SetTextHTML(tukk pHtml) = 0;
	virtual bool SetTextHTML(const wchar_t* pHtml) = 0;
	virtual bool GetText(SFlashVarValue& text) const = 0;
	virtual bool GetTextHTML(SFlashVarValue& html) const = 0;

	// AS MovieClip support. These methods are only valid for MovieClips.
	virtual bool CreateEmptyMovieClip(IFlashVariableObject*& pVarObjMC, tukk pInstanceName, i32 depth = -1) = 0;
	virtual bool AttachMovie(IFlashVariableObject*& pVarObjMC, tukk pSymbolName, tukk pInstanceName, i32 depth = -1, const IFlashVariableObject* pInitObj = 0) = 0;
	virtual bool GotoAndPlay(tukk pFrame) = 0;
	virtual bool GotoAndStop(tukk pFrame) = 0;
	virtual bool GotoAndPlay(u32 frame) = 0;
	virtual bool GotoAndStop(u32 frame) = 0;
	// </interfuscator:shuffle>
	bool         Invoke0(tukk pMethodName, SFlashVarValue* pResult = 0)
	{
		return Invoke(pMethodName, static_cast<SFlashVarValue*>(0), 0, pResult);
	}
	bool Invoke1(tukk pMethodName, const SFlashVarValue& arg, SFlashVarValue* pResult = 0)
	{
		return Invoke(pMethodName, &arg, 1, pResult);
	}
	bool RemoveElement(u32 idx)
	{
		return RemoveElements(idx, 1);
	}
	bool ClearElements()
	{
		return RemoveElements(0);
	}

protected:
	virtual ~IFlashVariableObject() {}
};

//! \cond INTERNAL
//! Bootstrapper to efficiently instantiate Flash assets on demand with minimal file IO.
struct IFlashPlayerBootStrapper
{
	// <interfuscator:shuffle>

	//! Lifetime.
	virtual void Release() = 0;

	//! Initialization.
	virtual bool Load(tukk pFilePath) = 0;

	//! Bootstrapping.
	virtual std::shared_ptr<IFlashPlayer> CreatePlayerInstance(u32 options = IFlashPlayer::DEFAULT, u32 cat = IFlashPlayer::eCat_Default) = 0;

	//! General property queries
	//! ##@{
	virtual tukk GetFilePath() const = 0;
	virtual size_t      GetMetadata(tuk pBuff, u32 buffSize) const = 0;
	virtual bool        HasMetadata(tukk pTag) const = 0;
	//! ##@}
	// </interfuscator:shuffle>

protected:
	virtual ~IFlashPlayerBootStrapper() {}
};
//! \endcond

//! Clients of IFlashPlayer implement this interface to receive action script events.
struct IFSCommandHandler
{
	virtual void HandleFSCommand(tukk pCommand, tukk pArgs, uk pUserData = 0) = 0;

protected:
	virtual ~IFSCommandHandler() {}
};

//! \cond INTERNAL
//! Clients of IFlashPlayer implement this interface to expose external interface calls.
struct IExternalInterfaceHandler
{
	virtual void HandleExternalInterfaceCall(tukk pMethodName, const SFlashVarValue* pArgs, i32 numArgs, uk pUserData = 0, SFlashVarValue* pResult = 0) = 0;

protected:
	virtual ~IExternalInterfaceHandler() {}
};

//! Clients of IFlashPlayer implement this interface to replace inject C++ code into Action Script.
struct IActionScriptFunction
{
	struct Params
	{
		IFlashPlayer*                pFromPlayer;
		uk                        pUserData;

		const IFlashVariableObject*  pThis;
		const IFlashVariableObject** pArgs;
		u32                 numArgs;
	};

	struct IReturnValue
	{
		//! Clients setting the return value in their implementation of Call() should think about "createManagedValue".
		//! For PODs its value doesn't have any meaning. When passing strings however it offers an optimization opportunity.
		//! If the string passed in "value" is managed by the client, there is no need to request internal creation of a managed value (a copy)
		//! as the (pointer to the) string will still be valid after Call() returns. However, if a pointer to a string on the stack is being
		//! passed, "createManagedValue" must be set to true!
		virtual void Set(const SFlashVarValue& value, bool createManagedValue = true) = 0;
		virtual void Set(const IFlashVariableObject* value) = 0;
	protected:
		virtual ~IReturnValue() {}
	};

	virtual void Call(const Params& params, IReturnValue* pRetVal) = 0;

protected:
	virtual ~IActionScriptFunction() {}
};

//! Clients of IFlashPlayer implement this interface to handle custom loadMovie API calls.
struct IFlashLoadMovieImage
{
	enum EFmt
	{
		eFmt_None,
		eFmt_RGB_888,
		eFmt_ARGB_8888,
	};

	// <interfuscator:shuffle>
	virtual void  Release() = 0;

	virtual i32   GetWidth() const = 0;
	virtual i32   GetHeight() const = 0;
	virtual i32   GetPitch() const = 0;
	virtual uk GetPtr() const = 0;
	virtual EFmt  GetFormat() const = 0;
	// </interfuscator:shuffle>

	bool IsValid() const
	{
		return GetPtr() && GetPitch() > 0 && GetWidth() > 0 && GetHeight() > 0;
	}

protected:
	virtual ~IFlashLoadMovieImage() {}
};

//! Clients of IFlashPlayer implement this interface to handle custom loadMovie API calls.
struct IFlashLoadMovieHandler
{
	virtual IFlashLoadMovieImage* LoadMovie(tukk pFilePath) = 0;

protected:
	virtual ~IFlashLoadMovieHandler() {}
};
//! \endcond

//! Variant type to pass values to flash variables.
struct SFlashVarValue
{
	union Data
	{
		bool           b;
		i32            i;
		u32   ui;
		double         d;
		float          f;
		tukk    pStr;
		const wchar_t* pWstr;
	};

	//! Enumerates types that can be sent to and received from flash.
	enum Type
	{
		eUndefined,
		eNull,

		eBool,
		eInt,
		eUInt,
		eDouble,
		eFloat,
		eConstStrPtr,
		eConstWstrPtr,

		eObject //!< Receive only!
	};

	SFlashVarValue(bool val)
		: type(eBool)
	{
		data.b = val;
	}
	SFlashVarValue(i32 val)
		: type(eInt)
	{
		data.i = val;
	}
	SFlashVarValue(u32 val)
		: type(eUInt)
	{
		data.ui = val;
	}
	SFlashVarValue(double val)
		: type(eDouble)
	{
		data.d = val;
	}
	SFlashVarValue(float val)
		: type(eFloat)
	{
		data.f = val;
	}
	SFlashVarValue(tukk val)
		: type(eConstStrPtr)
	{
		data.pStr = val;
	}
	SFlashVarValue(const wchar_t* val)
		: type(eConstWstrPtr)
	{
		data.pWstr = val;
	}
	static SFlashVarValue CreateUndefined()
	{
		return SFlashVarValue(eUndefined);
	}
	static SFlashVarValue CreateNull()
	{
		return SFlashVarValue(eNull);
	}

	bool GetBool() const
	{
		assert(type == eBool);
		return data.b;
	}
	i32 GetInt() const
	{
		assert(type == eInt);
		return data.i;
	}
	u32 GetUInt() const
	{
		assert(type == eUInt);
		return data.ui;
	}
	double GetDouble() const
	{
		assert(type == eDouble);
		return data.d;
	}
	float GetFloat() const
	{
		assert(type == eFloat);
		return data.f;
	}
	tukk GetConstStrPtr() const
	{
		assert(type == eConstStrPtr);
		return data.pStr;
	}
	const wchar_t* GetConstWstrPtr() const
	{
		assert(type == eConstWstrPtr);
		return data.pWstr;
	}

	Type GetType() const
	{
		return type;
	}
	bool IsUndefined() const
	{
		return GetType() == eUndefined;
	}
	bool IsNull() const
	{
		return GetType() == eNull;
	}
	bool IsBool() const
	{
		return GetType() == eBool;
	}
	bool IsInt() const
	{
		return GetType() == eInt;
	}
	bool IsUInt() const
	{
		return GetType() == eUInt;
	}
	bool IsDouble() const
	{
		return GetType() == eDouble;
	}
	bool IsFloat() const
	{
		return GetType() == eFloat;
	}
	bool IsConstStr() const
	{
		return GetType() == eConstStrPtr;
	}
	bool IsConstWstr() const
	{
		return GetType() == eConstWstrPtr;
	}
	bool IsObject() const
	{
		return GetType() == eObject;
	}

protected:
	Type type;
	Data data;

protected:
	//! Don't define default constructor to enforce efficient default initialization of argument lists!
	SFlashVarValue();

	SFlashVarValue(Type t)
		: type(t)
	{
		//data.d = 0;

		//static const Data init = {0};
		//data = init;

		memset(&data, 0, sizeof(data));
	}
};

//! \cond INTERNAL
//! Color transformation to control flash movie clips.
struct SFlashCxform
{
	ColorF mul; // Range: 0.0f - 1.0f
	ColorB add; // Range: 0 - 255
};

//! DisplayInfo structure for flash display objects (MovieClip, TextField, Button).
struct SFlashDisplayInfo
{
public:
	enum Flags
	{
		FDIF_X         = 0x001,
		FDIF_Y         = 0x002,
		FDIF_Z         = 0x004,

		FDIF_XScale    = 0x008,
		FDIF_YScale    = 0x010,
		FDIF_ZScale    = 0x020,

		FDIF_Rotation  = 0x040,
		FDIF_XRotation = 0x080,
		FDIF_YRotation = 0x100,

		FDIF_Alpha     = 0x200,
		FDIF_Visible   = 0x400,
	};

	SFlashDisplayInfo()
		: m_x(0), m_y(0), m_z(0)
		, m_xscale(0), m_yscale(0), m_zscale(0)
		, m_rotation(0), m_xrotation(0), m_yrotation(0)
		, m_alpha(0)
		, m_visible(false)
		, m_varsSet(0)
	{}

	SFlashDisplayInfo(float x, float y, float z,
	                  float xscale, float yscale, float zscale,
	                  float rotation, float xrotation, float yrotation,
	                  float alpha,
	                  bool visible,
	                  unsigned short varsSet)
		: m_x(x), m_y(y), m_z(z)
		, m_xscale(xscale), m_yscale(yscale), m_zscale(zscale)
		, m_rotation(rotation), m_xrotation(xrotation), m_yrotation(yrotation)
		, m_alpha(alpha)
		, m_visible(visible)
		, m_varsSet(varsSet)
	{}

	void Clear()                                            { m_varsSet = 0; }

	void SetX(float x)                                      { SetFlags(FDIF_X); m_x = x; }
	void SetY(float y)                                      { SetFlags(FDIF_Y); m_y = y; }
	void SetZ(float z)                                      { SetFlags(FDIF_Z); m_z = z; }

	void SetXScale(float xscale)                            { SetFlags(FDIF_XScale); m_xscale = xscale; } //!< 100 == 100%.
	void SetYScale(float yscale)                            { SetFlags(FDIF_YScale); m_yscale = yscale; } //!< 100 == 100%.
	void SetZScale(float zscale)                            { SetFlags(FDIF_ZScale); m_zscale = zscale; } //!< 100 == 100%.

	void SetRotation(float degrees)                         { SetFlags(FDIF_Rotation); m_rotation = degrees; }
	void SetXRotation(float degrees)                        { SetFlags(FDIF_XRotation); m_xrotation = degrees; }
	void SetYRotation(float degrees)                        { SetFlags(FDIF_YRotation); m_yrotation = degrees; }

	void SetAlpha(float alpha)                              { SetFlags(FDIF_Alpha); m_alpha = alpha; }
	void SetVisible(bool visible)                           { SetFlags(FDIF_Visible); m_visible = visible; }

	void SetPosition(float x, float y)                      { SetFlags(FDIF_X | FDIF_Y); m_x = x; m_y = y; }
	void SetPosition(float x, float y, float z)             { SetFlags(FDIF_X | FDIF_Y | FDIF_Z); m_x = x; m_y = y; m_z = z; }

	void SetScale(float xscale, float yscale)               { SetFlags(FDIF_XScale | FDIF_YScale); m_xscale = xscale; m_yscale = yscale; }
	void SetScale(float xscale, float yscale, float zscale) { SetFlags(FDIF_XScale | FDIF_YScale | FDIF_ZScale); m_xscale = xscale; m_yscale = yscale; m_zscale = zscale; }

	void Set(float x, float y, float z,
	         float xscale, float yscale, float zscale,
	         float rotation, float xrotation, float yrotation,
	         float alpha,
	         bool visible)
	{
		m_x = x;
		m_y = y;
		m_z = z;

		m_xscale = xscale;
		m_yscale = yscale;
		m_zscale = zscale;

		m_rotation = rotation;
		m_xrotation = xrotation;
		m_yrotation = yrotation;

		m_alpha = alpha;
		m_visible = visible;

		SetFlags(FDIF_X | FDIF_Y | FDIF_Z |
		         FDIF_XScale | FDIF_YScale | FDIF_ZScale |
		         FDIF_Rotation | FDIF_XRotation | FDIF_YRotation |
		         FDIF_Alpha | FDIF_Visible);
	}

	float GetX() const                       { return m_x; }
	float GetY() const                       { return m_y; }
	float GetZ() const                       { return m_z; }

	float GetXScale() const                  { return m_xscale; }
	float GetYScale() const                  { return m_yscale; }
	float GetZScale() const                  { return m_zscale; }

	float GetRotation() const                { return m_rotation; }
	float GetXRotation() const               { return m_xrotation; }
	float GetYRotation() const               { return m_yrotation; }

	float GetAlpha() const                   { return m_alpha; }
	bool  GetVisible() const                 { return m_visible; }

	bool  IsAnyFlagSet() const               { return 0 != m_varsSet; }
	bool  IsFlagSet(u32 flag) const { return 0 != (m_varsSet & flag); }

private:
	void SetFlags(u32 flags) { m_varsSet |= flags; }

private:
	float          m_x;
	float          m_y;
	float          m_z;

	float          m_xscale;
	float          m_yscale;
	float          m_zscale;

	float          m_rotation;
	float          m_xrotation;
	float          m_yrotation;

	float          m_alpha;
	bool           m_visible;

	unsigned short m_varsSet;
};
//! \endcond

//! Cursor input event sent to flash.
struct SFlashCursorEvent
{
public:
	//! Enumeration of possible cursor state.
	enum ECursorState
	{
		eCursorMoved,
		eCursorPressed,
		eCursorReleased,
		eWheel
	};

	SFlashCursorEvent(ECursorState state, i32 cursorX, i32 cursorY, i32 button = 0, float wheelScrollVal = 0.0f)
		: m_state(state)
		, m_cursorX(cursorX)
		, m_cursorY(cursorY)
		, m_button(button)
		, m_wheelScrollVal(wheelScrollVal)
	{
	}

	ECursorState m_state;
	i32          m_cursorX;
	i32          m_cursorY;
	i32          m_button;
	float        m_wheelScrollVal;
};

//! Key input event sent to flash.
struct SFlashKeyEvent
{
public:
	enum EKeyState
	{
		eKeyDown,
		eKeyUp
	};

	enum EKeyCode
	{
		VoidSymbol = 0,

		//! A through Z and numbers 0 through 9.
		//! ##@{
		A = 65,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		Num0 = 48,
		Num1,
		Num2,
		Num3,
		Num4,
		Num5,
		Num6,
		Num7,
		Num8,
		Num9,
		//! ##@}

		//! Numeric keypad.
		//! ##@{
		KP_0 = 96,
		KP_1,
		KP_2,
		KP_3,
		KP_4,
		KP_5,
		KP_6,
		KP_7,
		KP_8,
		KP_9,
		KP_Multiply,
		KP_Add,
		KP_Enter,
		KP_Subtract,
		KP_Decimal,
		KP_Divide,
		//! ##@}

		//! Function keys.
		//! ##@{
		F1 = 112,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		F13,
		F14,
		F15,
		//! ##@}

		//! Other keys.
		//! ##@{
		Backspace = 8,
		Tab,
		Clear     = 12,
		Return,
		Shift     = 16,
		Control,
		Alt,
		CapsLock = 20,        //!< Toggle.
		Escape   = 27,
		Space    = 32,
		PageUp,
		PageDown,
		End = 35,
		Home,
		Left,
		Up,
		Right,
		Down,
		Insert = 45,
		Delete,
		Help,
		NumLock      = 144,    //!< Toggle.
		ScrollLock   = 145,    //!< Toggle.

		Semicolon    = 186,
		Equal        = 187,
		Comma        = 188,    //!< Platform specific?
		Minus        = 189,
		Period       = 190,    //!< Platform specific?
		Slash        = 191,
		Bar          = 192,
		BracketLeft  = 219,
		Backslash    = 220,
		BracketRight = 221,
		Quote        = 222,
		//! ##@}

		//! Total number of keys.
		KeyCount
	};

	enum ESpecialKeyState
	{
		eShiftPressed  = 0x01,
		eCtrlPressed   = 0x02,
		eAltPressed    = 0x04,
		eCapsToggled   = 0x08,
		eNumToggled    = 0x10,
		eScrollToggled = 0x20
	};

	SFlashKeyEvent(EKeyState state, EKeyCode keyCode, u8 specialKeyState, u8 asciiCode, u32 wcharCode)
		: m_state(state)
		, m_keyCode(keyCode)
		, m_specialKeyState(specialKeyState)
		, m_asciiCode(asciiCode)
		, m_wcharCode(wcharCode)
	{
	}

	EKeyState     m_state;
	EKeyCode      m_keyCode;
	u8 m_specialKeyState;
	u8 m_asciiCode;
	u32  m_wcharCode;
};

//! Char event sent to flash.
struct SFlashCharEvent
{
public:
	SFlashCharEvent(u32 wCharCode, u8 keyboardIndex = 0)
		: m_wCharCode(wCharCode)
		, m_keyboardIndex(keyboardIndex)
	{
	}

	u32 m_wCharCode;

	//! The index of the physical keyboard controller.
	u8 m_keyboardIndex;
};
