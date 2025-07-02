
#include "../SimpleOpenGL2Renderer.h"
#include "../OpenGL2Include.h"
#include <drx3D/Common/b3Vec3.h>
#include <drx3D/Common/b3AlignedObjectArray.h>
#include "../GLInstanceGraphicsShape.h"
#include <drx3D/Common/b3Quat.h>
#include <drx3D/Common/b3Transform.h>
#include <drx3D/Common/b3ResizablePool.h>

D3_ATTRIBUTE_ALIGNED16(struct)
SimpleGL2Shape
{
	D3_DECLARE_ALIGNED_ALLOCATOR();

	i32 m_textureIndex;
	i32 m_primitiveType;
	b3AlignedObjectArray<i32> m_indices;
	b3AlignedObjectArray<GLInstanceVertex> m_vertices;
	b3Vec3 m_scaling;
};

D3_ATTRIBUTE_ALIGNED16(struct)
SimpleGL2Instance
{
	D3_DECLARE_ALIGNED_ALLOCATOR();

	i32 m_shapeIndex;
	b3Vec3 m_position;
	b3Quat orn;
	b3Vec4 m_rgbColor;
	b3Vec3 m_scaling;
	void clear()
	{
	}
};

struct InternalTextureHandle2
{
	GLuint m_glTexture;
	i32 m_width;
	i32 m_height;
};

typedef b3PoolBodyHandle<SimpleGL2Instance> SimpleGL2InstanceHandle;

struct SimpleOpenGL2RendererInternalData
{
	i32 m_width;
	i32 m_height;
	SimpleCamera m_camera;
	b3AlignedObjectArray<SimpleGL2Shape*> m_shapes;
	//b3AlignedObjectArray<SimpleGL2Instance> m_graphicsInstances1;

	b3ResizablePool<SimpleGL2InstanceHandle> m_graphicsInstancesPool;

	b3AlignedObjectArray<InternalTextureHandle2> m_textureHandles;
};

SimpleOpenGL2Renderer::SimpleOpenGL2Renderer(i32 width, i32 height)
{
	m_data = new SimpleOpenGL2RendererInternalData;
	m_data->m_width = width;
	m_data->m_height = height;
}

SimpleOpenGL2Renderer::~SimpleOpenGL2Renderer()
{
	delete m_data;
}

void SimpleOpenGL2Renderer::init()
{
}

const CommonCameraInterface* SimpleOpenGL2Renderer::getActiveCamera() const
{
	return &m_data->m_camera;
}
CommonCameraInterface* SimpleOpenGL2Renderer::getActiveCamera()
{
	return &m_data->m_camera;
}
void SimpleOpenGL2Renderer::setActiveCamera(CommonCameraInterface* cam)
{
	drx3DAssert(0);  //not supported yet
}

void SimpleOpenGL2Renderer::setLightPosition(const float lightPos[3])
{
}
void SimpleOpenGL2Renderer::setLightPosition(const double lightPos[3])
{
}

void SimpleOpenGL2Renderer::updateCamera(i32 upAxis)
{
	float projection[16];
	float view[16];
	getActiveCamera()->setAspectRatio((float)m_data->m_width / (float)m_data->m_height);
	getActiveCamera()->setCameraUpAxis(upAxis);
	m_data->m_camera.update();  //??
	getActiveCamera()->getCameraProjectionMatrix(projection);
	getActiveCamera()->getCameraViewMatrix(view);
	GLfloat projMat[16];
	GLfloat viewMat[16];
	for (i32 i = 0; i < 16; i++)
	{
		viewMat[i] = view[i];
		projMat[i] = projection[i];
	}
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(projMat);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(viewMat);
}

void SimpleOpenGL2Renderer::removeAllInstances()
{
	for (i32 i = 0; i < m_data->m_shapes.size(); i++)
	{
		delete m_data->m_shapes[i];
	}
	m_data->m_shapes.clear();
	m_data->m_graphicsInstancesPool.exitHandles();
	m_data->m_graphicsInstancesPool.initHandles();

	//also destroy textures?
	m_data->m_textureHandles.clear();
}

void SimpleOpenGL2Renderer::removeGraphicsInstance(i32 instanceUid)
{
	m_data->m_graphicsInstancesPool.freeHandle(instanceUid);
}

