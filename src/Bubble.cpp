#include "Bubble.h"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "ColoredDrawable.h"
#include "RoomManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

Bubble::Bubble(const Color3& ambientColor) : GameObject()
{
	// Assign color
	mAmbientColor = ambientColor;

	// Create test mesh
	Trade::MeshData mMeshData = Primitives::icosphereSolid(2);

	GL::Buffer vertices;
	vertices.setData(MeshTools::interleave(mMeshData.positions3DAsArray(), mMeshData.normalsAsArray()));

	std::pair<Containers::Array<char>, MeshIndexType> compressed = MeshTools::compressIndices(mMeshData.indicesAsArray());
	GL::Buffer indices;
	indices.setData(compressed.first);

	GL::Mesh mesh;
	mesh
		.setPrimitive(mMeshData.primitive())
		.setCount(mMeshData.indexCount())
		.addVertexBuffer(std::move(vertices), 0, Shaders::Phong::Position{}, Shaders::Phong::Normal{})
		.setIndexBuffer(std::move(indices), 0, compressed.second);

	// Set diffuse color
	mDiffuseColor = 0xffffff_rgbf;

	// Create Phong shader
	Shaders::Phong shader;
	
	// Create colored drawable
	mColoredDrawable = std::make_shared<ColoredDrawable>(RoomManager::singleton->mDrawables, shader, mesh, mAmbientColor);
	mColoredDrawable->setParent(&RoomManager::singleton->mScene);
	mColoredDrawable->setDrawCallback(this);
	drawables.emplace_back(mColoredDrawable);
}

void Bubble::update()
{
}

void Bubble::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	mColoredDrawable->mShader
		.setLightPositions({ position + Vector3({ 10.0f, 10.0f, 1.75f }) })
		.setDiffuseColor(mDiffuseColor)
		.setAmbientColor(mAmbientColor)
		.setTransformationMatrix(transformationMatrix * Matrix4::translation(position))
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.draw(mColoredDrawable->mMesh);
}

void Bubble::collidedWith(GameObject* gameObject)
{
}

void Bubble::updateBBox()
{
	bbox = Range3D{ position - Vector3(0.8f), position + Vector3(0.8f) };
}

void Bubble::destroyNearbyBubbles()
{
	// Destroy me first
	printf("Not implemented yet\n");
}