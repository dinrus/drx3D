#ifndef BSP_LOADER_H
#define BSP_LOADER_H

#include <drx3D/Maths/Linear/AlignedObjectArray.h>

#define BSPMAXTOKEN 1024
#define BSPMAX_KEY 32
#define BSPMAX_VALUE 1024
#define BSPCONTENTS_SOLID 1
#define BSPCONTENTS_AREAPORTAL 0x8000
#define BSPLUMP_ENTITIES 0
#define BSPLUMP_SHADERS 1
#define BSPLUMP_PLANES 2
#define BSPLUMP_NODES 3
#define BSPLUMP_LEAFS 4
#define BSPLUMP_LEAFSURFACES 5
#define BSPLUMP_LEAFBRUSHES 6
#define LUMP_MODELS 7
#define LUMP_BRUSHES 8
#define LUMP_BRUSHSIDES 9
#define LUMP_DRAWVERTS 10
#define LUMP_DRAWINDEXES 11
#define LUMP_SURFACES 13
#define LUMP_LIGHTMAPS 14
#define LUMP_LIGHTGRID 15
#define LUMP_VISIBILITY 16
#define HEADER_LUMPS 17
#define MAX_QPATH 64

typedef struct
{
	i32 fileofs, filelen;
} BSPLump;

typedef float BSPVector3[3];

typedef struct
{
	i32 ident;
	i32 version;

	BSPLump lumps[HEADER_LUMPS];
} BSPHeader;

typedef struct
{
	float mins[3], maxs[3];
	i32 firstSurface, numSurfaces;
	i32 firstBrush, numBrushes;
} BSPModel;

typedef struct
{
	char shader[MAX_QPATH];
	i32 surfaceFlags;
	i32 contentFlags;
} BSPShader;

typedef struct
{
	float normal[3];
	float dist;
} BSPPlane;

typedef struct
{
	i32 planeNum;
	i32 children[2];
	i32 mins[3];
	i32 maxs[3];
} BSPNode;

typedef struct
{
	i32 cluster;
	i32 area;

	i32 mins[3];
	i32 maxs[3];

	i32 firstLeafSurface;
	i32 numLeafSurfaces;

	i32 firstLeafBrush;
	i32 numLeafBrushes;
} BSPLeaf;

typedef struct
{
	i32 planeNum;
	i32 shaderNum;
} BSPBrushSide;

typedef struct
{
	i32 firstSide;
	i32 numSides;
	i32 shaderNum;
} BSPBrush;

typedef struct BSPPair
{
	struct BSPPair *next;
	char *key;
	char *value;
} BSPKeyValuePair;

typedef struct
{
	BSPVector3 origin;
	struct bspbrush_s *brushes;
	struct parseMesh_s *patches;
	i32 firstDrawSurf;
	BSPKeyValuePair *epairs;
} BSPEntity;

typedef enum
{
	MST_BAD,
	MST_PLANAR,
	MST_PATCH,
	MST_TRIANGLE_SOUP,
	MST_FLARE
} BSPMapSurface;

typedef struct
{
	i32 shaderNum;
	i32 fogNum;
	i32 surfaceType;

	i32 firstVert;
	i32 numVerts;

	i32 firstIndex;
	i32 numIndexes;

	i32 lightmapNum;
	i32 lightmapX, lightmapY;
	i32 lightmapWidth, lightmapHeight;

	BSPVector3 lightmapOrigin;
	BSPVector3 lightmapVecs[3];

	i32 patchWidth;
	i32 patchHeight;
} BSPSurface;

///GPL code from IdSofware to parse a Quake 3 BSP file
///check that your platform define __BIG_ENDIAN__ correctly (in BspLoader.cpp)
class BspLoader
{
	i32 m_Endianness;

public:
	BspLoader();

	bool loadBSPFile(uk memoryBuffer);

	tukk getValueForKey(const BSPEntity *ent, tukk key) const;

	bool getVectorForKey(const BSPEntity *ent, tukk key, BSPVector3 vec);

	float getFloatForKey(const BSPEntity *ent, tukk key);

	void parseEntities(void);

	bool findVectorByName(float *outvec, tukk name);

	const BSPEntity *getEntityByValue(tukk name, tukk value);

protected:
	void parseFromMemory(char *buffer, i32 size);

	bool isEndOfScript(bool crossline);

	bool getToken(bool crossline);

	char *copystring(tukk s);

	void stripTrailing(char *e);

	BSPKeyValuePair *parseEpair(void);

	bool parseEntity(void);

	short isLittleShort(short l);
	i32 isLittleLong(i32 l);
	float isLittleFloat(float l);

	i32 isBigLong(i32 l);
	short isBigShort(short l);
	float isBigFloat(float l);

	void swapBlock(i32 *block, i32 sizeOfBlock);

	i32 copyLump(BSPHeader *header, i32 lump, uk dest, i32 size);

	void swapBSPFile(void);

public:  //easier for conversion
	i32 m_num_entities;
	AlignedObjectArray<BSPEntity> m_entities;

	i32 m_nummodels;
	AlignedObjectArray<BSPModel> m_dmodels;

	i32 m_numShaders;
	AlignedObjectArray<BSPShader> m_dshaders;

	i32 m_entdatasize;
	AlignedObjectArray<char> m_dentdata;

	i32 m_numleafs;
	AlignedObjectArray<BSPLeaf> m_dleafs;

	i32 m_numplanes;
	AlignedObjectArray<BSPPlane> m_dplanes;

	i32 m_numnodes;
	AlignedObjectArray<BSPNode> m_dnodes;

	i32 m_numleafsurfaces;
	AlignedObjectArray<i32> m_dleafsurfaces;

	i32 m_numleafbrushes;
	AlignedObjectArray<i32> m_dleafbrushes;

	i32 m_numbrushes;
	AlignedObjectArray<BSPBrush> m_dbrushes;

	i32 m_numbrushsides;
	AlignedObjectArray<BSPBrushSide> m_dbrushsides;

	i32 m_numLightBytes;
	AlignedObjectArray<u8> m_lightBytes;

	i32 m_numGridPoints;
	AlignedObjectArray<u8> m_gridData;

	i32 m_numVisBytes;
	AlignedObjectArray<u8> m_visBytes;

	i32 m_numDrawIndexes;
	AlignedObjectArray<i32> m_drawIndexes;

	i32 m_numDrawSurfaces;
	AlignedObjectArray<BSPSurface> m_drawSurfaces;

	enum
	{
		BSP_LITTLE_ENDIAN = 0,
		BSP_BIG_ENDIAN = 1
	};

	//returns machines big endian / little endian
	//
	i32 getMachineEndianness();

	inline i32 machineEndianness()
	{
		return m_Endianness;
	}
};

#endif  //BSP_LOADER_H
