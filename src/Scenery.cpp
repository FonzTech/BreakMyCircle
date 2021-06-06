#include "Scenery.h"

#include "AssetManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

Scenery::Scenery()
{
	// Load asset as first drawable
	AssetManager::singleton->loadAssets(*this, "scenes/world_1.glb", this);
}

const Int Scenery::getType() const
{
	return GOT_SCENERY;
}

void Scenery::update()
{
	// Apply transformationss
	const auto& m = Matrix4::translation(position + Vector3(8.0f, 1.0f, 0.0f)) * Matrix4::rotationX(Deg(90.0f));
	mManipulator->setTransformation(m);
}

void Scenery::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	((Shaders::Phong&) baseDrawable->getShader())
		.setLightPosition(camera.cameraMatrix().transformPoint(position + Vector3(0.0f, 0.0f, 20.0f)))
		.setLightColor(0xffffff00_rgbaf)
		.setSpecularColor(0x00000000_rgbaf)
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
		.draw(*baseDrawable->mMesh);
}

void Scenery::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}