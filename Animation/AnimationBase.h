// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Animation/cvars.h>
#include <drx3D/Animation/AnimationUpr.h>

// Угловые утилиты
#define DEG2COS(a)   (drxmath::cos((a) * (gf_PI / 180.0f)))
#define COS2DEG(a)   (drxmath::acos(a) * (180.0f / gf_PI))
#define HCOS2RAD(a)  (drxmath::acos(a) * 2.0f)
#define DEG2HCOS(a)  (drxmath::cos((a * 0.5f) * (gf_PI / 180.0f)))
#define DEG2HSIN(a)  (drxmath::sin((a * 0.5f) * (gf_PI / 180.0f)))
#define HCOS2DEG(a)  (drxmath::acos(a) * 2.0f * (180.0f / gf_PI))

#define APX_NUM_OF_CGA_ANIMATIONS (200) //to avoid unnecessary resizing of a dynamic array
#define MAX_FEET_AMOUNT           (4)   //this is used for feetlock

#define MAX_EXEC_QUEUE            (0x8u)   //max anims in exec-queue
#define NUM_CAF_FILES             (0x4000) //to avoid resizing in production mode
#define NUM_AIM_FILES             (0x0400) //to avoid resizing in production mode

#define STORE_ANIMATION_NAMES

ILINE void g_LogToFile(tukk szFormat, ...) PRINTF_PARAMS(1, 2);

class CharacterUpr;

struct ISystem;
struct IConsole;
struct ITimer;
struct ILog;
struct IDrxPak;
struct IStreamEngine;

struct IRenderer;
struct IPhysicalWorld;
struct I3DEngine;
class CCamera;
struct SParametricSamplerInternal;

//////////////////////////////////////////////////////////////////////////
// В процессе есть только один ISystem, также как и один менеджер персонажа CharacterUpr.
// Поэтому этот ISystem хранится в глобальном указателе, инициализируется при создании
// этого CharacterUpr и полноценен до момента его разрушения.
// По разрушению он обнуляется в NULL. В любом случае не должно оставаться ни одного
// объекта, нуждающегося в нём, поскольку система анимации активна только тогда,
// когда есть "живой" объект менеджера Upr.
//////////////////////////////////////////////////////////////////////////

//Глобальные интерфейсы
extern ISystem* g_pISystem;
extern IConsole* g_pIConsole;
extern ITimer* g_pITimer;
extern ILog* g_pILog;
extern IDrxPak* g_pIPak;
extern IStreamEngine* g_pIStreamEngine;

extern IRenderer* g_pIRenderer;

#define g_pAuxGeom gEnv->pRenderer->GetIRenderAuxGeom()

extern IPhysicalWorld* g_pIPhysicalWorld;
extern I3DEngine* g_pI3DEngine;

extern bool g_bProfilerOn;
extern f32 g_fCurrTime;
extern f32 g_AverageFrameTime;
extern CAnimation g_DefaultAnim;
extern CharacterUpr* g_pCharacterUpr;
extern QuatT g_IdentityQuatT;
extern AABB g_IdentityAABB;
extern i32 g_nRenderThreadUpdateID;
extern DynArray<string> g_DataMismatch;
extern SParametricSamplerInternal* g_parametricPool;
extern bool* g_usedParametrics;
extern i32 g_totalParametrics;
extern u32 g_DefaultTransitionInterpolationType;

#define g_AnimationUpr g_pCharacterUpr->GetAnimationUpr()

// Инициализует глобальные значения - просто запоминается указатель на систему, который
// сохраняется действенным до деинициализации этого класса (происходящей при деструкции
// экземпляра CharacterUpr ("МенеджерПерсонажа"). Также инициализуются консольные переменные.
ILINE void g_InitInterfaces()
{
	assert(g_pISystem);
	g_pIConsole = gEnv->pConsole;
	g_pITimer = gEnv->pTimer;
	g_pILog = g_pISystem->GetILog();
	g_pIPak = gEnv->pDrxPak;
	g_pIStreamEngine = g_pISystem->GetStreamEngine();

	//Эти указатели единожды инициализуются здесь, затем каждый фрейм обновляется через
	//CharacterUpr::Update().
	g_pIRenderer = g_pISystem->GetIRenderer();
	g_pIPhysicalWorld = g_pISystem->GetIPhysicalWorld();
	g_pI3DEngine = g_pISystem->GetI3DEngine();

	Console::GetInst().Init();
}

// Деинициализует этот класс - реально только обнуляет (NULLs) системные указатели и удаляет переменные.
ILINE void g_DeleteInterfaces()
{
	g_pISystem = NULL;
	g_pITimer = NULL;
	g_pILog = NULL;
	g_pIConsole = NULL;
	g_pIPak = NULL;
	g_pIStreamEngine = NULL;
	;

	g_pIRenderer = NULL;
	g_pIPhysicalWorld = NULL;
	g_pI3DEngine = NULL;
}

// Общая единая структура для статистики времени\памяти.
struct AnimStatisticsInfo
{
	int64 m_iDBASizes;
	int64 m_iCAFSizes;
	int64 m_iInstancesSizes;
	int64 m_iModelsSizes;

	AnimStatisticsInfo() : m_iDBASizes(0), m_iCAFSizes(0), m_iInstancesSizes(0), m_iModelsSizes(0) {}

	void Clear()            { m_iDBASizes = 0; m_iCAFSizes = 0; m_iInstancesSizes = 0; m_iModelsSizes = 0;  }
	// i32 - for convenient output to log
	i32  GetInstancesSize() { return (i32)(m_iInstancesSizes - GetModelsSize() - m_iDBASizes - m_iCAFSizes); }
	i32  GetModelsSize()    { return (i32)(m_iModelsSizes - m_iDBASizes - m_iCAFSizes); }
};
extern AnimStatisticsInfo g_AnimStatisticsInfo;

#define ENABLE_GET_MEMORY_USAGE 1

#ifndef AUTO_PROFILE_SECTION
	#pragma message ("Предупреждение: ITimer не вложен")
#else
	#undef AUTO_PROFILE_SECTION
#endif

#define AUTO_PROFILE_SECTION(g_fTimer) CITimerAutoProfiler<double> __section_auto_profiler(g_pITimer, g_fTimer)

#define DEFINE_PROFILER_FUNCTION()     DRX_PROFILE_FUNCTION(PROFILE_ANIMATION)
#define DEFINE_PROFILER_SECTION(NAME)  DRX_PROFILE_REGION(PROFILE_ANIMATION, NAME)
