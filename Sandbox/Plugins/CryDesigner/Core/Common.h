// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Plane.h"
#include "Line.h"
#include <Drx3DEngine/IIndexedMesh.h>
#include <drx3D/CoreX/Serialization/Serializer.h>
#include <drx3D/CoreX/Serialization/IArchiveHost.h>
#include <drx3D/CoreX/Serialization/Decorators/Range.h>
#include "FlexibleMesh.h"

#ifdef DEBUG
	#define ENABLE_DESIGNERASSERTION
#endif

#ifdef ENABLE_DESIGNERASSERTION
	#define DESIGNER_ASSERT(condition) assert(condition)
#else
	#define DESIGNER_ASSERT(condition) assert(1)
#endif

namespace Designer
{

class ElementSet;
class Model;
class ModelCompiler;
class Polygon;

struct TexInfo
{
	float shift[2];
	float scale[2];
	float rotate;

	TexInfo()
	{
		shift[0] = shift[1] = 0;
		scale[0] = scale[1] = 1.0f;
		rotate = 0;
	}

	TexInfo(const TexInfo& ti)
	{
		shift[0] = ti.shift[0];
		shift[1] = ti.shift[1];

		scale[0] = ti.scale[0];
		scale[1] = ti.scale[1];

		rotate = ti.rotate;
	}

