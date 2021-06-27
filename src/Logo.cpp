#include "Logo.h"

#include <Corrade/Containers/StridedArrayView.h>
#include <Magnum/Animation/Easing.h>
#include <Magnum/Resource.h>
#include <Magnum/Math/Math.h>
#include <Magnum/GL/Mesh.h>

#include "AssetManager.h"
#include "RoomManager.h"
#include "BaseDrawable.h"

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
}

const Int Logo::getType() const
{
	return GOT_LOGO;
}

void Logo::update()
{
	// Set camera
	auto& layer = RoomManager::singleton->mGoLayers[GOL_MAIN];
	layer.mCameraEye = position + Vector3(0.0f, 0.0f, 6.0f);
	layer.mCameraTarget = position;

	// Advance animation
	mAnimPlayer->advance(mAnimTimeline.previousFrameTime());
	mAnimTimeline.nextFrame();
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
	for (Uint8 i = 0; i < 3; ++i)
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
		}

		auto as = Containers::arraySize(mKeyframes);
		auto at = Containers::StridedArrayView1D<Float>{ mKeyframes, &mKeyframes[i][0].time, as, sizeof(Keyframe) };

		// Track view for positions
		mTrackViewPositions[i] = std::make_unique<AnimPosition>(
			at,
			Containers::StridedArrayView1D<Vector3>{ mKeyframes, &mKeyframes[i][0].position, as, sizeof(Keyframe) },
			Animation::ease<Vector3, Math::lerp, Animation::Easing::quadraticOut>()
		);

		// Track view for rotations
		mTrackViewRotations[i] = std::make_unique<AnimRotation>(
			at,
			Containers::StridedArrayView1D<Deg>{ mKeyframes, &mKeyframes[i][0].rotation, as, sizeof(Keyframe) },
			Animation::ease<Deg, Math::lerp, Animation::Easing::quadraticOut>()
		);

		// Animations for both positions and rotations
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

	// Start animation
	mAnimTimeline.start();
	mAnimPlayer->play(mAnimTimeline.previousFrameTime());
}