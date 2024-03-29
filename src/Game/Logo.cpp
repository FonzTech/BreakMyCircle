#include "Logo.h"

#include <Corrade/Containers/StridedArrayView.h>
#include <Magnum/Animation/Easing.h>
#include <Magnum/Resource.h>
#include <Magnum/Math/Math.h>
#include <Magnum/GL/Mesh.h>

#include "../AssetManager.h"
#include "../RoomManager.h"
#include "../InputManager.h"
#include "../Common/CommonUtility.h"
#include "../Graphics/BaseDrawable.h"
#include "Bubble.h"
#include "FallingBubble.h"
#include "SafeMinigame.h"

using namespace Corrade;
using namespace Magnum;
using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> Logo::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate player object
	std::shared_ptr<Logo> p = std::make_shared<Logo>(parent);
	return p;
}

Logo::Logo(const Int parentIndex) : GameObject()
{
	// Assign parent index
	mParentIndex = parentIndex;

	// Check if safe minigame shall be created
	{
		const auto& value = CommonUtility::singleton->getValueFromIntent(GO_LG_INTENT_SAFE_MINIGAME);
		mSafeMinigame = value != nullptr;
	}
	
	// Init members
	computeCanvasPadding();

	mLightPosition = Vector3(0.0f, -0.5f, 0.0f);
	mLightDirection = false;
	mIntroBubbles = true;
	mLogoZoom = 0.0f;
	mAnimElapsed = -3.001f; // Cycle waste
	mPlaneAlpha = 1.5f;

	// Load assets
	mPosition = Vector3(0.0f, 10.0f, 0.0f);
	mManipulator->setTransformation(Matrix4::translation(mPosition));

	mLogoManipulator = new Object3D(mManipulator);
	AssetManager().loadAssets(*this, *mLogoManipulator, RESOURCE_SCENE_LOGO, this);

	// Create black plane
	{
		Resource<GL::Mesh> resMesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<Shaders::Flat3D::Position, Shaders::Flat3D::TextureCoordinates>(RESOURCE_MESH_PLANE_FLAT);
		Resource<GL::Texture2D> resTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_WHITE);
		Resource<GL::AbstractShaderProgram, Shaders::Flat3D> resShader = CommonUtility::singleton->getFlat3DShader();

		// Create child manipulator
		mPlaneManipulator = new Object3D{ mManipulator };

		(*mPlaneManipulator)
			.resetTransformation()
			.scale(Vector3(100.0f))
			.translate(Vector3(0.0f, 0.0f, -10.0f));

		// Create drawable
		auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
		std::shared_ptr<GameDrawable<Shaders::Flat3D>> d = std::make_shared<GameDrawable<Shaders::Flat3D>>(*drawables, resShader, resMesh, resTexture);
		d->setParent(mPlaneManipulator);
		d->setDrawCallback(this);
		mDrawables.emplace_back(d);
	}

	// Filter required meshes
	const std::unordered_map<std::string, UnsignedInt> indexes{
		{ "BreakV", 0 },
		{ "MyV", 1 },
		{ "CircleV", 2 }
	};

	for (auto& item : mDrawables)
	{
		const auto& label = item->mMesh->label();
		const auto& it = indexes.find(label);
		if (it != indexes.end())
		{
			mLogoObjects[it->second] = (Object3D*)item.get();
			(*mLogoObjects[it->second])
				.resetTransformation()
				.translate(Vector3(500.0f));
		}
	}

	// Init timers
	mBubbleTimer = 0.0f;
	mFinishTimer = GO_LS_FINISH_TIMER_STARTING_VALUE;

	// Create overlay texts
	{
		const std::string& text = "Tap here to begin";
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::MiddleCenter, UnsignedInt(text.length()));
		go->mPosition = Vector3(0.0f, -0.15f, 0.0f);
		go->mSize = Vector2(1.0f);
		go->mColor.data()[3] = 0.0f;
		go->mOutlineColor.data()[3] = 0.0f;
		go->setText(text);

		mTexts[0] = go;
		RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(go);
	}

	{
		const std::string& text = "Created by\nFonzTech";
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::MiddleCenter, UnsignedInt(text.length()));
		go->mSize = Vector2(1.0f);
		go->mColor = Color4(1.0f, 0.5f, 0.5f, 0.0f);
		go->mOutlineColor.data()[3] = 0.0f;
		go->setText(text);

		mTexts[1] = go;
		RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(go);
	}

	{
		const std::string& text = "Copyright 2021";
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::MiddleCenter, UnsignedInt(text.length()));
		go->mSize = Vector2(0.75f);
		go->mColor = Color4(1.0f, 0.9f, 0.0f, 0.0f);
		go->mOutlineColor.data()[3] = 0.0f;
		go->setText(text);

		mTexts[2] = go;
		RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(go);
	}

	// Build animations
	buildAnimations();
}

