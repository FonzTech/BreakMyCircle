#include "SafeMinigame.h"

#include <Magnum/Animation/Easing.h>

#include "LevelSelector.h"
#include "../AssetManager.h"
#include "../InputManager.h"
#include "../RoomManager.h"
#include "../Common/CommonUtility.h"
#include "../Graphics/GameDrawable.h"

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
	mGlowAnim = 0.0f;
	mScale = 0.0f;
	mAnimY = 0.0f;
	mAngleHandle = 0.0f;
	mAngleDoor = 0.0f;
	mPuRotation = 0.0f;
	mPuAnimation = 0.0f;

	// Get powerup index to obtain
	mPowerupIndex = std::rand() % GO_LS_MAX_POWERUP_COUNT;

	// Get assets
	mSafeManipulator = new Object3D{ mManipulator.get() };
	AssetManager().loadAssets(*this, *mSafeManipulator, RESOURCE_SCENE_SAFE, this);

	// Get assets
	mPowerupManipulator = new Object3D{ mManipulator.get() };
	AssetManager().loadAssets(*this, *mPowerupManipulator, RESOURCE_SCENE_POWERUP, this);

	// Create plane for glow effect
	{
		// Load assets
		Resource<GL::Mesh> resMesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<Shaders::Flat3D::Position, Shaders::Flat3D::TextureCoordinates>(RESOURCE_MESH_PLANE_FLAT);
		Resource<GL::Texture2D> resTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_GLOW);
		Resource<GL::AbstractShaderProgram, Shaders::Flat3D> resShader = CommonUtility::singleton->getFlat3DShader();

		// Create child manipulator
		mGlowManipulator = new Object3D{ mManipulator.get() };

		// Create drawable
		auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
		std::shared_ptr<GameDrawable<Shaders::Flat3D>> d = std::make_shared<GameDrawable<Shaders::Flat3D>>(*drawables, resShader, resMesh, resTexture);
		d->setParent(mGlowManipulator);
		d->setDrawCallback(this);
		mDrawables.emplace_back(d);

        mDrawableGlow = d;
	}

	for (const auto& item : mDrawables)
	{
		if (item->mMesh->label() == "HandleV")
		{
			mDrawableHandle = item;
		}
		else if (item->mMesh->label() == "DoorV")
		{
			mDrawableDoor = item;
		}
		else if (item->mMesh->label() == "KnobV")
		{
			mDrawableKnob = item;
		}
        else if (item->mMesh->label() == "FaceV")
        {
            const std::string tn = CommonUtility::singleton->getTextureNameForPowerup(mPowerupIndex);
            item->mTexture = CommonUtility::singleton->loadTexture(tn);
        }
	}

	// Create overlay texts
	{
		const std::string& text = "Tap to discover\nyour powerup!";
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::MiddleCenter, 30U);
		go->mPosition = Vector3(0.0f, 0.39f, 0.0f);
		go->mColor.data()[3] = 0.0f;
		go->mOutlineColor.data()[3] = 0.0f;
		go->setText(text);

		mTexts[0] = (std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(go, true);
	}

	// Load audios
	for (UnsignedInt i = 0; i < 2; ++i)
	{
		Resource<Audio::Buffer> buffer = CommonUtility::singleton->loadAudioData(i == 0 ? RESOURCE_AUDIO_COIN : RESOURCE_AUDIO_SAFE_OPEN);
		mPlayables[i] = std::make_shared<Audio::Playable3D>(*mManipulator.get(), &RoomManager::singleton->mAudioPlayables);
		mPlayables[i]->source()
			.setBuffer(buffer)
			.setLooping(false);
	}
}

SafeMinigame::~SafeMinigame()
{
	for (auto& text : mTexts)
	{
		text->mDestroyMe = true;
	}
}

const Int SafeMinigame::getType() const
{
	return GOT_SAFE_MINIGAME;
}

