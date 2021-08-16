#include "FallingBubble.h"

#include <thread>

#include <Corrade/Containers/PointerStl.h>
#include <Magnum/ImageView.h>
#include <Magnum/GL/Buffer.h>

#include "../Common/CommonUtility.h"
#include "../Graphics/GameDrawable.h"
#include "../Shaders/SpriteShader.h"
#include "../AssetManager.h"
#include "../RoomManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> FallingBubble::getInstance(const nlohmann::json & params)
{
	// No default constructor exists for this class!!
	return nullptr;
}

FallingBubble::FallingBubble(const Int parentIndex, const Color3& ambientColor, const Int customType, const Float maxVerticalSpeed) : GameObject(parentIndex), mCustomType(customType)
{
	// Assign members
	mParentIndex = parentIndex;
	mAmbientColor = ambientColor;
	mMaxVerticalSpeed = maxVerticalSpeed;

	// Create sparkle plane
	switch (mCustomType)
	{
	case GO_FB_TYPE_BUBBLE:

		// Init members
		mVelocity = { 0.0f };
		mDelay = Float(std::rand() % 250) * 0.001f;

		// Create bubble
		CommonUtility::singleton->createGameSphere(this, *mManipulator, mAmbientColor);

		break;

	case GO_FB_TYPE_SPARK:

		// Create assets
		{
			// Get sparkles texture
			Resource<GL::Texture2D> resTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_SPARKLES);

			// Create plane
			std::shared_ptr<GameDrawable<SpriteShader>> td = CommonUtility::singleton->createSpriteDrawable(mParentIndex, *mManipulator, resTexture, this);
			mDrawables.emplace_back(td);

			// Create shader data wrapper
			mWrapper.shader = &td->getShader();
			mWrapper.parameters.index = 0.0f;
			mWrapper.parameters.total = 16.0f;
			mWrapper.parameters.rows = 4.0f;
			mWrapper.parameters.columns = 4.0f;
			mWrapper.speed = 16.0f;
		}
		break;

	case GO_FB_TYPE_COIN:

		// Init members
		mVelocity = { 0.0f };
		mDelay = 0.0f;

		// Load assets
		AssetManager().loadAssets(*this, *mManipulator, RESOURCE_SCENE_COIN, this);

		break;
	}
}

const Int FallingBubble::getType() const
{
	return GOT_FALLING_BUBBLE;
}

void FallingBubble::update()
{
	// Update motion
	switch (mCustomType)
	{
	case GO_FB_TYPE_BUBBLE:

		// Advance falling animation
		if (mDelay > 0.0f)
		{
			mDelay -= mDeltaTime;
		}
		else
		{
			if (mPlayables.size() > 0 && mPlayables[0]->source().state() == Audio::Source::State::Initial)
			{
				mPlayables[0]->source().play();
			}

			if (mVelocity.y() > mMaxVerticalSpeed)
			{
				mVelocity += { 0.0f, -2.0f, 0.0f };
			}
			mPosition += mVelocity * mDeltaTime;
		}

		// Check for off-screen position
		if (mPosition.y() < -100.0f)
		{
			mDestroyMe = true;
			return;
		}

		// Apply transformations
		(*mManipulator)
			.resetTransformation()
			.translate(mPosition);

		break;

	case GO_FB_TYPE_COIN:

		// Play sound
		if (mPlayables.size() > 0 && mPlayables[0]->source().state() == Audio::Source::State::Initial)
		{
			mPlayables[0]->source().play();
		}

		// Update position
		{
			auto& p = mVelocity.data()[0];
			p += -p * mDeltaTime;

			if (p > 0.0f)
			{
				p = 0.0f;
			}
		}

		mPosition += mVelocity * mDeltaTime;

		// Control life-time
		if (mPosition.y() > 20.0f)
		{
			mDestroyMe = true;
		}

		// Apply transformations
		mDelay += Constants::pi() * mDeltaTime;

		(*mManipulator)
			.resetTransformation()
			.rotateX(Rad(Deg(90.0f)))
			.rotateY(Rad(mDelay))
			.translate(mPosition);

		break;

	case GO_FB_TYPE_SPARK:

		if (mWrapper.parameters.index >= mWrapper.parameters.total)
		{
			mDestroyMe = true;
		}
		else
		{
			mWrapper.parameters.index += mDeltaTime * mWrapper.speed;
		}

		// Apply transformations
		(*mManipulator)
			.resetTransformation()
			.scale(Vector3(3.0f, 3.0f, 1.0f))
			.translate(mPosition);

		break;
	}
}

void FallingBubble::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (mCustomType == GO_FB_TYPE_SPARK)
	{
		((SpriteShader&)baseDrawable->getShader())
			.bindTexture(*baseDrawable->mTexture)
			.setTransformationMatrix(transformationMatrix)
			.setProjectionMatrix(camera.projectionMatrix())
			.setColor(mAmbientColor)
			.setIndex(mWrapper.parameters.index)
			.setRows(mWrapper.parameters.rows)
			.setColumns(mWrapper.parameters.columns)
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		((Shaders::Phong&) baseDrawable->getShader())
			.setLightPosition(mPosition + Vector3(0.0f, 0.0f, 1.0f))
			.setLightColor(0x808080_rgbf)
			.setSpecularColor(0xffffff00_rgbaf)
			.setAmbientColor(0xc0c0c0_rgbf)
			.setDiffuseColor(0x808080_rgbf)
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
			.draw(*baseDrawable->mMesh);
	}
}

void FallingBubble::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

std::shared_ptr<Audio::Playable3D>& FallingBubble::buildBubbleSound()
{
	std::string filename;
	switch (mCustomType)
	{
	case GO_FB_TYPE_COIN:
		filename = RESOURCE_AUDIO_COIN;
		break;

	case GO_FB_TYPE_SPARK:
		filename = RESOURCE_AUDIO_BUBBLE_POP;
		break;

	case GO_FB_TYPE_BUBBLE:
		filename = RESOURCE_AUDIO_BUBBLE_FALL;
		break;
	}

	Resource<Audio::Buffer> buffer = CommonUtility::singleton->loadAudioData(filename);
	mPlayables[0] = std::make_shared<Audio::Playable3D>(*mManipulator.get(), &RoomManager::singleton->mAudioPlayables);
	mPlayables[0]->source()
		.setBuffer(buffer)
		.setMinGain(1.0f)
		.setMaxGain(1.0f)
		.setLooping(false);
	return mPlayables[0];
}