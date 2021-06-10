#include "Scenery.h"

#include "AssetManager.h"
#include "RoomManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

Scenery::Scenery()
{
	// Create manipulator list
	mManipulatorList.push_back(new Object3D{ mManipulator.get() });
	mManipulatorList.push_back(new Object3D{ mManipulator.get() });

	// Load assets
	AssetManager::singleton->loadAssets(*this, *mManipulatorList[0], "scenes/world_1.glb", this);
	AssetManager::singleton->loadAssets(*this, *mManipulatorList[1], "scenes/scenery_1.glb", this);
}

const Int Scenery::getType() const
{
	return GOT_SCENERY;
}

void Scenery::update()
{
	// Apply transformationss
	{
		const auto& m = Matrix4::translation(position + Vector3(8.0f, 1.0f, 0.0f)) * Matrix4::rotationX(Deg(90.0f));
		mManipulatorList[0]->setTransformation(m);
	}

	{
		const auto& m = Matrix4::translation(position + Vector3(8.0f, -35.0f, -1.1f));
		mManipulatorList[1]->setTransformation(m);
	}
}

void Scenery::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	Vector3 pos[] = {
		Vector3(8.0f, 20.0f, 1.0f),
		Vector3(8.0f, -40.0f, 1.0f),
		Vector3(0.0f, 0.0f, 20.0f)
	};

	((Shaders::Phong&) baseDrawable->getShader())
		/*
		.setLightPositions({
			camera.cameraMatrix().transformPoint(position + pos[0]),
			camera.cameraMatrix().transformPoint(position + pos[1])
		})
		*/
		.setLightPosition(camera.cameraMatrix().transformPoint(position + pos[0]))
		.setLightColor(0xffffff00_rgbaf)
		.setSpecularColor(0xffffff00_rgbaf)
		.setAmbientColor(0x444444ff_rgbaf)
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
		.draw(*baseDrawable->mMesh);
}

void Scenery::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}