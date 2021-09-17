#include "SafeMinigame.h"

#include <Magnum/Animation/Easing.h>

#include "../AssetManager.h"
#include "../InputManager.h"

using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> SafeMinigame::getInstance(const nlohmann::json & params)
{
	// No default constructor exists for this class!!
	return nullptr;
}

SafeMinigame::SafeMinigame(const Int parentIndex) : GameObject(parentIndex)
{
	// Assign members
	mParentIndex = parentIndex;
	mMode = 0;
	mScale = 0.0f;
	mAnimY = 0.0f;
	mAngleHandle = 0.0f;

	// Get assets
	AssetManager().loadAssets(*this, *mManipulator, RESOURCE_SCENE_SAFE, this);

	for (const auto& item : mDrawables)
	{
		if (item->mMesh->label() == "HandleV")
		{
			mDrawableHandle = item;
		}
	}
}

const Int SafeMinigame::getType() const
{
	return GOT_SAFE_MINIGAME;
}

void SafeMinigame::update()
{
	// Handle mode
	switch (mMode)
	{
	case 0:
		// Compute animations
		if (mScale < 1.0f)
		{
			mScale += mDeltaTime;
		}
		else
		{
			mScale = 1.0f;

			mAnimY += mDeltaTime;
			mPosition = Vector3(0.0f, Math::sin(Deg(mAnimY * 180.0f)) * 0.25f, 0.0f);
		}

		// Check for tap
		if (mScale >= 0.99f && InputManager::singleton->mMouseStates[PRIMARY_BUTTON] == IM_STATE_RELEASED)
		{
			mAnimY = std::fmodf(mAnimY, 2.0f);
			mMode = 1;
		}
		break;

	case 1:
		mAnimY -= mDeltaTime;
		mPosition = Vector3(0.0f, Math::sin(Deg(mAnimY * 180.0f)) * 0.25f, 0.0f);

		if (mAnimY < 0.0f)
		{
			mPosition = Vector3(0.0f);
			mAnimY = 0.0f;
			mMode = 2;
		}
		break;

	case 2:
		mAngleHandle += mDeltaTime;
		if (mAngleHandle > 2.0f)
		{
			mAngleHandle = 2.0f;
			mMode = 3;
		}
		break;
	}

	// Set transformations
	{
		(*mManipulator)
			.resetTransformation()
			.scale(Vector3(mScale < 1.0f ? Math::lerp(0.0f, 1.0f, Animation::Easing::smoothstep(mScale)) : 1.0f))
			.translate(Vector3(mPosition));

		const Float rot = mAngleHandle < 2.0f ? Math::lerp(0.0f, 1.0f, Animation::Easing::smoothstep(mAngleHandle * 0.5f)) : 1.0f;
		(*mDrawableHandle.lock())
			.resetTransformation()
			.rotateZ(Deg(rot * 720.0f));
	}
}

void SafeMinigame::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{	
	((Shaders::Phong&)baseDrawable->getShader())
		.setLightPosition(camera.cameraMatrix().transformPoint(mPosition + Vector3(1.5f, 2.5f, 1.5f)))
		.setLightColor(0xc0c0c0_rgbf)
		.setSpecularColor(0x000000_rgbf)
		.setDiffuseColor(0x909090_rgbf)
		.setAmbientColor(0x909090_rgbf)
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
		.draw(*baseDrawable->mMesh);
}

void SafeMinigame::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}