void SafeMinigame::update()
{
	// Animate glow plane
	mGlowAnim += mDeltaTime;
	(*mGlowManipulator)
		.resetTransformation()
		.scale(Vector3(Math::min(1.0f, mScale) * 0.25f))
		.rotateZ(Deg(mGlowAnim * 90.0f))
		.translate(mPosition - Vector3(0.0f, 0.0f, 0.04f));

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
			mPosition = Vector3(0.0f, Math::sin(Deg(mAnimY * 180.0f)) * 0.01f, 0.0f);
		}

		// Set alpha for texts
		if (mScale > 0.99f)
		{
			animateTextAlpha(0U, true);
		}

		// Check for tap
		if (mTexts[0]->mColor.data()[3] >= 0.99f && InputManager::singleton->mMouseStates[PRIMARY_BUTTON] == IM_STATE_RELEASED)
		{
			mAnimY = std::fmodf(mAnimY, 2.0f);
			mMode = 1;
		}
		break;

	case 1:
		// Set animation
		mAnimY -= mDeltaTime;
		mPosition = Vector3(0.0f, Math::sin(Deg(mAnimY * 180.0f)) * 0.01f, 0.0f);

		// Set alpha for texts
		animateTextAlpha(0U, false);

		// Manage mode change
		if (mAnimY < 0.0f)
		{
			playSfxAudio(1);
			mPosition = Vector3(0.0f);
			mAnimY = 0.0f;
			mMode = 2;
		}
		break;
		
	case 2:
		// Door handle animation6
		mAngleHandle += mDeltaTime;
		if (mAngleHandle > 2.0f)
		{
			mAngleHandle = 2.0f;
			mMode = 3;
		}
		break;

	case 3:
		mPuRotation += mDeltaTime;
		mAngleDoor += mDeltaTime;
		if (mAngleDoor > 1.0f)
		{
			mTexts[0]->setText("Congratulations!\nTap to get it.");
			mAngleDoor = 1.0f;
			mMode = 4;
		}
		break;

	case 4:
		mPuRotation += mDeltaTime;
		animateTextAlpha(0U, true);

		// Check for tap
		if (mTexts[0]->mColor.data()[3] >= 0.99f && InputManager::singleton->mMouseStates[PRIMARY_BUTTON] == IM_STATE_RELEASED)
		{
			obtainPowerup();
			playSfxAudio(0);
			mAnimY = std::fmodf(mAnimY, 2.0f);
			mMode = 5;
		}
		break;

	case 5:
		animateTextAlpha(0U, false);
		mPuRotation += mDeltaTime * 2.0f;
		mPuAnimation += mDeltaTime;
		if (mPuAnimation > 1.0f)
		{
			mMode = 6;
		}
		break;

	case 6:
		mScale -= mDeltaTime;
		if (mScale < 0.0f)
		{
			mScale = 0.0f;
			mDestroyMe = true;
		}
		break;
	}

	// Set transformations for safe
	{
		(*mSafeManipulator)
			.resetTransformation()
			.scale(Vector3(mScale < 1.0f ? Math::lerp(0.0f, 1.0f, Animation::Easing::smoothstep(mScale)) : 1.0f))
			.translate(Vector3(mPosition));

		const Float rotDoor = mAngleDoor < 1.0f ? Math::lerp(0.0f, 1.0f, Animation::Easing::smoothstep(mAngleDoor)) : 1.0f;
		const Rad rotKnob = Rad(Deg(rotDoor * 90.0f));
		const Vector3 rotPos = Vector3(-0.036f * Math::cos(rotKnob), 0.0f, 0.036f * Math::sin(rotKnob));

		// Handle
		{
			const Float rot = mAngleHandle < 2.0f ? Math::lerp(0.0f, 1.0f, Animation::Easing::smoothstep(mAngleHandle * 0.5f)) : 1.0f;
			(*mDrawableHandle.lock())
				.resetTransformation()
				.rotateZ(Deg(rot * -720.0f))
				.rotateY(rotKnob)
				.translate(rotPos);
		}

		// Knob
		{
			(*mDrawableKnob.lock())
				.resetTransformation()
				.rotateY(rotKnob)
				.translate(rotPos);
		}

		// Door
		{
			(*mDrawableDoor.lock())
				.resetTransformation()
				.rotateY(Deg(rotDoor * 90.0f));
		}
	}


	// Set transformations for powerup
	{
		const Float rot = mPuRotation;
		(*mPowerupManipulator)
			.resetTransformation()
			.scale(Vector3(0.025f))
			.rotateY(Deg(rot * 180.0f))
			.translate(mMode >= 3 && mMode < 6 ? mPosition + Vector3(0.0f, 0.0f, mPuAnimation) : Vector3(1000.0f));
	}
}

void SafeMinigame::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (!mDrawableGlow.expired() && mDrawableGlow.lock().get() == baseDrawable)
	{
		((Shaders::Flat3D&)baseDrawable->getShader())
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindTexture(*baseDrawable->mTexture)
			.setColor(Color4{ 1.0f })
			.setAlphaMask(0.001f)
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		const bool b2 = baseDrawable->mMesh->label() == "FaceV" || baseDrawable->mMesh->label() == "PowerupV";
#ifdef CORRADE_TARGET_ANDROID
        const auto ambientColor = b2 ? 0x303030_rgbf : 0x909090_rgbf;
#else
        const auto ambientColor = 0x909090_rgbf;
#endif

		((Shaders::Phong&)baseDrawable->getShader())
			.setLightPosition(camera.cameraMatrix().transformPoint(mPosition + Vector3(3.0f)))
			.setLightColor(b2 ? 0x202020_rgbf : 0xc0c0c0_rgbf)
			.setSpecularColor(0x00000000_rgbaf)
			.setDiffuseColor(ambientColor)
			.setAmbientColor(0x909090_rgbf)
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
			.draw(*baseDrawable->mMesh);
	}
}

void SafeMinigame::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void SafeMinigame::obtainPowerup()
{
	Debug{} << "Obtain powerup" << mPowerupIndex;
	++RoomManager::singleton->mSaveData.powerupAmounts[mPowerupIndex];
	RoomManager::singleton->mSaveData.save();
}

void SafeMinigame::animateTextAlpha(const UnsignedInt index, const bool increment)
{
	auto* p = &mTexts[index]->mColor.data()[3];
	*p += increment ? mDeltaTime : -mDeltaTime;
	if (*p < 0.0f)
	{
		*p = 0.0f;
	}
	else if (*p > 1.0f)
	{
		*p = 1.0f;
	}
	mTexts[index]->mOutlineColor.data()[3] = *p;
}