Logo::~Logo()
{
	for (auto& item : mTexts)
	{
		item->mDestroyMe = true;
	}
}

const Int Logo::getType() const
{
	return GOT_LOGO;
}

void Logo::update()
{
	// Set camera parameters
	setCameraParameters();

	// Set positions based on canvas padding
	computeCanvasPadding();
	mTexts[1]->mPosition = Vector3(0.0f, -0.3f + mCanvasPadding, 0.0f);
	mTexts[2]->mPosition = Vector3(0.0f, -0.46f + mCanvasPadding, 0.0f);

	// Advance animation
	if (mAnimElapsed < 0.0f) // Cycle waste
	{
		mAnimElapsed += 1.0f;
		return;
	}
	else
	{
		mPlaneAlpha -= mDeltaTime;
		mAnimElapsed += mDeltaTime;
		mAnimPlayer->advance(mAnimElapsed);
	}

	// Check for bubble timer
	if (mBubbleTimer > 0.0f)
	{
		mBubbleTimer -= mDeltaTime;
	}
	else if (mIntroBubbles)
	{
		if (mPlaneAlpha < 0.001f)
		{
			// Create bubble of random color
			const auto& index = std::rand() % RoomManager::singleton->sBubbleKeys.size();
			const auto& ckey = RoomManager::singleton->sBubbleKeys[index];
			const auto& color = RoomManager::singleton->sBubbleColors[ckey].color;

			// Create random bubble
			std::shared_ptr<FallingBubble> fb = std::make_shared<FallingBubble>(mParentIndex, color, GO_FB_TYPE_BUBBLE, -25.0f);
			fb->mPosition = mPosition;
			fb->mPosition -= RoomManager::singleton->mGoLayers[mParentIndex].cameraEye;
			fb->mPosition += Vector3(-6.0f + 12.0f * (std::rand() % 12) / 12.0f, 20.0, 0.0f);
			RoomManager::singleton->mGoLayers[mParentIndex].push_back(fb);

			// Reset timer
			mBubbleTimer = Float(std::rand() % 100) * 0.001f + 0.25f;
		}
		else
		{
			mBubbleTimer = 0.1f;
		}
	}

	// Check for finish timer
	if (mFinishTimer < -1.0f)
	{
		// Handle "tap here"
		{
			if (InputManager::singleton->mMouseStates[PRIMARY_BUTTON] == IM_STATE_RELEASED && mTexts[0]->mColor.a() >= 0.99f)
			{
				mIntroBubbles = false;
			}

			// Handle its text
			for (UnsignedInt i = 0; i < 3; ++i)
			{
				for (UnsignedInt j = 0; j < 2; ++j)
				{
					auto* p = &(j ? mTexts[i]->mOutlineColor : mTexts[i]->mColor).data()[3];

					if (mIntroBubbles)
					{
						*p += mDeltaTime;
					}
					else
					{
						*p -= mDeltaTime;

						if (i == 0 && j == 0)
						{
							mLogoZoom += mDeltaTime;

							const Float z = Math::lerp(0.0f, 1.0f, Animation::Easing::quadraticIn(mLogoZoom)) * RoomManager::singleton->getWindowAspectRatio() * -15.0f;
							for (UnsignedInt k = 0; k < 3; ++k)
							{
								(*mLogoObjects[k])
									.resetTransformation()
									.translate(mKeyframes[3][2].position + Vector3(0.0f, 0.0f, z));
							}
						}
					}

					if (*p > 1.0f)
					{
						*p = 1.0f;
					}
					else if (*p < -0.15f)
					{
						if (i == 0 && j == 0)
						{
							continueLogic();
							return;
						}
					}
				}
			}
		}

		// Handle logo light effect
		mLightPosition += Vector3(0.0f, mLightDirection ? -4.0f : 4.0f, 0.0f) * mDeltaTime;

		if (mLightPosition.y() > 11.0f)
		{
			if (!mLightDirection)
			{
				mLightPosition.data()[1] = 11.0f;
				mLightDirection = true;
			}
		}
		else if (mLightPosition.y() < -1.0f)
		{
			if (mLightDirection)
			{
				mLightPosition.data()[1] = -1.0f;
				mLightDirection = false;
			}
		}
	}
	else
	{
		mFinishTimer -= mDeltaTime;
	}
}

