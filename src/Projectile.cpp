#include "Projectile.h"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "ColoredDrawable.h"
#include "RoomManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

Projectile::Projectile(Color3& ambientColor) : GameObject()
{
	// Initialize members
	mAmbientColor = ambientColor;
	velocity = { 0.0f };

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
	cd = std::make_shared<ColoredDrawable>(RoomManager::singleton->mDrawables, shader, mesh, mAmbientColor);
	cd->setParent(&RoomManager::singleton->mScene);
	cd->setDrawCallback(this);
	drawables.emplace_back(cd);
}

void Projectile::update()
{
	// Affect position by velocity
	position += velocity * deltaTime * 10.0f;
}

void Projectile::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	cd->mShader
		.setLightPositions({ position + Vector3({ 10.0f, 20.0f, 1.75f }) })
		.setDiffuseColor(mDiffuseColor)
		.setAmbientColor(mAmbientColor)
		.setTransformationMatrix(transformationMatrix * Matrix4::translation(position))
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.draw(cd->mMesh);
}