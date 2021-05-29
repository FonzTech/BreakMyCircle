#include "Player.h"

#include <Magnum/Math/Angle.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "AssetManager.h"
#include "InputManager.h"
#include "RoomManager.h"
#include "Projectile.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

Player::Player() : GameObject()
{
	// Load asset
	AssetManager::singleton->loadAssets(*this, "scenes/cannon_1.glb");

	// Set diffuse color
	mDiffuseColor = 0xffffff_rgbf;
	mAmbientColor = 0xff0000_rgbf;
}

Int Player::getType()
{
	return GOT_PLAYER;
}

void Player::update()
{
	// Check for mouse input
	Vector2 p1 = Vector2(InputManager::singleton->mMousePosition);
	Vector2 p2 = Vector2({ RoomManager::singleton->windowSize.x() * 0.5f, Float(RoomManager::singleton->windowSize.y()) });
	Vector2 pdir = p2 - p1;
	Math::Unit<Math::Rad, Float> unitRads(std::atan2(pdir.y(), pdir.x()));
	Float rads(unitRads);

	if (InputManager::singleton->mMouseStates[ImMouseButtons::Left] == IM_STATE_PRESSED)
	{
		Color3 bc = 0xc00000_rgbf;
		std::shared_ptr<Projectile> go = std::make_shared<Projectile>(bc);
		go->position = position;
		go->mVelocity = { -std::cos(rads), std::sin(rads), 0.0f };
		RoomManager::singleton->mGameObjects.push_back(go);
	}

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

void Player::collidedWith(GameObject* gameObject)
{
}