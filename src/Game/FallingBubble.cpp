#include "FallingBubble.h"

#include <thread>

#include <Corrade/Containers/PointerStl.h>
#include <Magnum/ImageView.h>
#include <Magnum/GL/Buffer.h>

#include "../Common/CommonUtility.h"
#include "../Graphics/GameDrawable.h"
#include "../Shaders/SpriteShader.h"
#include "../RoomManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> FallingBubble::getInstance(const nlohmann::json & params)
{
	// No default constructor exists for this class!!
	return nullptr;
}

FallingBubble::FallingBubble(const Int parentIndex, const Color3& ambientColor, const bool spark, const Float maxVerticalSpeed) : GameObject(parentIndex)
{
	// Assign members
	mParentIndex = parentIndex;
	mAmbientColor = ambientColor;
	mSpark = spark;
	mVelocity = { 0.0f };
	mMaxVerticalSpeed = maxVerticalSpeed;

	mDelay = Float(std::rand() % 250) * 0.001f;

	// Create sparkle plane
	if (mSpark)
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
		mWrapper.parameters.texWidth = Float(td->mTexture->imageSize(0).x());
		mWrapper.parameters.texHeight = Float(td->mTexture->imageSize(0).y());
		mWrapper.parameters.rows = 4.0f;
		mWrapper.parameters.columns = 4.0f;
		mWrapper.speed = 16.0f;
	}
	else
	{
		CommonUtility::singleton->createGameSphere(this, *mManipulator, mAmbientColor);
	}
}

const Int FallingBubble::getType() const
{
	return GOT_FALLING_BUBBLE;
}

void FallingBubble::update()
{
	// Update motion
	if (mSpark)
	{
		if (mWrapper.parameters.index >= mWrapper.parameters.total)
		{
			mDestroyMe = true;
		}
		else
		{
			mWrapper.parameters.index += mDeltaTime * mWrapper.speed;
		}
	}
	else
	{
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
	}

	// Update transformations
	if (mSpark)
	{
		// Apply transformations
		const Matrix4 mat = Matrix4::translation(mPosition) * Matrix4::scaling(Vector3(3.0f));
		mDrawables.at(0)->setTransformation(mat);
	}
	else
	{
		const Matrix4 mat = Matrix4::translation(mPosition);
		mDrawables.at(0)->setTransformation(mat);
	}
}

void FallingBubble::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (mSpark)
	{
		((SpriteShader&)baseDrawable->getShader())
			.bindTexture(*baseDrawable->mTexture)
			.setTransformationMatrix(transformationMatrix)
			.setProjectionMatrix(camera.projectionMatrix())
			.setColor(mAmbientColor)
			.setIndex(mWrapper.parameters.index)
			.setTextureWidth(mWrapper.parameters.texWidth)
			.setTextureHeight(mWrapper.parameters.texHeight)
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
	Resource<Audio::Buffer> buffer = CommonUtility::singleton->loadAudioData(mSpark ? RESOURCE_AUDIO_BUBBLE_POP : RESOURCE_AUDIO_BUBBLE_FALL);
	mPlayables[0] = std::make_shared<Audio::Playable3D>(*mManipulator.get(), &RoomManager::singleton->mAudioPlayables);
	mPlayables[0]->source()
		.setBuffer(buffer)
		.setMinGain(1.0f)
		.setMaxGain(1.0f)
		.setLooping(false);
	return mPlayables[0];
}