#include "FallingBubble.h"	

#include <thread>

#include <Corrade/Containers/PointerStl.h>
#include <Magnum/ImageView.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/Shaders/Flat.h>

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
	if (mCustomType == GO_FB_TYPE_BUBBLE || mCustomType == GO_FB_TYPE_BLACKHOLE)
	{
		// Init members
		mVelocity = Vector3(0.0f);
		mDelay = Float(std::rand() % 250) * 0.001f;

		// Load assets
		mFlatShader = CommonUtility::singleton->getFlat3DShader();
		CommonUtility::singleton->createGameSphere(this, *mManipulator, mAmbientColor);
	}
	else if (mCustomType == GO_FB_TYPE_SPARK)
	{
		// Init members
		mDelay = 0.0f;

		// Load assets
		{
			Resource<GL::Mesh> resMesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<Shaders::Flat3D::Position, Shaders::Flat3D::TextureCoordinates>(RESOURCE_MESH_PLANE_FLAT);
			Resource<GL::Texture2D> resTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_SPARKLE);
			mFlatShader = CommonUtility::singleton->getFlat3DShader();

			// Create drawable
			auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
			std::shared_ptr<GameDrawable<Shaders::Flat3D>> d = std::make_shared<GameDrawable<Shaders::Flat3D>>(*drawables, mFlatShader, resMesh, resTexture);
			d->setParent(&*mManipulator);
			d->setDrawCallback(this);
			mDrawables.emplace_back(d);
		}
	}
	else if (mCustomType == GO_FB_TYPE_COIN)
	{
		// Init members
		mVelocity = Vector3(0.0f);
		mDelay = 0.0f;

		// Load assets
		AssetManager().loadAssets(*this, *mManipulator, RESOURCE_SCENE_COIN, this);
	}
	else if (mCustomType == GO_FB_TYPE_BOMB)
	{
		// Init members
		mVelocity = Vector3(0.0f);
		mDelay = 0.0f;

		// Create assets
		{
			// Get sparkles texture
			Resource<GL::Texture2D> resTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_EXPLOSION);

			// Create plane
			const std::shared_ptr<GameDrawable<SpriteShader>> td = std::static_pointer_cast<GameDrawable<SpriteShader>>(CommonUtility::singleton->createSpriteDrawable(mParentIndex, *mManipulator, resTexture, this));
			mDrawables.emplace_back(td);

			// Create shader data wrapper
			mWrapper.shader = &td->getShader();
			mWrapper.parameters.index = 0.0f;
			mWrapper.parameters.total = 16.0f;
			mWrapper.parameters.rows = 4.0f;
			mWrapper.parameters.columns = 4.0f;
			mWrapper.speed = 16.0f;
		}
	}
	else if (mCustomType == GO_FB_TYPE_STONE)
	{
		// Init members
		mVelocity = Vector3(0.0f);
		mDelay = 0.0f;

		// Load assets
		AssetManager().loadAssets(*this, *mManipulator, RESOURCE_SCENE_STONE, this);
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
	case GO_FB_TYPE_STONE:
	case GO_FB_TYPE_BLACKHOLE:

		// Advance falling animation
		if (mDelay > 0.0f)
		{
			mDelay -= mDeltaTime;
		}
		else
		{
			// Play sound
			playPrimarySound();

			// Raise velocity
			if (mVelocity.y() > mMaxVerticalSpeed)
			{
				mVelocity += { 0.0f, -100.0f * mDeltaTime, 0.0f };
			}
			mPosition += mVelocity * mDeltaTime;
		}

		// Check for off-screen position
		if (mPosition.y() < -10.0f)
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
		playPrimarySound();

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

		mDelay += mDeltaTime;
		if (mDelay >= 1.0f)
		{
			mDelay = 1.0f;
			mDestroyMe = true;
		}

		(*mManipulator)
			.resetTransformation()
			.rotateZ(Deg(mDelay * 210.0f))
			.scale(Vector3(Math::sin(Deg(mDelay * 180.0f)) * 2.0f))
			.translate(mPosition + Vector3(0.0f, 0.0f, 0.05f));

		break;

	case GO_FB_TYPE_BOMB:

		playPrimarySound();
		checkForSpriteEnding();

		(*mManipulator)
			.resetTransformation()
			.scale(Vector3(8.0f, 8.0f, 1.0f))
			.translate(mPosition + Vector3(0.0f, 0.0f, 0.75f));

		break;
	}
}

void FallingBubble::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	const bool isWhite = mCustomType == GO_FB_TYPE_BUBBLE || mCustomType == GO_FB_TYPE_BLACKHOLE;
	if (isWhite || mCustomType == GO_FB_TYPE_SPARK)
	{
		((Shaders::Flat3D&)*mFlatShader)
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindTexture(*baseDrawable->mTexture)
			.setColor(isWhite ? 0xffffffff_rgbaf : mAmbientColor)
			.setAlphaMask(0.5f)
			.draw(*baseDrawable->mMesh);
	}
	else if (mCustomType == GO_FB_TYPE_BOMB)
	{
		((SpriteShader&)baseDrawable->getShader())
			.bindTexture(*baseDrawable->mTexture)
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.setColor(mAmbientColor)
			.setIndex(mWrapper.parameters.index)
			.setRows(mWrapper.parameters.rows)
			.setColumns(mWrapper.parameters.columns)
			.setAlphaMask(0.5f)
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

std::shared_ptr<Audio::Playable3D>& FallingBubble::buildSound()
{
	std::string filename;
	switch (mCustomType)
	{
	case GO_FB_TYPE_BUBBLE:
	case GO_FB_TYPE_BLACKHOLE:
		filename = RESOURCE_AUDIO_BUBBLE_FALL;
		break;

	case GO_FB_TYPE_SPARK:
		filename = RESOURCE_AUDIO_BUBBLE_POP;
		break;

	case GO_FB_TYPE_COIN:
		filename = RESOURCE_AUDIO_COIN;
		break;

	case GO_FB_TYPE_BOMB:
		filename = RESOURCE_AUDIO_EXPLOSION;
		break;

	case GO_FB_TYPE_STONE:
		filename = RESOURCE_AUDIO_STONE;
		break;
	}

	Resource<Audio::Buffer> buffer = CommonUtility::singleton->loadAudioData(filename);
	mPlayables[0] = std::make_shared<Audio::Playable3D>(*mManipulator, &RoomManager::singleton->mAudioPlayables);
	mPlayables[0]->source()
		.setBuffer(buffer)
		.setLooping(false);
	return mPlayables[0];
}

void FallingBubble::checkForSpriteEnding()
{
	if (mWrapper.parameters.index >= mWrapper.parameters.total - 0.5f)
	{
		mDestroyMe = true;
	}
	else
	{
		mWrapper.parameters.index += mDeltaTime * mWrapper.speed;
	}
}

void FallingBubble::playPrimarySound()
{
	if (mPlayables.size() > 0 && mPlayables[0]->source().state() == Audio::Source::State::Initial)
	{
		playSfxAudio(0);
	}
}