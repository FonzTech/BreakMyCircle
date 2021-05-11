#include "Bubble.h"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

using namespace Magnum;
using namespace Magnum::Math::Literals;

Bubble::Bubble(SceneGraph::DrawableGroup3D& group) : GameObject(group)
{
	// Create test mesh
	meshData = std::make_shared<Trade::MeshData>(Primitives::icosphereSolid(2));

	GL::Buffer vertices;
	vertices.setData(MeshTools::interleave(meshData->positions3DAsArray(), meshData->normalsAsArray()));

	std::pair<Containers::Array<char>, MeshIndexType> compressed = MeshTools::compressIndices(meshData->indicesAsArray());
	GL::Buffer indices;
	indices.setData(compressed.first);

	mMesh.setPrimitive(meshData->primitive())
		.setCount(meshData->indexCount())
		.addVertexBuffer(std::move(vertices), 0, Shaders::Phong::Position{}, Shaders::Phong::Normal{})
		.setIndexBuffer(std::move(indices), 0, compressed.second);

	// Set diffuse color
	mDiffuseColor = 0xffffff_rgbf;
	mAmbientColor = 0xff0000_rgbf;
}

void Bubble::update()
{
}

void Bubble::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	mShader.setLightPositions({ position + Vector3({ 10.0f, 10.0f, 1.75f }) })
		.setDiffuseColor(mDiffuseColor)
		.setAmbientColor(mAmbientColor)
		.setTransformationMatrix(transformationMatrix * Matrix4::translation(position))
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.draw(mMesh);
}