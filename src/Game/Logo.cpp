#include "Logo.h"

#include <Corrade/Containers/StridedArrayView.h>
#include <Magnum/Animation/Easing.h>
#include <Magnum/Resource.h>
#include <Magnum/Math/Math.h>
#include <Magnum/GL/Mesh.h>

#include "../AssetManager.h"
#include "../RoomManager.h"
#include "../Common/CommonUtility.h"
#include "../Graphics/BaseDrawable.h"
#include "Bubble.h"
#include "FallingBubble.h"

#if NDEBUG or _DEBUG
#include "../InputManager.h"
#endif

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
	
	// Init members
	mLightPosition = Vector3(0.0f, -0.5f, 0.0f);
	mLightDirection = false;
	mIntroBubbles = true;
	mLogoZoom = 0.0f;
	mAnimElapsed = -3.001f; // Cycle waste

	// Load assets
	mPosition = Vector3(0.0f, 10.0f, 0.0f);
	mManipulator->setTransformation(Matrix4::translation(mPosition));

	mLogoManipulator = new Object3D(mManipulator.get());
	AssetManager().loadAssets(*this, *mLogoManipulator, RESOURCE_SCENE_LOGO, this);

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
	mFinishTimer = FINISH_TIMER_STARTING_VALUE;

	// Set camera parameters
	setCameraParameters();

	// Create overlay text
	{
		const std::string& text = "Tap here to begin";
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::MiddleCenter, UnsignedInt(text.length()));
		go->mPosition = Vector3(0.0f, -0.25f, 0.0f);
		go->mColor.data()[3] = 0.0f;
		go->mOutlineColor.data()[3] = 0.0f;
		go->setText(text);

		mTexts[0] = (std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(go, true);
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
	// Advance animation
	if (mAnimElapsed < 0.0f) // Cycle waste
	{
		mAnimElapsed += 1.0f;
		return;
	}
	else
	{
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
		// Create bubble of random color
		const auto& bc = RoomManager::singleton->mBubbleColors;
		while (true)
		{
			// Get random color (no special objects, like coins)
			const auto& it = std::next(std::begin(bc), std::rand() % bc.size());
			if (it->second.color == BUBBLE_COIN)
			{
				continue;
			}

			// Create random bubble
			std::shared_ptr<FallingBubble> fb = std::make_shared<FallingBubble>(mParentIndex, it->second.color, false, -25.0f);
			fb->mPosition = mPosition;
			fb->mPosition -= RoomManager::singleton->mGoLayers[mParentIndex].cameraEye;
			fb->mPosition += Vector3(-6.0f + 12.0f * (std::rand() % 12) / 12.0f, 20.0, 0.0f);
			RoomManager::singleton->mGoLayers[mParentIndex].push_back(fb);

			// Break from cycle
			break;
		}

		// Reset timer
		mBubbleTimer = Float(std::rand() % 100) * 0.001f + 0.25f;
	}

	// Check for finish timer
	if (mFinishTimer < -1.0f)
	{
		// Handle "tap here"
		{
			if (InputManager::singleton->mMouseStates[ImMouseButtons::Left] == IM_STATE_RELEASED)
			{
				mIntroBubbles = false;
			}

			// Handle its text
			for (UnsignedInt i = 0; i < 2; ++i)
			{
				auto* p = &(i ? mTexts[0]->mOutlineColor : mTexts[0]->mColor).data()[3];

				if (mIntroBubbles)
				{
					*p += mDeltaTime;
				}
				else
				{
					*p -= mDeltaTime;
					mLogoZoom += mDeltaTime;

					for (UnsignedInt i = 0; i < 3; ++i)
					{
						mLogoObjects[i]->translate(Vector3(0.0f, 0.0f, Math::sin(Rad(mLogoZoom)) * -0.06f));
					}
				}

				if (*p > 1.0f)
				{
					*p = 1.0f;
				}
				else if (*p < -0.15f)
				{
					for (auto& item : *RoomManager::singleton->mGoLayers[mParentIndex].list)
					{
						item->mDestroyMe = true;
					}
					return;
				}
			}
		}

		// Handle logo light effect
		mLightPosition += Vector3(0.0f, mLightDirection ? -4.0f : 4.0f, 0.0f) * mDeltaTime;

		if (mLightPosition.y() > 11.0f)
		{
			mLightDirection = true;
		}
		else if (mLightPosition.y() < -1.0f)
		{
			mLightDirection = false;
		}
	}
	else
	{
		mFinishTimer -= mDeltaTime;
	}
}

void Logo::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	((Shaders::Phong&) baseDrawable->getShader())
		.setLightPosition(mLightPosition)
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

void Logo::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
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
			mKeyframes[i][2] = { 2.0f, Vector3(0.0f, 0.0f, 0.0f), 0.0_degf };
			break;

		case 1:
			mKeyframes[i][0] = { 0.0f, Vector3(0.0f, 10.0f, 0.0f), 360.0_degf * 4 };
			mKeyframes[i][1] = { 1.5f, Vector3(0.0f, 10.0f, 0.0f), 360.0_degf * 4 };
			mKeyframes[i][2] = { 3.5f, Vector3(0.0f, 0.0f, 0.0f), 0.0_degf };
			break;

		case 2:
			mKeyframes[i][0] = { 0.0f, Vector3(0.0f, 0.0f, -10.0f), 360.0_degf * 2 };
			mKeyframes[i][1] = { 3.0f, Vector3(0.0f, 0.0f, -10.0f), 360.0_degf * 2 };
			mKeyframes[i][2] = { 5.0f, Vector3(0.0f, 0.0f, 0.0f), 0.0_degf };
			break;

		case 3:
			mKeyframes[i][0] = { 0.0f, Vector3(0.0f, 0.0f, 0.0f), 0.0_degf };
			mKeyframes[i][1] = { 6.0f, Vector3(0.0f, 0.0f, 0.0f), 0.0_degf };
			mKeyframes[i][2] = { 7.0f, Vector3(0.0f, -2.0f, 0.0f), 0.0_degf };
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