#include "Player.h"

#include <Magnum/Math/Angle.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "AssetManager.h"
#include "InputManager.h"
#include "RoomManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

Player::Player() : GameObject()
{
	// Load asset
	AssetManager::singleton->loadAssets(*this, "scenes/test.glb");

	// Set diffuse color
	mDiffuseColor = 0xffffff_rgbf;
	mAmbientColor = 0xff0000_rgbf;
}

void Player::update()
{
	// Check for mouse input
	Vector2 p1 = Vector2(InputManager::singleton->mMousePosition);
	Vector2 p2 = Vector2({ RoomManager::singleton->windowSize.x() * 0.5f, Float(RoomManager::singleton->windowSize.y()) });
	Vector2 pdir = p2 - p1;
	Math::Unit<Math::Rad, Float> rads(std::atan2(pdir.y(), pdir.x()));
	Math::Deg<Float> degs(rads);
	printf("Angle to shoot is %f degrees (OpenGL directions)\n", Float(degs));

	// Apply transformations to all drawables for this instance
	const auto& m = Matrix4::translation(position);
	for (const auto& d : drawables)
	{
		d->setTransformation(m);
	}
}

void Player::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	CORRADE_ASSERT(false, "The draw method for Player class must not be called.");
}