#include "Player.h"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "AssetManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

Player::Player(SceneGraph::DrawableGroup3D& group) : GameObject()
{
	AssetManager::singleton->loadAssets(*this, "scenes/test.glb");

	// Set diffuse color
	mDiffuseColor = 0xffffff_rgbf;
	mAmbientColor = 0xff0000_rgbf;
}

void Player::update()
{
}

/*
void Player::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	mShader.setLightPositions({ position })
		.setDiffuseColor(mDiffuseColor)
		.setAmbientColor(mAmbientColor)
		.setTransformationMatrix(transformationMatrix * Matrix4::translation(position))
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.draw(mMesh);
}
		*/