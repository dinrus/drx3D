// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __FacialEdContext_h__
#define __FacialEdContext_h__
#pragma once

#include <DrxAnimation/IFacialAnimation.h>
#include <DrxInput/IJoystick.h>

struct ICharacterInstance;
struct IFacialModel;
struct IFacialInstance;
struct IFacialEffectorsLibrary;
struct IFacialEffector;

enum EFacialEdEvent
{
	EFD_EVENT_REDRAW_PREVIEW,
	EFD_EVENT_ADD,
	EFD_EVENT_REMOVE,
	EFD_EVENT_CHANGE,
	EFD_EVENT_CHANGE_RELOAD, // Big change to item, needs reload of controls
	EFD_EVENT_SELECT_EFFECTOR,
	EFD_EVENT_SELECT_CHANNEL,
	EFD_EVENT_SLIDERS_CHANGE,
	EFD_EVENT_LIBRARY_CHANGE,
	EFD_EVENT_LIBRARY_UNDO,
	EFD_EVENT_SEQUENCE_CHANGE,
	EFD_EVENT_SEQUENCE_LOAD,
	EFD_EVENT_SEQUENCE_UNDO,
	EFD_EVENT_SEQUENCE_TIMES_CHANGE,
	EFD_EVENT_SEQUENCE_TIME,  // Send when sequence time change.
	EFD_EVENT_SOUND_CHANGE,
	EFD_EVENT_SKELETON_ANIMATION_CHANGE,
	EFD_EVENT_SPLINE_CHANGE,
	EFD_EVENT_SPLINE_CHANGE_CURRENT, // Change to splines only around current time (no need for full redraw)
	EFD_EVENT_START_EDITTING_JOYSTICKS,
	EFD_EVENT_STOP_EDITTING_JOYSTICKS,
	EFD_EVENT_START_CHANGING_SPLINES,
	EFD_SPLINES_NEED_ACTIVATING,
	EFD_EVENT_JOYSTICK_SET_CHANGED,
	EFD_EVENT_SEQUENCE_PLAY_OR_STOP,
	EFD_EVENT_ANIMATE_CAMERA_CHANGED,
	EFD_EVENT_ANIMATE_SKELETON_CHANGED,
	EFD_EVENT_SEQUENCE_TIME_RANGE_CHANGE  // Send when time range sequence change. Doesnt reload everything like sequence_change
};

//////////////////////////////////////////////////////////////////////////
struct IFacialEdListener
{
	virtual void OnFacialEdEvent(EFacialEdEvent event, IFacialEffector* pEffector, i32 nChannelCount = 0, IFacialAnimChannel** ppChannels = 0) = 0;
};

class IUndoLibraryChangeContext
{
public:
	virtual void OnLibraryUndo() = 0;
};

enum SequenceChangeType
{
	FE_SEQ_CHANGE_TOTAL,
	FE_SEQ_CHANGE_SOUND_TIMES
};
class IUndoSequenceChangeContext
{
public:
	virtual void OnSequenceUndo(SequenceChangeType changeType) = 0;
};

class CFacialVideoFrameReader
{
public:
	CFacialVideoFrameReader();
	~CFacialVideoFrameReader();

	i32                  AddAVI(tukk filename, float time);
	void                 DeleteAVI(i32 index);
	void                 DeleteUnusedAVIs();
	i32                  GetLastLoadedAVIFrameFromTime(float time);
	i32                  GetAVICount() const;
	i32                  GetWidth(float time);
	i32                  GetHeight(float time);
	u8k* GetFrame(float time);

private:
	void FindAVIFrame(float time, IAVI_Reader*& pAVI, i32& frame);

	struct AVIEntry
	{
		AVIEntry() : refCount(0), pAVI(0) {}
		i32          refCount;
		IAVI_Reader* pAVI;
	};

	typedef std::map<string, AVIEntry> AVIMap;

	struct AVIEntity
	{
		AVIMap::iterator itAVI;
		float            time;
	};

	AVIMap                 m_avis;
	std::vector<AVIEntity> m_aviEntities;
	i32                    m_lastLoadedIndex;
};

//////////////////////////////////////////////////////////////////////////
typedef struct
{
	float fX[16], fY[16];
}tMarkerDebugInfo;

//////////////////////////////////////////////////////////////////////////
// Project definition.
//////////////////////////////////////////////////////////////////////////
class CFacialEdContext : private IUndoLibraryChangeContext, private IUndoSequenceChangeContext
{
public:
	CString                       projectFile;
	bool                          bLibraryModfied;
	bool                          bProjectModfied;
	bool                          bSequenceModfied;
	bool                          bJoysticksModfied;

	ICharacterInstance*           pCharacter;
	IFacialModel*                 pModel;
	IFacialInstance*              pInstance;
	IFacialEffectorsLibrary*      pLibrary;
	IFaceState*                   pFaceState;
	IFacialEffector*              pSelectedEffector;
	IFacialAnimChannel*           pSelectedChannel;

