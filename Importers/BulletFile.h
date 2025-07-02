#ifndef DRX3D_DRX3D_FILE_H
#define DRX3D_DRX3D_FILE_H

#include "bFile.h"
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include "bDefines.h"

#include <drx3D/Maths/Linear/Serializer.h>

namespace bParse
{
// ----------------------------------------------------- //
class BulletFile : public bFile
{
protected:
	tuk m_DnaCopy;

public:
	AlignedObjectArray<bStructHandle*> m_multiBodies;

	AlignedObjectArray<bStructHandle*> m_multiBodyLinkColliders;

	AlignedObjectArray<bStructHandle*> m_softBodies;

	AlignedObjectArray<bStructHandle*> m_rigidBodies;

	AlignedObjectArray<bStructHandle*> m_collisionObjects;

	AlignedObjectArray<bStructHandle*> m_collisionShapes;

	AlignedObjectArray<bStructHandle*> m_constraints;

	AlignedObjectArray<bStructHandle*> m_bvhs;

	AlignedObjectArray<bStructHandle*> m_triangleInfoMaps;

	AlignedObjectArray<bStructHandle*> m_dynamicsWorldInfo;

	AlignedObjectArray<bStructHandle*> m_contactManifolds;

	AlignedObjectArray<tuk> m_dataBlocks;
	BulletFile();

	BulletFile(tukk fileName);

	BulletFile(tuk memoryBuffer, i32 len);

	virtual ~BulletFile();

	virtual void addDataBlock(tuk dataBlock);

	// experimental
	virtual i32 write(tukk fileName, bool fixupPointers = false);

	virtual void parse(i32 verboseMode);

	virtual void parseData();

	virtual void writeDNA(FILE* fp);

	void addStruct(tukk structType, uk data, i32 len, uk oldPtr, i32 code);
};
};  // namespace bParse

#endif  //DRX3D_DRX3D_FILE_H
