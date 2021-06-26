#include "Logo.h"

#include <Corrade/Containers/StridedArrayView.h>
#include <Magnum/Math/Math.h>

#include "AssetManager.h"
#include "RoomManager.h"

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
	AssetManager().loadAssets(*this, *mManipulator, "scenes/logo.glb", this);

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
	// Create raw animation data
	mKeyframes[0] = { 0.0f, Vector3(3.0f, 3.0f, 0.0f), 360.0_degf };
	mKeyframes[1] = { 3.0f, Vector3(0.0f, 0.0f, 0.0f), 0.0_degf };

	auto as = Containers::arraySize(mKeyframes);
	auto at = Containers::StridedArrayView1D<Float>{ mKeyframes, &mKeyframes[0].time, as, sizeof(Keyframe) };

	// Track view for positions
	mTrackViewPositions = std::make_unique<AnimPosition>(
		at,
		Containers::StridedArrayView1D<Vector3>{ mKeyframes, &mKeyframes[0].position, as, sizeof(Keyframe) },
		Animation::Interpolation::Linear
	);

	// Track view for rotations
	mTrackViewRotations = std::make_unique<AnimRotation>(
		at,
		Containers::StridedArrayView1D<Deg>{ mKeyframes, &mKeyframes[0].rotation, as, sizeof(Keyframe) },
		Animation::Interpolation::Linear
	);

	// Animations for both positions and rotations
	mAnimPlayer = std::make_unique<Animation::Player<Float>>();
	mAnimPlayer->addWithCallback(
		*mTrackViewRotations,
		[](Float, const Deg& tr, Object3D& object) {
			object.setTransformation(Matrix4());
			object.rotate(tr, Vector3::yAxis());
		},
		*((Object3D*)mDrawables.at(0).get())
		);
	mAnimPlayer->addWithCallback(
		*mTrackViewPositions,
		[](Float, const Vector3& tp, Object3D& object) {
			object.translate(tp);
		},
		*((Object3D*)mDrawables.at(0).get())
	);

	// Start animation
	mAnimTimeline.start();
	mAnimPlayer->play(mAnimTimeline.previousFrameTime());
}