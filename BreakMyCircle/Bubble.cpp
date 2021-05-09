#include "Bubble.h"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

using namespace Magnum;

Bubble::Bubble() : GameObject()
{
	// Create test mesh
	meshData = std::make_shared<Trade::MeshData>(Primitives::icosphereSolid(1));

	GL::Buffer vertices;
	vertices.setData(MeshTools::interleave(meshData->positions3DAsArray(), meshData->normalsAsArray()));

	std::pair<Containers::Array<char>, MeshIndexType> compressed = MeshTools::compressIndices(meshData->indicesAsArray());
	GL::Buffer indices;
	indices.setData(compressed.first);

	mMesh.setPrimitive(meshData->primitive())
		.setCount(meshData->indexCount())
		.addVertexBuffer(std::move(vertices), 0, Shaders::Phong::Position{}, Shaders::Phong::Normal{})
		.setIndexBuffer(std::move(indices), 0, compressed.second);
}

void Bubble::update()
{
}

void Bubble::draw()
{
	mShader.setLightPosition({ {1.4f, 1.0f, 0.75f} })
		.setDiffuseColor(mColor)
		.setAmbientColor({ 0.0f, 0.0f, 0.0f, 1.0f })
		.setTransformationMatrix(mTransformation)
		.setNormalMatrix(mTransformation.normalMatrix())
		.setProjectionMatrix(mProjection)
		.draw(mMesh);
}