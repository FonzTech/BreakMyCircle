#include "Logo.h"

#include "AssetManager.h"
#include "RoomManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> Logo::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Sint8 parent;
	params.at("parent").get_to(parent);

	// Instantiate player object
	std::shared_ptr<Logo> p = std::make_shared<Logo>(parent);
	return p;
}

Logo::Logo(const Sint8 parentIndex) : GameObject()
{
	// Assign parent index
	mParentIndex = parentIndex;

	// Load assets
	AssetManager().loadAssets(*this, *mManipulator, "scenes/logo.glb", this);
}

const Int Logo::getType() const
{
	return GOT_LOGO;
}

void Logo::update()
{
	// Set camera
	auto& layer = RoomManager::singleton->mGoLayers[GOL_MAIN];
	layer.mCameraEye = position + Vector3(0.0f, 0.0f, 6.0f);
	layer.mCameraTarget = position;

	// Set transformations
	for (auto& item : mDrawables)
	{
		item->setTransformation(Matrix4::translation(position));
	}
}

void Logo::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	((Shaders::Phong&) baseDrawable->getShader())
		.setLightPosition(position + Vector3(0.0f, 0.0f, 1.0f))
		.setLightColor(0xffffff60_rgbaf)
		.setSpecularColor(0xffffff00_rgbaf)
		.setAmbientColor(0x00000080_rgbaf)
		.setDiffuseColor(0xffffffff_rgbaf)
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
		.draw(*baseDrawable->mMesh);
}

void Logo::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}