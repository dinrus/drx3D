#ifndef D3_DRX3D_FILE_H
#define D3_DRX3D_FILE_H

#include <drx3D/Serialize/Bullet2FileLoader/b3File.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include <drx3D/Serialize/Bullet2FileLoader/b3Defines.h>

#include <drx3D/Serialize/Bullet2FileLoader/b3Serializer.h>

namespace bParse
{
// ----------------------------------------------------- //
class b3BulletFile : public bFile
{
protected:
	tuk m_DnaCopy;

public:
	b3AlignedObjectArray<bStructHandle*> m_softBodies;

	b3AlignedObjectArray<bStructHandle*> m_rigidBodies;

	b3AlignedObjectArray<bStructHandle*> m_collisionObjects;

	b3AlignedObjectArray<bStructHandle*> m_collisionShapes;

	b3AlignedObjectArray<bStructHandle*> m_constraints;

	b3AlignedObjectArray<bStructHandle*> m_bvhs;

	b3AlignedObjectArray<bStructHandle*> m_triangleInfoMaps;

	b3AlignedObjectArray<bStructHandle*> m_dynamicsWorldInfo;

	b3AlignedObjectArray<tuk> m_dataBlocks;
	b3BulletFile();

	b3BulletFile(tukk fileName);

	b3BulletFile(tuk memoryBuffer, i32 len);

	virtual ~b3BulletFile();

	virtual void addDataBlock(tuk dataBlock);

	// experimental
	virtual i32 write(tukk fileName, bool fixupPointers = false);

	virtual void parse(i32 verboseMode);

	virtual void parseData();

	virtual void writeDNA(FILE* fp);

	void addStruct(tukk structType, uk data, i32 len, uk oldPtr, i32 code);
};
};  // namespace bParse

#endif  //D3_DRX3D_FILE_H