bool SimpleOpenGL2Renderer::readSingleInstanceTransformToCPU(float* position, float* orientation, i32 srcIndex)
{
	return false;
}

void SimpleOpenGL2Renderer::writeSingleInstanceColorToCPU(const float* color, i32 srcIndex)
{
}
void SimpleOpenGL2Renderer::writeSingleInstanceColorToCPU(const double* color, i32 srcIndex)
{
}

void SimpleOpenGL2Renderer::writeSingleInstanceScaleToCPU(const float* scale, i32 srcIndex)
{
}
void SimpleOpenGL2Renderer::writeSingleInstanceScaleToCPU(const double* scale, i32 srcIndex)
{
}

i32 SimpleOpenGL2Renderer::getTotalNumInstances() const
{
	return m_data->m_graphicsInstancesPool.getNumHandles();
}

void SimpleOpenGL2Renderer::getCameraViewMatrix(float viewMat[16]) const
{
	drx3DAssert(0);
}
void SimpleOpenGL2Renderer::getCameraProjectionMatrix(float projMat[16]) const
{
	drx3DAssert(0);
}

void SimpleOpenGL2Renderer::drawOpenGL(i32 instanceIndex)
{
	const SimpleGL2Instance* instPtr = m_data->m_graphicsInstancesPool.getHandle(instanceIndex);
	if (0 == instPtr)
	{
		drx3DAssert(0);
		return;
	}
	const SimpleGL2Instance& inst = *instPtr;
	const SimpleGL2Shape* shape = m_data->m_shapes[inst.m_shapeIndex];

	if (inst.m_rgbColor[3] == 0)
	{
		return;
	}

	glPushMatrix();
	b3Transform tr;
	tr.setOrigin(b3MakeVector3(inst.m_position[0], inst.m_position[1], inst.m_position[2]));
	tr.setRotation(b3Quat(inst.orn[0], inst.orn[1], inst.orn[2], inst.orn[3]));

	b3Scalar m[16];

	tr.getOpenGLMatrix(m);

#ifdef D3_USE_DOUBLE_PRECISION
	glMultMatrixd(m);
#else
	glMultMatrixf(m);
#endif

#if 0
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef(0.025f,0.025f,0.025f);
	glMatrixMode(GL_MODELVIEW);

	static const GLfloat	planex[]={1,0,0,0};
	//	static const GLfloat	planey[]={0,1,0,0};
		static const GLfloat	planez[]={0,0,1,0};
		glTexGenfv(GL_S,GL_OBJECT_PLANE,planex);
		glTexGenfv(GL_T,GL_OBJECT_PLANE,planez);
		glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
		glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
		m_textureinitialized=true;

#endif

	//drawCoordSystem();

	//glPushMatrix();
	//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	//	glMatrixMode(GL_TEXTURE);
	//	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_COLOR_MATERIAL);

	if (shape->m_textureIndex >= 0)
	{
		glEnable(GL_TEXTURE_2D);
		activateTexture(shape->m_textureIndex);
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
	}

	glColor3f(inst.m_rgbColor[0], inst.m_rgbColor[1], inst.m_rgbColor[2]);
	glScalef(inst.m_scaling[0], inst.m_scaling[1], inst.m_scaling[2]);
	glShadeModel(GL_SMOOTH);

	glBegin(GL_TRIANGLES);
	for (i32 i = 0; i < shape->m_indices.size(); i += 3)
	{
		for (i32 v = 0; v < 3; v++)
		{
			const GLInstanceVertex& vtx0 = shape->m_vertices[shape->m_indices[i + v]];
			glNormal3f(vtx0.normal[0], vtx0.normal[1], vtx0.normal[2]);
			glTexCoord2f(vtx0.uv[0], vtx0.uv[1]);
			glVertex3f(vtx0.xyzw[0], vtx0.xyzw[1], vtx0.xyzw[2]);
		}
	}
	glEnd();

	glPopMatrix();
}

