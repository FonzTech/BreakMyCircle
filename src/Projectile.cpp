#include "Projectile.h"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "ColoredDrawable.h"
#include "RoomManager.h"
#include "Bubble.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

Projectile::Projectile(const Color3& ambientColor) : GameObject()
{
	// Initialize members
	mAmbientColor = ambientColor;
	mVelocity = { 0.0f };
	mLeftX = 1.0f;
	mRightX = 19.0f;
	mSpeed = 50.0f;

	updateBBox();

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

void Projectile::update()
{
	// Affect position by velocity
	position += mVelocity * deltaTime * mSpeed;

	// Bounce against side walls
	if (position.x() <= mLeftX && mVelocity.x() < 0.0f)
	{
		position[0] = mLeftX;
		mVelocity[0] *= -1.0f;
	}
	else if (position.x() > mRightX && mVelocity.x() > 0.0f)
	{
		position[0] = mRightX;
		mVelocity[0] *= -1.0f;
	}

	// Update bounding box
	updateBBox();

	// Check for collision against other bubbles
	std::shared_ptr<GameObject> bubble = RoomManager::singleton->mCollisionManager->checkCollision(this);
	if (bubble != nullptr)
	{
		collidedWith(bubble.get());
	}
}

void Projectile::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	mColoredDrawable->mShader
		.setLightPositions({ position + Vector3({ 10.0f, 20.0f, 1.75f }) })
		.setDiffuseColor(mDiffuseColor)
		.setAmbientColor(mAmbientColor)
		.setTransformationMatrix(transformationMatrix * Matrix4::translation(position))
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.draw(mColoredDrawable->mMesh);
}

void Projectile::updateBBox()
{
	// Update bounding box
	bbox = Range3D{ position - Vector3(0.8f), position + Vector3(0.8f) };
}

void Projectile::collidedWith(GameObject* gameObject)
{
	Bubble* bubble = (Bubble*)gameObject;

	// Stop this projectile
	mVelocity = Vector3(0.0f);

	// Get row index
	const Int thisRowIndex = getRowIndexByBubble();

	// Snap to grid
	Float nx = thisRowIndex % 2 ? 1.0f : 0.0f;
	position = {
		round(position.x() / 2.0f) * 2.0f + nx,
		round(position.y() / 2.0f) * 2.0f,
		0.0f
	};

	// Create new bubbles with the same color
	std::shared_ptr<Bubble> b = std::make_shared<Bubble>(mAmbientColor);
	b->position = position;
	b->updateBBox();
	RoomManager::singleton->mGameObjects.push_back(b);

	// Destroy nearby bubbles
	b->destroyNearbyBubbles();

	// Destroy me
	destroyMe = true;
}

Int Projectile::getRowIndexByBubble()
{
	return Int(std::abs(position.y()) * 0.5f);
}