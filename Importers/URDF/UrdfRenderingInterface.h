#ifndef URDF_RENDERING_INTERFACE_H
#define URDF_RENDERING_INTERFACE_H

#include <drxtypes.h>

///UrdfLink and UrdfModel are the main URDF structures to define a robot
struct UrdfLink;
struct UrdfModel;
///Transform2 is a position and 3x3 matrix, as defined in drx3D/src/LinearMath/Transform2
class Transform2;
class Vec3;

///UrdfRenderingInterface is a simple rendering interface, mainly for URDF-based robots.
///There is an implementation in
///bullet3\examples\SharedMemory\plugins\tinyRendererPlugin\TinyRendererVisualShapeConverter.cpp
struct UrdfRenderingInterface
{
	virtual ~UrdfRenderingInterface() {}
	///given a URDF link, convert all visual shapes into internal renderer (loading graphics
	///meshes, textures etc)
	///use the visualShapeUniqueId as a unique identifier to synchronize the world transform
	///and to remove the visual shape.
	virtual i32 convertVisualShapes(i32 linkIndex, tukk pathPrefix,
	                         const Transform2& localInertiaFrame, const UrdfLink* linkPtr,
	                          const UrdfModel* model, i32 visualShapeUniqueId, i32 bodyUniqueId,
	                                                 struct CommonFileIOInterface* fileIO) = 0;

	virtual i32 registerShapeAndInstance(const struct b3VisualShapeData& visualShape,
	                  const float* vertices, i32 numvertices, i32k* indices, i32 numIndices,
	                   i32 primitiveType, i32 textureId, i32 orgGraphicsUniqueId, i32 bodyUniqueId,
	                                                                           i32 linkIndex)=0;

	virtual void updateShape(i32 shapeUniqueId, const Vec3* vertices, i32 numVertices,
	                                                   const Vec3* normals, i32 numNormals) = 0;

	///remove a visual shapes, based on the shape unique id (shapeUid)
	virtual void removeVisualShape(i32 collisionObjectUid) = 0;

	///update the world transform + scaling of the visual shape, using the shapeUid
	virtual void syncTransform(i32 collisionObjectUid, const class Transform2& worldTransform,
	                                                       const class Vec3& localScaling) = 0;

	///return the number of visual shapes, for a particular body unique id
	virtual i32 getNumVisualShapes(i32 bodyUniqueId) = 0;

	///return the visual shape information, for a particular body unique id and 'shape index'
	virtual i32 getVisualShapesData(i32 bodyUniqueId, i32 shapeIndex,
	                                                  struct b3VisualShapeData* shapeData) = 0;

	///change the RGBA color for some visual shape.
	virtual void changeRGBAColor(i32 bodyUniqueId, i32 linkIndex, i32 shapeIndex,
	                                                            const double rgbaColor[4]) = 0;

	//change the instance flags, double-sided rendering
	virtual void changeInstanceFlags(i32 bodyUniqueId, i32 linkIndex, i32 shapeIndex,
	                                                                            i32 flags) = 0;

	///select a given texture for some visual shape.
	virtual void changeShapeTexture(i32 objectUniqueId, i32 linkIndex, i32 shapeIndex,
	                                                                  i32 textureUniqueId) = 0;

	///pick the up-axis, either Y (1) or Z (2)
	virtual void setUpAxis(i32 axis) = 0;

	///compute the view matrix based on those parameters
	virtual void resetCamera(float camDist, float yaw, float pitch, float camPosX,
	                                                         float camPosY, float camPosZ) = 0;

	///clear the render buffer with a particular color.
	virtual void clearBuffers(struct TGAColor& clearColor) = 0;

	///remove all visual shapes.
	virtual void resetAll() = 0;

	///return the frame buffer width and height for the renderer
	virtual void getWidthAndHeight(i32& width, i32& height) = 0;

	///set the frame buffer width and height for the renderer
	virtual void setWidthAndHeight(i32 width, i32 height) = 0;

	///set the light direction, in world coordinates
	virtual void setLightDirection(float x, float y, float z) = 0;

	///set the ambient light color, in world coordinates
	virtual void setLightColor(float x, float y, float z) = 0;

	///set the light distance
	virtual void setLightDistance(float dist) = 0;

	///set the light ambient coefficient
	virtual void setLightAmbientCoeff(float ambientCoeff) = 0;

	///set the light diffuse coefficient
	virtual void setLightDiffuseCoeff(float diffuseCoeff) = 0;

	///set the light specular coefficient
	virtual void setLightSpecularCoeff(float specularCoeff) = 0;

	///enable or disable rendering of shadows
	virtual void setShadow(bool hasShadow) = 0;

	///some undocumented flags for experimentation (todo: document)
	virtual void setFlags(i32 flags) = 0;

	///provide the image pixels as a part of a stream.
	virtual void copyCameraImageData(u8* pixelsRGBA, i32 rgbaBufferSizeInPixels,
	              float* depthBuffer, i32 depthBufferSizeInPixels, i32* segmentationMaskBuffer,
	               i32 segmentationMaskSizeInPixels, i32 startPixelIndex, i32* widthPtr,
	                                                 i32* heightPtr, i32* numPixelsCopied) = 0;

	///render an image, using some arbitraty view and projection matrix
	virtual void render() = 0;

	///render an image using the provided view and projection matrix
	virtual void render(const float viewMat[16], const float projMat[16]) = 0;

	///load a texture from file, in png or other popular/supported format
	//virtual i32 loadTextureFile(tukk filename) = 0;
	virtual i32 loadTextureFile(tukk filename, struct CommonFileIOInterface* fileIO)=0;
	
	///register a texture using an in-memory pixel buffer of a given width and height
	virtual i32 registerTexture(u8* texels, i32 width, i32 height) = 0;

	virtual void setProjectiveTextureMatrices(const float viewMatrix[16],
	                                                      const float projectionMatrix[16]) {}
	virtual void setProjectiveTexture(bool useProjectiveTexture) {}


	virtual bool getCameraInfo(i32* width, i32* height, float viewMatrix[16],
	              float projectionMatrix[16], float camUp[3], float camForward[3], float hor[3],
	                                  float vert[3], float* yaw, float* pitch, float* camDist,
	                                                              float cameraTarget[3]) const
	{
		return false;
	}

};

#endif  //LINK_VISUAL_SHAPES_CONVERTER_H