void SimpleOpenGL2Renderer::drawSceneInternal(i32 pass, i32 cameraUpAxis)
{
	b3AlignedObjectArray<i32> usedHandles;
	m_data->m_graphicsInstancesPool.getUsedHandles(usedHandles);
	for (i32 i = 0; i < usedHandles.size(); i++)
	{
		drawOpenGL(usedHandles[i]);
	}

#if 0
	b3Scalar	m[16];
	b3Matrix3x3	rot;rot.setIdentity();
	i32k	numObjects=dynamicsWorld->getNumCollisionObjects();
	Vec3 wireColor(1,0,0);
	//glDisable(GL_CULL_FACE);
			
	for(i32 i=0;i<numObjects;i++)
	{
		const CollisionObject2*	colObj=dynamicsWorld->getCollisionObjectArray()[i];
		const RigidBody*		body=RigidBody::upcast(colObj);
		if(body&&body->getMotionState())
		{
			btDefaultMotionState* myMotionState = (DefaultMotionState*)body->getMotionState();
			myMotionState->m_graphicsWorldTrans.getOpenGLMatrix(m);
			rot=myMotionState->m_graphicsWorldTrans.getBasis();
		}
		else
		{
			colObj->getWorldTransform().getOpenGLMatrix(m);
			rot=colObj->getWorldTransform().getBasis();
		}
		Vec3 wireColor(1.f,1.0f,0.5f); //wants deactivation
		if(i&1) wireColor=Vec3(0.f,0.0f,1.f);
		///color differently for active, sleeping, wantsdeactivation states
		if (colObj->getActivationState() == 1) //active
		{
			if (i & 1)
			{
				wireColor += Vec3 (1.f,0.f,0.f);
			}
			else
			{
				wireColor += Vec3 (.5f,0.f,0.f);
			}
		}
		if(colObj->getActivationState()==2) //ISLAND_SLEEPING
		{
			if(i&1)
			{
				wireColor += Vec3 (0.f,1.f, 0.f);
			}
			else
			{
				wireColor += Vec3 (0.f,0.5f,0.f);
			}
		}

		Vec3 aabbMin(0,0,0),aabbMax(0,0,0);
		//m_dynamicsWorld->getBroadphase()->getBroadphaseAabb(aabbMin,aabbMax);

		aabbMin-=Vec3(DRX3D_LARGE_FLOAT,DRX3D_LARGE_FLOAT,DRX3D_LARGE_FLOAT);
		aabbMax+=Vec3(DRX3D_LARGE_FLOAT,DRX3D_LARGE_FLOAT,DRX3D_LARGE_FLOAT);
	//		printf("aabbMin=(%f,%f,%f)\n",aabbMin.getX(),aabbMin.getY(),aabbMin.getZ());
	//		printf("aabbMax=(%f,%f,%f)\n",aabbMax.getX(),aabbMax.getY(),aabbMax.getZ());
	//		m_dynamicsWorld->getDebugDrawer()->drawAabb(aabbMin,aabbMax,Vec3(1,1,1));

		//switch(pass)
		
		//if (!(getDebugMode()& btIDebugDraw::DBG_DrawWireframe))
		i32 debugMode = 0;//getDebugMode()
		//Vec3 m_sundirection(-1,-1,-1);
		
		Vec3 m_sundirection(Vec3(1,-2,1)*1000);
		if (cameraUpAxis==2)
		{
			m_sundirection = Vec3(1,1,-2)*1000;
		}
				
		switch(pass)
		{
			case	0:	drawOpenGL(m,colObj->getCollisionShape(),wireColor,debugMode,aabbMin,aabbMax);break;
			case	1:	drawShadow(m,m_sundirection*rot,colObj->getCollisionShape(),aabbMin,aabbMax);break;
			case	2:	drawOpenGL(m,colObj->getCollisionShape(),wireColor*b3Scalar(0.3),0,aabbMin,aabbMax);break;
		}
	}
#endif
}

void SimpleOpenGL2Renderer::renderScene()
{
	GLfloat light_ambient[] = {b3Scalar(0.2), b3Scalar(0.2), b3Scalar(0.2), b3Scalar(1.0)};
	GLfloat light_diffuse[] = {b3Scalar(1.0), b3Scalar(1.0), b3Scalar(1.0), b3Scalar(1.0)};
	GLfloat light_specular[] = {b3Scalar(1.0), b3Scalar(1.0), b3Scalar(1.0), b3Scalar(1.0)};
	/*	light_position is NOT default value	*/
	GLfloat light_position0[] = {b3Scalar(1.0), b3Scalar(10.0), b3Scalar(1.0), b3Scalar(0.0)};
	GLfloat light_position1[] = {b3Scalar(-1.0), b3Scalar(-10.0), b3Scalar(-1.0), b3Scalar(0.0)};

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);

	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	drawSceneInternal(0, 0);
}