	UINT                          expressionClipboardFormat;
	UINT                          channelClipboardFormat;

	i32                           m_nMarkers;
	i32                           m_nMarkersDebug;
	std::vector<tMarkerDebugInfo> m_lstPosDebug;

public:
	CFacialEdContext();
	~CFacialEdContext();

	bool          LoadProject(const CString& filename);
	bool          SaveProject(const CString& filename);

	void          NewLibrary();
	bool          LoadLibrary(const CString& filename);
	bool          ImportLibrary(const CString& filename, CWnd* parentWindow);
	bool          ImportLibrary(IFacialEffectorsLibrary* pLibrary, CWnd* parentWindow);
	bool          SaveLibrary(const CString& filename, IFacialEffectorsLibrary* pLibrary = 0);
	void          StoreLibraryUndo();

	void          NewSequence();
	bool          LoadSequence(const CString& filename);
	bool          ImportSequence(const CString& filename, CWnd* parentWindow);
	bool          ImportSequence(IFacialAnimSequence* pImportSequence, CWnd* parentWindow);
	bool          SaveSequence(const CString& filename, IFacialAnimSequence* pSequence = 0);
	void          StoreSequenceUndo(SequenceChangeType changeType = FE_SEQ_CHANGE_TOTAL);

	void          NewJoystickSet();
	bool          LoadJoystickSet(const CString& filename);
	bool          SaveJoystickSet(const CString& filename);
	void          SetJoystickSet(IJoystickSet* pJoystickSet);
	IJoystickSet* GetJoystickSet();

	void          LoadCharacter(const CString& filename);
	void          SetCharacter(ICharacterInstance* pChar);

	void          RegisterListener(IFacialEdListener* pListener);
	void          UnregisterListner(IFacialEdListener* pListener);

	void          SelectEffector(IFacialEffector* pEffector);
	void          SelectChannel(IFacialAnimChannel* pSelectedChannel);
	void          ClearHighlightedChannels();
	void          AddHighlightedChannel(IFacialAnimChannel* pSelectedChannel);
	void          SetModified(IFacialEffector* pEffector = NULL);

	void          SetFromSliders(std::vector<float>& weights, std::vector<float>& balances);
	void          GetFromSliders(std::vector<float>& weights, std::vector<float>& balances);

	void          UpdateSelectedFromSliders();
	void          UpdateSelectedFromSequenceTime(float minimumWeight);

	// Send event about facial effector to all listeners.
	void                     SendEvent(EFacialEdEvent event, IFacialEffector* pEffector = NULL, i32 nNumChannels = 0, IFacialAnimChannel** ppChannels = 0);

	void                     SetLibrary(IFacialEffectorsLibrary* pLib);
	IFacialEffectorsLibrary* GetLibrary();

	void                     SetSequence(IFacialAnimSequence* pSequence);
	IFacialAnimSequence*     GetSequence() { return m_pCurrentSequence; };

	void                     SetSequenceTime(float fTime);
	float                    GetSequenceTime() { return m_fSequenceTime; };

	void                     PlaySequence(bool bPlay);
	bool                     IsPlayingSequence() const { return m_bPlaying; }

	void                     SetSkeletonAnimation(const CString& filename, float startTime);
	void                     PreviewEffector(IFacialEffector* pEffector, float fWeight = 1.0f);
	void                     StopPreviewingEffector();

	void                     GetAllEffectors(std::vector<IFacialEffector*>& effectors, IFacialEffector* pRootEffector, EFacialEffectorType ofType);
	void                     SupressFacialEvents(bool bSupress) { m_bSupressEvents = bSupress; }

	void                     BindJoysticks();

	void                     HandleDropToSequence(IFacialAnimChannel* pParentChannel, COleDataObject* pDataObject);
	void                     DoChannelDragDrop(IFacialAnimChannel* pChannel);
	void                     HandleChannelDropToSequence(IFacialAnimChannel* pParentChannel, COleDataObject* pDataObject);
	IFacialAnimChannel*      GetChannelFromDataSource(COleDataObject* pDataObject);
	IFacialAnimChannel*      GetChannelFromDescriptor(const string& descriptor);
	void                     HandleExpressionNameDropToSequence(IFacialAnimChannel* pParentChannel, COleDataObject* pDataObject);
	string                   GetChannelDescriptorFromDataSource(COleDataObject* dataObject);

	void                     DoExpressionNameDragDrop(IFacialEffector* pExpression, IFacialEffector* pParentExpression);
	void                     HandleExpressionNameDropToLibrary(IFacialEffector* pParentExpression, COleDataObject* pDataObject, const bool isToGarbageFolder = false);
	void                     CopyExpressionNameToClipboard(IFacialEffector* pEffector);
	void                     PasteExpressionFromClipboard();
	string                   GetExpressionNameFromClipboard();
	string                   GetExpressionNameFromDataSource(COleDataObject* dataObject);