void Logo::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (baseDrawable->mTexture.key() == ResourceKey(RESOURCE_TEXTURE_WHITE))
	{
		((Shaders::Flat3D&) baseDrawable->getShader())
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindTexture(*baseDrawable->mTexture)
			.setColor(Color4(0.05f, 0.05f, 0.05f, Math::min(1.0f, mPlaneAlpha)))
			.setAlphaMask(0.001f)
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		const Float lightX = mFinishTimer < -1.0f ? 0.0f : RoomManager::singleton->getWindowAspectRatio() * 4.0f;
		((Shaders::Phong&) baseDrawable->getShader())
			.setLightPosition(mLightPosition + Vector3(lightX, 0.0f, 0.0f))
			.setLightColor(0xffffff60_rgbaf)
			.setSpecularColor(0xffffff00_rgbaf)
			.setAmbientColor(0x505050ff_rgbaf)
			.setDiffuseColor(0xffffffff_rgbaf)
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
			.draw(*baseDrawable->mMesh);
	}
}

void Logo::computeCanvasPadding()
{
#if defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_IOS_SIMULATOR)
    mCanvasPadding = 0.0f;
#else
    mCanvasPadding = CommonUtility::singleton->mConfig.canvasVerticalPadding;
    mCanvasPadding /= CommonUtility::singleton->mFramebufferSize.y();
    mCanvasPadding /= CommonUtility::singleton->mConfig.displayDensity;
#endif
}