	void Load(XmlNodeRef& xmlNode);
	void Save(XmlNodeRef& xmlNode);
};

enum EBooleanOperationEnum
{
	eBOE_Union,
	eBOE_Intersection,
	eBOE_Difference,
};

enum EPointPosEnum
{
	ePP_BORDER,
	ePP_INSIDE,
	ePP_OUTSIDE
};

enum EClipType
{
	eCT_Positive,
	eCT_Negative,
};

enum ECompilerFlags
{
	BRF_MESH_VALID  = 0x40,
	BRF_MODIFIED    = 0x80,
	BRF_SUB_OBJ_SEL = 0x100,
	BRF_SELECTED    = 0x200,
};

static tukk BrushDesignerDefaultMaterial("%ENGINE%/EngineAssets/TextureMsg/DefaultSolids");
static tukk BrushSolidDefaultMaterial("%ENGINE%/EngineAssets/TextureMsg/DefaultSolids");
static i32k kLineThickness(2);
static i32k kChosenLineThickness(7);
static const ColorB kSelectedColor(214, 150, 0, 255);
static const ColorB kWeakSelectedColor(112, 75, 0, 255);
static const ColorB kResizedPolygonColor(128, 128, 240, 255);
static const ColorB PolygonLineColor(100, 100, 200, 255);
static const ColorB PolygonInvalidLineColor(255, 100, 0, 255);
static const ColorB PolygonParallelToAxis(100, 200, 200, 255);
static u32 s_nGlobalBrushFileId = 0;
static u32 s_nGlobalBrushDesignerFileId = 0;
static const BrushFloat PI = 3.14159265358979323846;
static const BrushFloat PI2 = PI * 2.0;
static const BrushFloat kNormalGap(0.01);
static const BrushFloat kEnoughBigNumber(3e10);
static constexpr DrxGUID ExcludeModeCameraID = "f7fd35da-9c82-4f4f-b451-2dc9d014ff28"_drx_guid;
static i32k kDefaultSubdivisionNum = 16;
static i32k kDefaultCurveEdgeNum = 18;
static i32k kDefaultViewDist = 100;
static const ColorB kElementBoxColor(0xFFFFAAAA);
static const BrushFloat kElementEdgeThickness = 6;
static i32k kSmoothingGroupIDNumber = 32;
static i32k kMaximumSubdivisionLevel = 5;
static tukk sessionName = "Settings\\Designer";

typedef enum
{
	eShelf_Any          = -1, // Any shelf. Used in some tools to operate on both shelves
	eShelf_Base         = 0,  // Shelf where the current model exists
	eShelf_Construction = 1,  // Shelf where new geometry is constructed
} ShelfID;

// Maximum number of shelves. Used to allocate and iterate through shelf arrays.
// Kept outside enum so it's not possible to use as valid parameter in functions
static i32k cShelfMax = 2;

// BrushEdge3D - A picked Edge, BrushVec3 - A picked point in the middle of the edge
typedef std::vector<std::pair<BrushEdge3D, BrushVec3>> EdgeQueryResult;

enum EPickingFlag
{
	ePF_Vertex  = BIT(0),
	ePF_Edge    = BIT(1),
	ePF_Polygon = BIT(2),
	ePF_All     = ePF_Vertex | ePF_Edge | ePF_Polygon
};

enum EClipObjective
{
	eCO_JustClip,
	eCO_Union0,
	eCO_Union1,
	eCO_Subtract,
	eCO_Intersection0,
	eCO_Intersection0IncludingCoSame,
	eCO_Intersection1,
	eCO_Intersection1IncludingCoDiff,
};

enum EIntersectionType
{
	eIT_None,
	eIT_Intersection,
	eIT_JustTouch,
};

enum EResetXForm
{
	eResetXForm_Rotation = BIT(0),
	eResetXForm_Scale    = BIT(1),
	eResetXForm_Position = BIT(2),
	eResetXForm_All      = eResetXForm_Rotation | eResetXForm_Scale | eResetXForm_Position
};

enum EDBResetFlag
{
	eDBRF_Plane  = BIT(0),
	eDBRF_Vertex = BIT(1),
	eDBRF_ALL    = 0xFFFFFFFF
};

static const float kDefaultStepRise = 0.33f;
static i32k kDefaultStepNumber = 16;

BrushVec3        WorldPos2ScreenPos(const BrushVec3& worldPos);
BrushVec3        ScreenPos2WorldPos(const BrushVec3& screenPos);
void             DrawSpot(DisplayContext& dc, const BrushMatrix34& worldTM, const BrushVec3& pos, const ColorB& color, float fSize = 5.0f);

EOperationResult SubtractEdge3D(const BrushEdge3D& inEdge0, const BrushEdge3D& inEdge1, BrushEdge3D outEdge[2]);
EOperationResult IntersectEdge3D(const BrushEdge3D& inEdge0, const BrushEdge3D& inEdge1, BrushEdge3D& outEdge);
ESplitResult     Split(
  const BrushPlane& plane,
  const BrushLine& splitLine,
  const std::vector<BrushEdge3D>& splitEdges,
  const BrushEdge3D& inEdge,
  BrushEdge3D& positiveEdge,
  BrushEdge3D& negativeEdge);

void  MakeSectorOfCircle(BrushFloat fRadius, const BrushVec2& vCenter, BrushFloat startRadian, BrushFloat diffRadian, i32 nSegmentCount, std::vector<BrushVec2>& outVertexList);
float ComputeAnglePointedByPos(const BrushVec2& vCenter, const BrushVec2& vPointedPos);
template<class T>
bool  FindVertexWithMinimum(std::vector<T> vertexList, i32 nElementIndex, i32& outMinimumIndex)
{
	i32 nMinimumIndex = -1;
	BrushFloat minimumValue = 3e10f;
	for (i32 i = 0, iVertexSize(vertexList.size()); i < iVertexSize; ++i)
	{
		if (vertexList[i][nElementIndex] < minimumValue)
		{
			nMinimumIndex = i;
			minimumValue = vertexList[i][nElementIndex];
		}
	}
	if (nMinimumIndex == -1)
		return false;
	outMinimumIndex = nMinimumIndex;
	return true;
}

template<class T>
T GetAdjustedFloatToAvoidZero(T x)
{
	T _x = std::abs(x) < kDesignerEpsilon ? kDesignerEpsilon : x;
	if (x < 0)
		_x *= -1;
	return _x;
}

bool                     DoesEquivalentExist(std::vector<BrushEdge3D>& edgeList, const BrushEdge3D& edge, i32* pOutIndex = NULL);
bool                     DoesEquivalentExist(std::vector<BrushVec3>& vertexList, const BrushVec3& vertex);
bool                     ComputePlane(const std::vector<Vertex>& vList, BrushPlane& outPlane);
BrushVec3                ComputeNormal(const BrushVec3& v0, const BrushVec3& v1, const BrushVec3& v2);
void                     GetLocalViewRay(const BrushMatrix34& worldTM, IDisplayViewport* view, CPoint point, BrushRay& outRay);
void                     DrawPlane(DisplayContext& dc, const BrushVec3& vPivot, const BrushPlane& plane, float size = 12.0f);
BrushFloat               SnapGrid(BrushFloat fValue);
BrushFloat               Snap(BrushFloat pos);

void                     Write2Buffer(std::vector<char>& buffer, ukk pData, i32 nDataSize);
i32                      ReadFromBuffer(std::vector<char>& buffer, i32 nPos, uk pData, i32 nDataSize);

bool                     GetIntersectionOfRayAndAABB(const BrushVec3& vPoint, const BrushVec3& vDir, const AABB& aabb, BrushFloat* pOutDistance);

i32                      FindShortestEdge(std::vector<BrushEdge3D>& edges);
std::vector<BrushEdge3D> FindNearestEdges(CViewport* pViewport, const BrushMatrix34& worldTM, std::vector<std::pair<BrushEdge3D, BrushVec3>>& edges);

BrushVec3                GetElementBoxSize(IDisplayViewport* pView, bool b2DViewport, const BrushVec3& vWorldPos);
bool                     AreTwoPositionsClose(const BrushVec3& modelPos0, const BrushVec3& modelPos1, const BrushMatrix34& worldTM, IDisplayViewport* pViewport, BrushFloat fBoundRadiuse, BrushFloat* pOutDistance = NULL);
bool                     HitTestEdge(const BrushRay& ray, const BrushVec3& vNormal, const BrushEdge3D& edge, IDisplayViewport* pView = NULL);
float                    GetDistanceOnScreen(const Vec3& v0, const Vec3& v1, IDisplayViewport* pViewport);
bool                     PickPosFromWorld(IDisplayViewport* view, const CPoint& point, Vec3& outPickedPos);
BrushVec3                CorrectVec3(const BrushVec3& v);
bool                     IsConvex(std::vector<BrushVec2>& vList);
bool                     HasEdgeInEdgeList(const std::vector<BrushEdge3D>& edgeList, const BrushEdge3D& edge);
bool                     HasVertexInVertexList(const std::vector<BrushVec3>& vertexList, const BrushVec3& v);

struct MainContext
{
	MainContext() : pObject(NULL), pCompiler(NULL), pModel(NULL) {}
	MainContext(CBaseObject* _pObject, ModelCompiler* _pCompiler, Model* _pModel) : pObject(_pObject), pCompiler(_pCompiler), pModel(_pModel) {}
	bool IsValid() const { return pObject && pCompiler && pModel; }
	CBaseObject*   pObject;
	ModelCompiler* pCompiler;
	Model*         pModel;
	ElementSet*    pSelected;
};

struct LessConstCharStrCmp
{
	bool operator()(tukk a, tukk b) const
	{
		return strcmp(a, b) < 0;
	}
};

stack_string                                GetSaveStateFilePath(tukk name);
bool                                        LoadSettings(const Serialization::SStruct& outObj, tukk name);
bool                                        SaveSettings(const Serialization::SStruct& outObj, tukk name);

inline Serialization::RangeDecorator<float> LENGTH_RANGE(float& value)    { return Serialization::Range(value, 0.01f, 100.0f); }
inline Serialization::RangeDecorator<i32>   SUBDIVISION_RANGE(i32& value) { return Serialization::Range(value, 3, 256); }
inline Serialization::RangeDecorator<float> STEPRISE_RANGE(float& value)  { return Serialization::Range(value, 0.1f, 10.0f); }
};

#define CRYDESIGNER_USER_DIRECTORY "DrxDesigner"
#define SERIALIZATION_ENUM_LABEL(value, label) \
  description.add(i32(value), label, label);

