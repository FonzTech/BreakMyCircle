#include "LimitLine.h"

#include <Magnum/Shaders/Flat.h>

#include "../RoomManager.h"
#include "../Common/CommonUtility.h"

using namespace Magnum;

std::shared_ptr<GameObject> LimitLine::getInstance(const nlohmann::json & params)
{
	// No default constructor exists for this class!!
	return nullptr;
}

LimitLine::LimitLine(const Int parentIndex, const Color4 & color, const Int customType) : GameObject(parentIndex)
{
	// Assign members
	mParentIndex = parentIndex;
	mColor = color;
	mCustomType = customType;

	// Get assets
	Resource<GL::Mesh> mesh = CommonUtility::singleton->getPlaneMeshForFlatShader();
	Resource<GL::AbstractShaderProgram, Shaders::Flat3D> shader = CommonUtility::singleton->getFlat3DShader();
	Resource<GL::Texture2D> texture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_WHITE);

	// Create drawable
	auto& drawables = RoomManager::singleton->mGoLayers[parentIndex].drawables;

	const std::shared_ptr<GameDrawable<Shaders::Flat3D>> td = std::make_shared<GameDrawable<Shaders::Flat3D>>(*drawables, shader, mesh, mColor);
	td->mTexture = texture;
	td->setParent(mManipulator.get());
	td->setDrawCallback(this);
	mDrawables.emplace_back(td);
}

const Int LimitLine::getType() const
{
	return GOT_LIMIT_LINE;
}

void LimitLine::update()
{
	(*mManipulator)
		.resetTransformation()
		.scale(mScale)
		.translate(Vector3(mPosition));
}

void LimitLine::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	((Shaders::Flat3D&) baseDrawable->getShader())
		.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
		.setColor(baseDrawable->mColor)
		.setAlphaMask(0.1f)
		.bindTexture(*baseDrawable->mTexture)
		.draw(*baseDrawable->mMesh);
}

void LimitLine::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

const Int LimitLine::getCustomType() const
{
	return mCustomType;
}

const void LimitLine::setScale(const Vector3 & scale)
{
	mScale = scale;
}