void Logo::buildAnimations()
{
	// Create animation player
	mAnimPlayer = std::make_unique<Animation::Player<Float>>();

	// Cycle through all the three meshes
	for (UnsignedInt i = 0; i < 4; ++i)
	{
		// Create raw animation data
		switch (i)
		{
		case 0:
			mKeyframes[i][0] = { 0.0f, Vector3(10.0f, 10.0f, 0.0f), 360.0_degf * 4 };
			mKeyframes[i][1] = { 0.01f, Vector3(10.0f, 10.0f, 0.0f), 360.0_degf * 4 };
			mKeyframes[i][2] = { 3.0f, Vector3(0.0f, 0.0f, 0.0f), 0.0_degf };
			break;

		case 1:
			mKeyframes[i][0] = { 0.0f, Vector3(0.0f, 10.0f, 0.0f), 360.0_degf * 4 };
			mKeyframes[i][1] = { 2.5f, Vector3(0.0f, 10.0f, 0.0f), 360.0_degf * 4 };
			mKeyframes[i][2] = { 4.5f, Vector3(0.0f, 0.0f, 0.0f), 0.0_degf };
			break;

		case 2:
		{
			const auto& xp = RoomManager::singleton->getWindowAspectRatio() * -20.0f;
			mKeyframes[i][0] = { 0.0f, Vector3(0.0f, 0.0f, xp), 360.0_degf * 2.0f };
			mKeyframes[i][1] = { 4.0f, Vector3(0.0f, 0.0f, xp), 360.0_degf * 2.0f };
			mKeyframes[i][2] = { 6.0f, Vector3(0.0f, 0.0f, 0.0f), 0.0_degf };
		}
			break;

		case 3:
			mKeyframes[i][0] = { 0.0f, Vector3(0.0f, 0.0f, 0.0f), 0.0_degf };
			mKeyframes[i][1] = { 7.0f, Vector3(0.0f, 0.0f, 0.0f), 0.0_degf };
			mKeyframes[i][2] = { 8.0f, Vector3(0.0f, -2.0f + mCanvasPadding * 8.0f, 0.0f), 0.0_degf };
			break;
		}

		const bool notLast = i < 3;
		auto as = Containers::arraySize(mKeyframes[i]);
		auto at = Containers::StridedArrayView1D<Float>{ mKeyframes, &mKeyframes[i][0].time, as, sizeof(Keyframe) };

		// Track view for rotations
		if (notLast)
		{
			mTrackViewRotations[i] = std::make_unique<AnimRotation>(
				at,
				Containers::StridedArrayView1D<Deg>{ mKeyframes, &mKeyframes[i][0].rotation, as, sizeof(Keyframe) },
				Animation::ease<Deg, Math::lerp, Animation::Easing::quadraticOut>()
				);
		}

		// Track view for positions
		mTrackViewPositions[i] = std::make_unique<AnimPosition>(
			at,
			Containers::StridedArrayView1D<Vector3>{ mKeyframes, &mKeyframes[i][0].position, as, sizeof(Keyframe) },
			notLast ? Animation::ease<Vector3, Math::lerp, Animation::Easing::quadraticOut>() : Animation::ease<Vector3, Math::lerp, Animation::Easing::quadraticInOut>()
			);

		// Animations for both positions and rotations
		if (notLast)
		{
			mAnimPlayer->addWithCallback(
				*mTrackViewRotations[i],
				[](Float, const Deg& tr, Object3D& object) {
				object
					.resetTransformation()
					.rotate(tr, Vector3::yAxis());
			},
				*mLogoObjects[i]
				);

			mAnimPlayer->addWithCallback(
				*mTrackViewPositions[i],
				[](Float, const Vector3& tp, Object3D& object) {
				object.translate(tp);
			},
				*mLogoObjects[i]
				);
		}
		else
		{
			for (UnsignedInt j = 0; j < 3; ++j)
			{
				mAnimPlayer->addWithCallback(
					*mTrackViewPositions[i],
					[](Float, const Vector3& tp, Object3D& object) {
					object.translate(tp);
				},
					*mLogoObjects[j]
					);
			}
		}
	}

	// Start animation
	mAnimPlayer->play(0.001f);
}

void Logo::setCameraParameters()
{
	const auto& ar = RoomManager::singleton->getWindowAspectRatio();
	auto& layer = RoomManager::singleton->mGoLayers[mParentIndex];
	layer.cameraEye = mPosition + Vector3(0.0f, 0.0f, 10.5f * ar);
	layer.cameraTarget = mPosition;
}

void Logo::continueLogic()
{
	// Clear current layer
	for (auto& item : *RoomManager::singleton->mGoLayers[mParentIndex].list)
	{
		item->mDestroyMe = true;
	}

	// Check if safe minigame is required
	if (mSafeMinigame)
	{
		// Create game object
		const std::shared_ptr<SafeMinigame> sm = std::make_shared<SafeMinigame>(mParentIndex);
		sm->setupCamera();
		RoomManager::singleton->mGoLayers[mParentIndex].push_back(sm);
	}
}
