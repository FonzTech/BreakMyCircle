#include "Logo.h"

#include <Corrade/Containers/StridedArrayView.h>
#include <Magnum/Animation/Easing.h>
#include <Magnum/Resource.h>
#include <Magnum/Math/Math.h>
#include <Magnum/GL/Mesh.h>

#include "AssetManager.h"
#include "RoomManager.h"
#include "BaseDrawable.h"
#include "FallingBubble.h"

#if NDEBUG or _DEBUG
#include "InputManager.h"
#endif

using namespace Corrade;
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
	mLogoManipulator = new Object3D(mManipulator.get());
	AssetManager().loadAssets(*this, *mLogoManipulator, "scenes/logo.glb", this);

	// Filter required meshes
	const std::unordered_map<std::string, Uint8> indexes{
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
		}
	}

	// Build animations
	buildAnimations();

	// Init timers
	mBubbleTimer = 0.0f;
	mFinishTimer = 6.0f;

	// Set camera parameters
	setCameraParameters();
}

const Int Logo::getType() const
{
	return GOT_LOGO;
}

void Logo::update()
{
	// Advance animation
	mAnimPlayer->advance(mAnimTimeline.previousFrameTime());
	mAnimTimeline.nextFrame();

	// Check for bubble timer
	if (mBubbleTimer > 0.0f)
	{
		mBubbleTimer -= mDeltaTime;
	}
	else if (mFinishTimer > 0.0f)
	{
		// Get random color
		const auto& bc = RoomManager::singleton->mBubbleColors;
		const auto& it = std::next(std::begin(bc), std::rand() % bc.size());

		// Get random position
		Vector3 rp; 
		rp[0] = -6.0f + 12.0f * (std::rand() % 12) / 12.0f;
		rp[1] = 20.0f;
		rp[2] = 0.0f;

		// Create random bubble
		std::shared_ptr<FallingBubble> fb = std::make_shared<FallingBubble>(GOL_FIRST, it->second.color, false, -25.0f);
		fb->position = position + rp;
		RoomManager::singleton->mGoLayers[mParentIndex].push_back(fb);

		// Reset timer
		mBubbleTimer = Float(std::rand() % 100) * 0.001f + 0.25f;
	}

	// Check for finish timer
	if (mFinishTimer < 0.0f)
	{
		if (false)
		{
			RoomManager::singleton->prepareRoom();
			RoomManager::singleton->createLevelRoom();
		}
	}
	else
	{
		mFinishTimer -= mDeltaTime;
	}

#if NDEBUG or _DEBUG
	if (InputManager::singleton->mMouseStates[ImMouseButtons::Right] == IM_STATE_RELEASED)
	{
		RoomManager::singleton->prepareRoom();
		RoomManager::singleton->createLevelRoom();
	}
#endif
}

void Logo::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	((Shaders::Phong&) baseDrawable->getShader())
		.setLightPosition(position + Vector3(0.0f, 0.0f, 1.0f))
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
	for (Uint8 i = 0; i < 4; ++i)
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
				object.setTransformation(Matrix4());
				object.rotate(tr, Vector3::yAxis());
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
			for (Uint8 j = 0; j < 3; ++j)
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
	mAnimTimeline.start();
	mAnimPlayer->play(mAnimTimeline.previousFrameTime());
}

void Logo::setCameraParameters()
{
	// First layer
	{
		auto& layer = RoomManager::singleton->mGoLayers[GOL_FIRST];
		layer.mCameraEye = position + Vector3(0.0f, 0.0f, 20.0f);
		layer.mCameraTarget = position;
	}

	// Second layer
	{
		auto& layer = RoomManager::singleton->mGoLayers[GOL_SECOND];
		layer.mCameraEye = position + Vector3(0.0f, 0.0f, 6.0f);
		layer.mCameraTarget = position;
	}
}