	float                    GetVideoLength();

	//////////////////////////////////////////////////////////////////////////

	//void	SetNumberVideoFrames(i32 nFrames);
	//i32		GetNumberVideoFrames() { return(m_nVideoFrames); }
	//void	SetCurrentFrame(i32 nFrame) { m_nCurrFrame=nFrame; }
	//i32		GetCurrentFrame()						{ return(m_nCurrFrame); }
	//void	SetVideoWidth(i32 nW)		{ m_nVideoWidth=nW; }
	//void	SetVideoHeight(i32 nH)	{ m_nVideoHeight=nH; }
	//void	SetVideoFPS(i32 nFPS)		{ m_nVideoFPS=nFPS; }
	//i32		GetVideoWidth()		{ return(m_nVideoWidth); }
	//i32		GetVideoHeight()	{ return(m_nVideoHeight); }
	//i32		GetVideoFPS()		{		return(m_nVideoFPS); }
	//const char	*GetVideoFolder() {	if (m_szVideoFolder[0])	return(m_szVideoFolder); return(NULL); }
	//void				SetVideoFolder(tukk szFolder) {	strcpy(m_szVideoFolder,szFolder); }
	//u8k	*GetVideoFrame(i32 nFrame);
	CFacialVideoFrameReader& GetVideoFrameReader() { return m_videoFrameReader; }
	//////////////////////////////////////////////////////////////////////////

	CFacialVideoFrameReader m_videoFrameReader;

	float                     GetTimelineLength();

	void                      SetPoseFromExpression(IFacialEffector* pEffector, float weight, float time);

	template<typename H> void CheckMorphs(H& handler)
	{
		MorphCheckHandler<H> handlerObject(handler);
		CheckMorphs(&handlerObject);
	}

	class IMorphCheckHandler
	{
	public:
		virtual void HandleMorphError(tukk morphName) = 0;
	};

	enum MoveToKeyDirection
	{
		MoveToKeyDirectionForward,
		MoveToKeyDirectionBackward
	};
	void  MoveToKey(MoveToKeyDirection direction);
	void  MoveToFrame(MoveToKeyDirection direction);

	void  SetAnimateSkeleton(bool bAnimateSkeleton);
	bool  GetAnimateSkeleton() const;
	void  SetAnimateCamera(bool bAnimateCamera);
	bool  GetAnimateCamera() const;
	void  SetOverlapSounds(bool bSetOverlapSounds);
	bool  GetOverlapSounds() const;

	void  SetPreviewWeight(float fPreviewWeight);
	float GetPreviewWeight() const;

	void  ConvertSequenceToCorrectFrameRate(IFacialAnimSequence* pSequence);

protected:
	template<typename H> class MorphCheckHandler : public IMorphCheckHandler
	{
	public:
		MorphCheckHandler(H& handler) : m_handler(handler) {}
		virtual void HandleMorphError(tukk morphName) { m_handler(morphName); }
	private:
		H& m_handler;
	};

	void CheckMorphs(IMorphCheckHandler* handler);

	typedef std::set<IFacialEffector*> EffectorSet;
	template<typename H> void ForEachEffector(H& handler);
	template<typename H> void ForEachEffectorRecurse(H& handler, IFacialEffector* pEffector, EffectorSet& visitedEffectors);

	void                      RegisterClipboardFormats();

	COleDataSource*           CreateExpressionNameDataSource(IFacialEffector* pEffector);
	COleDataSource*           CreateChannelDescriptorDataSource(IFacialAnimChannel* pChannel);
	string                    GetDescriptorForChannel(IFacialAnimChannel* pChannel);

	void                      UpdateSkeletonAnimationStatus();

private:
	// IUndoLibraryChangeContext
	virtual void OnLibraryUndo();

	// IUndoSequenceChangeContext
	virtual void OnSequenceUndo(SequenceChangeType changeType);

	_smart_ptr<IFacialAnimSequence> m_pCurrentSequence;
	_smart_ptr<IJoystickSet>        m_pJoysticks;
	std::vector<IFacialEdListener*> m_listeners;
	std::vector<float>              m_weights;
	std::vector<float>              m_balances;
	IFacialEffector*                m_pSlidersPreviewEffector;
	std::set<IFacialAnimChannel*>   m_highlightedChannels;

	float                           m_fSequenceTime;
	//float m_fVideoLength;
	//i32		m_nVideoWidth;
	//i32		m_nVideoHeight;
	//i32		m_nVideoFPS;
	//i32		m_nCurrFrame;
	//i32		m_nVideoFrames;
	//char	m_szVideoFolder[256];
	//IAVI_Reader	*m_pAVIReader;

	bool  m_bPlaying;
	bool  m_bSupressEvents;
	bool  m_bAnimateSkeleton;
	bool  m_bAnimateCamera;
	bool  m_bOverlapSounds;
	float m_fPreviewWeight;

public:
	float m_fC3DScale;

};

#endif // __FacialEdContext_h__

