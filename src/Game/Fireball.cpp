#include "Fireball.h"

#include <Magnum/Shaders/Flat.h>

#include "../Common/CommonUtility.h"
#include "../Graphics/GameDrawable.h"
#include "../RoomManager.h"

using namespace Magnum;

std::shared_ptr<GameObject> Fireball::getInstance(const nlohmann::json & params)
{
	// No default constructor exists for this class!!
	return nullptr;
}

Fireball::Fireball(const Int parentIndex) : GameObject(parentIndex)
{
	// Initialize members
	mParentIndex = parentIndex;
	mFrame = 0.0f;

	// Load assets
	Resource<GL::Mesh> resMesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<Shaders::Flat3D::Position, Shaders::Flat3D::TextureCoordinates>(RESOURCE_MESH_PLANE_FLAT);
	Resource<GL::Texture2D> resTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_FIRE);
	Resource<GL::AbstractShaderProgram, Shaders::Flat3D> resShader = CommonUtility::singleton->getFlat3DShader();

	// Create drawable
	auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
	std::shared_ptr<GameDrawable<Shaders::Flat3D>> d = std::make_shared<GameDrawable<Shaders::Flat3D>>(*drawables, resShader, resMesh, resTexture);
	d->setParent(mManipulator.get());
	d->setDrawCallback(this);
	mDrawables.emplace_back(d);
}

const Int Fireball::getType() const
{
	return GOT_FIREBALL;
}

void Fireball::update()
{
	// Raise animation
	mFrame += mDeltaTime;

	// Upgrade transformations
	(*mManipulator)
		.resetTransformation()
		.scale(Vector3(10.0f))
		.translate(mPosition);
}

void Fireball::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	((Shaders::Flat3D&) baseDrawable->getShader())
		.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
		.bindTexture(*baseDrawable->mTexture)
		.setColor(0xffffff_rgbf)
		.setAlphaMask(0.001f)
		.draw(*baseDrawable->mMesh);
}