i32 SimpleOpenGL2Renderer::registerTexture(u8k* texels, i32 width, i32 height, bool flipTexelsY)
{
	drx3DAssert(glGetError() == GL_NO_ERROR);
	glActiveTexture(GL_TEXTURE0);
	i32 textureIndex = m_data->m_textureHandles.size();
	//  const GLubyte*	image= (const GLubyte*)texels;
	GLuint textureHandle;
	glGenTextures(1, (GLuint*)&textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);

	drx3DAssert(glGetError() == GL_NO_ERROR);

	InternalTextureHandle2 h;
	h.m_glTexture = textureHandle;
	h.m_width = width;
	h.m_height = height;

	m_data->m_textureHandles.push_back(h);
	updateTexture(textureIndex, texels, flipTexelsY);
	return textureIndex;
}

void SimpleOpenGL2Renderer::updateTexture(i32 textureIndex, u8k* texels, bool flipTexelsY)
{
	if (textureIndex >= 0)
	{
		glActiveTexture(GL_TEXTURE0);
		drx3DAssert(glGetError() == GL_NO_ERROR);
		InternalTextureHandle2& h = m_data->m_textureHandles[textureIndex];
		glBindTexture(GL_TEXTURE_2D, h.m_glTexture);
		drx3DAssert(glGetError() == GL_NO_ERROR);

		if (flipTexelsY)
		{
			//textures need to be flipped for OpenGL...
			b3AlignedObjectArray<u8> flippedTexels;
			flippedTexels.resize(h.m_width * h.m_height * 3);
			for (i32 i = 0; i < h.m_width; i++)
			{
				for (i32 j = 0; j < h.m_height; j++)
				{
					flippedTexels[(i + j * h.m_width) * 3] = texels[(i + (h.m_height - 1 - j) * h.m_width) * 3];
					flippedTexels[(i + j * h.m_width) * 3 + 1] = texels[(i + (h.m_height - 1 - j) * h.m_width) * 3 + 1];
					flippedTexels[(i + j * h.m_width) * 3 + 2] = texels[(i + (h.m_height - 1 - j) * h.m_width) * 3 + 2];
				}
			}
			//  const GLubyte*	image= (const GLubyte*)texels;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, h.m_width, h.m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, &flippedTexels[0]);
		}
		else
		{
			//  const GLubyte*	image= (const GLubyte*)texels;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, h.m_width, h.m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, &texels[0]);
		}

		drx3DAssert(glGetError() == GL_NO_ERROR);
		//glGenerateMipmap(GL_TEXTURE_2D);
		drx3DAssert(glGetError() == GL_NO_ERROR);
	}
}

