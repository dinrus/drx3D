#ifndef CONCAVE_SCENE_H
#define CONCAVE_SCENE_H

#include "GpuRigidBodyDemo.h"
#include <drx3D/Common/b3Vec3.h"

class ConcaveScene : public GpuRigidBodyDemo
{
public:
	ConcaveScene() {}
	virtual ~ConcaveScene() {}
	virtual tukk getName()
	{
		return "BoxTrimesh";
	}

	static GpuDemo* MyCreateFunc()
	{
		GpuDemo* demo = new ConcaveScene;
		return demo;
	}

	virtual void setupScene(const ConstructionInfo& ci);

	virtual void createDynamicObjects(const ConstructionInfo& ci);

	virtual void createConcaveMesh(const ConstructionInfo& ci, tukk fileName, const b3Vec3& shift, const b3Vec3& scaling);
};

class ConcaveSphereScene : public ConcaveScene
{
public:
	ConcaveSphereScene() {}
	virtual ~ConcaveSphereScene() {}
	virtual tukk getName()
	{
		return "SphereTrimesh";
	}

	static GpuDemo* MyCreateFunc()
	{
		GpuDemo* demo = new ConcaveSphereScene;
		return demo;
	}

	virtual void setupScene(const ConstructionInfo& ci);

	virtual void createDynamicObjects(const ConstructionInfo& ci);
};

class ConcaveCompoundScene : public ConcaveScene
{
public:
	ConcaveCompoundScene() {}
	virtual ~ConcaveCompoundScene() {}
	virtual tukk getName()
	{
		return "CompoundConcave";
	}

	static GpuDemo* MyCreateFunc()
	{
		GpuDemo* demo = new ConcaveCompoundScene;
		return demo;
	}

	virtual void setupScene(const ConstructionInfo& ci);

	virtual void createDynamicObjects(const ConstructionInfo& ci);
};

class ConcaveCompound2Scene : public ConcaveCompoundScene
{
public:
	ConcaveCompound2Scene() {}
	virtual ~ConcaveCompound2Scene() {}
	virtual tukk getName()
	{
		return "GRBConcave2Compound";
	}

	static GpuDemo* MyCreateFunc()
	{
		GpuDemo* demo = new ConcaveCompound2Scene;
		return demo;
	}
	virtual void createDynamicObjects(const ConstructionInfo& ci);
};

#endif  //CONCAVE_SCENE_H
