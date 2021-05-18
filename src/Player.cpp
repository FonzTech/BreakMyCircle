#include "Player.h"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "AssetManager.h"
#include "ColoredDrawable.h"

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
}

void Player::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	CORRADE_ASSERT(false, "The draw method for Player class must not be called.");
}