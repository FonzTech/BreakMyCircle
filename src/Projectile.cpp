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
	mSpeed = 20.0f;

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
	std::shared_ptr<GameObject> bubble = RoomManager::singleton->mCollisionManager->checkBubbleCollision(this);
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
	bbox = Range3D{ position - Vector3(1.0f), position + Vector3(1.0f) };
}

void Projectile::collidedWith(GameObject* gameObject)
{
	Bubble* bubble = (Bubble*)gameObject;

	// Stop this projectile
	mVelocity = Vector3(0.0f);

	// Snap to grid
	position = {
		floorf(position.x()) / 8.0f * 8.0f,
		floorf(position.y()) / 8.0f * 8.0f,
		0.0f
	};

	// Check if bubble is the same color as this projectile
	if (bubble->mAmbientColor == mAmbientColor)
	{
		mAmbientColor = 0xffffff_rgbf;
		bubble->mAmbientColor = 0xffffff_rgbf;
		bubble->destroyNearbyBubbles();
	}
	else
	{
		std::shared_ptr<Bubble> b = std::make_shared<Bubble>(mAmbientColor);
		b->position = position;
		b->bbox = Range3D{ position - Vector3(1.0f), position + Vector3(1.0f) };
		RoomManager::singleton->mGameObjects.push_back(b);

		const Int rowIndex = RoomManager::singleton->mCollisionManager->getRowIndexByBubble(b.get());
		RoomManager::singleton->mCollisionManager->addBubbleToRow(rowIndex, b);
	}

	// Destroy me
	destroyMe = true;
}