void SimpleOpenGL2Renderer::removeTexture(i32 textureIndex)
{
	if ((textureIndex >= 0) && (textureIndex < m_data->m_textureHandles.size()))
	{
		glDeleteTextures(1, &m_data->m_textureHandles[textureIndex].m_glTexture);
	}
}
void SimpleOpenGL2Renderer::activateTexture(i32 textureIndex)
{
	glActiveTexture(GL_TEXTURE0);

	if (textureIndex >= 0)
	{
		glBindTexture(GL_TEXTURE_2D, m_data->m_textureHandles[textureIndex].m_glTexture);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

i32 SimpleOpenGL2Renderer::registerGraphicsInstance(i32 shapeIndex, const double* position, const double* quaternion, const double* color, const double* scaling)
{
	i32 newHandle = m_data->m_graphicsInstancesPool.allocHandle();

	//	i32 sz = m_data->m_graphicsInstances.size();

	SimpleGL2Instance& instance = *m_data->m_graphicsInstancesPool.getHandle(newHandle);
	instance.m_shapeIndex = shapeIndex;
	instance.m_position[0] = position[0];
	instance.m_position[1] = position[1];
	instance.m_position[2] = position[2];
	instance.orn[0] = quaternion[0];
	instance.orn[1] = quaternion[1];
	instance.orn[2] = quaternion[2];
	instance.orn[3] = quaternion[3];
	instance.m_rgbColor[0] = color[0];
	instance.m_rgbColor[1] = color[1];
	instance.m_rgbColor[2] = color[2];
	instance.m_rgbColor[3] = color[3];

	instance.m_scaling[0] = scaling[0];
	instance.m_scaling[1] = scaling[1];
	instance.m_scaling[2] = scaling[2];
	return newHandle;
}

i32 SimpleOpenGL2Renderer::registerGraphicsInstance(i32 shapeIndex, const float* position, const float* quaternion, const float* color, const float* scaling)
{
	i32 newHandle = m_data->m_graphicsInstancesPool.allocHandle();
	SimpleGL2Instance& instance = *m_data->m_graphicsInstancesPool.getHandle(newHandle);
	instance.m_shapeIndex = shapeIndex;
	instance.m_position[0] = position[0];
	instance.m_position[1] = position[1];
	instance.m_position[2] = position[2];
	instance.orn[0] = quaternion[0];
	instance.orn[1] = quaternion[1];
	instance.orn[2] = quaternion[2];
	instance.orn[3] = quaternion[3];
	instance.m_rgbColor[0] = color[0];
	instance.m_rgbColor[1] = color[1];
	instance.m_rgbColor[2] = color[2];
	instance.m_rgbColor[3] = color[3];

	instance.m_scaling[0] = scaling[0];
	instance.m_scaling[1] = scaling[1];
	instance.m_scaling[2] = scaling[2];
	return newHandle;
}

void SimpleOpenGL2Renderer::drawLines(const float* positions, const float color[4], i32 numPoints, i32 pointStrideInBytes, u32k* indices, i32 numIndices, float pointDrawSize)
{
	i32 pointStrideInFloats = pointStrideInBytes / 4;
	glLineWidth(pointDrawSize);
	for (i32 i = 0; i < numIndices; i += 2)
	{
		i32 index0 = indices[i];
		i32 index1 = indices[i + 1];

		b3Vec3 fromColor = b3MakeVector3(color[0], color[1], color[2]);
		b3Vec3 toColor = b3MakeVector3(color[0], color[1], color[2]);

		b3Vec3 from = b3MakeVector3(positions[index0 * pointStrideInFloats], positions[index0 * pointStrideInFloats + 1], positions[index0 * pointStrideInFloats + 2]);
		b3Vec3 to = b3MakeVector3(positions[index1 * pointStrideInFloats], positions[index1 * pointStrideInFloats + 1], positions[index1 * pointStrideInFloats + 2]);

		glBegin(GL_LINES);
		glColor3f(fromColor.getX(), fromColor.getY(), fromColor.getZ());
		glVertex3d(from.getX(), from.getY(), from.getZ());
		glColor3f(toColor.getX(), toColor.getY(), toColor.getZ());
		glVertex3d(to.getX(), to.getY(), to.getZ());
		glEnd();
	}
}

void SimpleOpenGL2Renderer::drawLine(const float from[4], const float to[4], const float color[4], float lineWidth)
{
	glLineWidth(lineWidth);
	glBegin(GL_LINES);
	glColor3f(color[0], color[1], color[2]);
	glVertex3d(from[0], from[1], from[2]);
	glVertex3d(to[0], to[1], to[2]);
	glEnd();
}

i32 SimpleOpenGL2Renderer::registerShape(const float* vertices, i32 numvertices, i32k* indices, i32 numIndices, i32 primitiveType, i32 textureIndex)
{
	SimpleGL2Shape* shape = new SimpleGL2Shape();
	shape->m_textureIndex = textureIndex;
	shape->m_indices.resize(numIndices);

	for (i32 i = 0; i < numIndices; i++)
	{
		shape->m_indices[i] = indices[i];
	}

	shape->m_vertices.resize(numvertices);

	for (i32 v = 0; v < numvertices; v++)
	{
		GLInstanceVertex& vtx = shape->m_vertices[v];
		vtx.xyzw[0] = vertices[9 * v + 0];
		vtx.xyzw[1] = vertices[9 * v + 1];
		vtx.xyzw[2] = vertices[9 * v + 2];
		vtx.xyzw[3] = vertices[9 * v + 3];
		vtx.normal[0] = vertices[9 * v + 4];
		vtx.normal[1] = vertices[9 * v + 5];
		vtx.normal[2] = vertices[9 * v + 6];
		vtx.uv[0] = vertices[9 * v + 7];
		vtx.uv[1] = vertices[9 * v + 8];
	}
	i32 sz = m_data->m_shapes.size();
	m_data->m_shapes.push_back(shape);
	return sz;
}

void SimpleOpenGL2Renderer::writeSingleInstanceTransformToCPU(const float* position, const float* orientation, i32 srcIndex)
{
	SimpleGL2Instance& graphicsInstance = *m_data->m_graphicsInstancesPool.getHandle(srcIndex);

	graphicsInstance.m_position[0] = position[0];
	graphicsInstance.m_position[1] = position[1];
	graphicsInstance.m_position[2] = position[2];

	graphicsInstance.orn[0] = orientation[0];
	graphicsInstance.orn[1] = orientation[1];
	graphicsInstance.orn[2] = orientation[2];
	graphicsInstance.orn[3] = orientation[3];
}
void SimpleOpenGL2Renderer::writeSingleInstanceTransformToCPU(const double* position, const double* orientation, i32 srcIndex)
{
	SimpleGL2Instance& graphicsInstance = *m_data->m_graphicsInstancesPool.getHandle(srcIndex);

	graphicsInstance.m_position[0] = position[0];
	graphicsInstance.m_position[1] = position[1];
	graphicsInstance.m_position[2] = position[2];

	graphicsInstance.orn[0] = orientation[0];
	graphicsInstance.orn[1] = orientation[1];
	graphicsInstance.orn[2] = orientation[2];
	graphicsInstance.orn[3] = orientation[3];
}
void SimpleOpenGL2Renderer::writeTransforms()
{
}

void SimpleOpenGL2Renderer::resize(i32 width, i32 height)
{
	m_data->m_width = width;
	m_data->m_height = height;
}

i32 SimpleOpenGL2Renderer::getScreenWidth()
{
	return m_data->m_width;
}
i32 SimpleOpenGL2Renderer::getScreenHeight()
{
	return m_data->m_height;
}

void SimpleOpenGL2Renderer::drawLine(const double from[4], const double to[4], const double color[4], double lineWidth)
{
	glLineWidth(lineWidth);
	glBegin(GL_LINES);
	glColor3f(color[0], color[1], color[2]);
	glVertex3d(from[0], from[1], from[2]);
	glVertex3d(to[0], to[1], to[2]);
	glEnd();
}
void SimpleOpenGL2Renderer::drawPoint(const float* position, const float color[4], float pointDrawSize)
{
}
void SimpleOpenGL2Renderer::drawPoint(const double* position, const double color[4], double pointDrawSize)
{
}
void SimpleOpenGL2Renderer::drawPoints(const float* positions, const float* colors, i32 numPoints, i32 pointStrideInBytes, float pointDrawSize)
{
}

void SimpleOpenGL2Renderer::updateShape(i32 shapeIndex, const float* vertices, i32 numVertices)
{
	SimpleGL2Shape* shape = m_data->m_shapes[shapeIndex];
	i32 numvertices = shape->m_vertices.size();
	drx3DAssert(numVertices = numvertices);
	if (numVertices != numvertices)
		return;

	for (i32 i = 0; i < numvertices; i++)
	{
		shape->m_vertices[i].xyzw[0] = vertices[9 * i + 0];
		shape->m_vertices[i].xyzw[1] = vertices[9 * i + 1];
		shape->m_vertices[i].xyzw[2] = vertices[9 * i + 2];
		shape->m_vertices[i].xyzw[3] = vertices[9 * i + 3];

		shape->m_vertices[i].normal[0] = vertices[9 * i + 4];
		shape->m_vertices[i].normal[1] = vertices[9 * i + 5];
		shape->m_vertices[i].normal[2] = vertices[9 * i + 6];

		shape->m_vertices[i].uv[0] = vertices[9 * i + 7];
		shape->m_vertices[i].uv[1] = vertices[9 * i + 8];
	}
}

void SimpleOpenGL2Renderer::clearZBuffer()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}