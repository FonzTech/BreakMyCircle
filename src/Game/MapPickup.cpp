#include "MapPickup.h"
#include "../AssetManager.h"

#include <Magnum/Animation/Easing.h>

using namespace Magnum;
using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> MapPickup::getInstance(const nlohmann::json & params)
{
	// Get required parameters
	Int parent, customType;
	params.at("parent").get_to(parent);
	params.at("customType").get_to(customType);

	// Instantiate player object
	std::shared_ptr<MapPickup> p = std::make_shared<MapPickup>(parent, customType);
	return p;
}

MapPickup::MapPickup(const Int parentIndex, const Int customType) : GameObject(parentIndex), mAnimation(0.0f), mPickupDestroy(0.0f), mAmbientColor(0x505050_rgbf), mCustomType(customType)
{
	// Assign properties
	mParentIndex = parentIndex;

	// Custom type
	switch (mCustomType)
	{
	case GO_MP_TYPE_COIN:
		AssetManager am(RESOURCE_SHADER_COLORED_PHONG, RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE, 1);
		am.loadAssets(*this, *mManipulator, RESOURCE_SCENE_COIN, this);
		break;
	}
}

MapPickup::~MapPickup()
{
}

const Int MapPickup::getType() const
{
	return GOT_MAPPICKUP;
}

void MapPickup::update()
{
	// Check for pickup destroy
	if (mPickupDestroy > 0.0f)
	{
		mPickupDestroy += mDeltaTime;

		if (mPickupDestroy >= 1.0f)
		{
			mDestroyMe = true;
		}
	}

	// Advance animation
	mAnimation += mDeltaTime;

	// Apply transformations
	switch (mCustomType)
	{
	case GO_MP_TYPE_COIN:
		const auto& np = mAnimation < 2.0f ? Math::max(0.0f, Animation::Easing::circularIn(1.0f - mAnimation * 0.5f) * 10.0f) : 0.0f;
		const auto& yp = mPickupDestroy < 1.0f ? Math::lerp(0.0f, 1.0f, Animation::Easing::circularIn(mPickupDestroy)) : 1.0f;
		(*mManipulator)
			.resetTransformation()
			.rotateX(Deg(90.0f))
			.rotateY(Deg(mAnimation * 75.0f) + Deg(mPickupDestroy * 360.0f))
			.translate(mPosition + Vector3(0.0f, np + yp * 10.0f, 0.0f));
		break;
	}
}

void MapPickup::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	((Shaders::Phong&) baseDrawable->getShader())
		.setLightPosition(Vector3(0.5f, 0.5f, 0.5f))
		.setLightColor(0xffffff_rgbf)
		.setSpecularColor(0xffffff_rgbf)
		.setAmbientColor(mAmbientColor)
		.setDiffuseColor(0xffffff_rgbf)
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
		.setObjectId(baseDrawable->getObjectId())
		.setAlphaMask(0.001f)
		.draw(*baseDrawable->mMesh);
}

void MapPickup::setObjectId(const UnsignedInt id)
{
	for (auto& item : mDrawables)
	{
		item->setObjectId(id);
	}
}

bool MapPickup::isPickupable()
{
	return mPickupDestroy <= 0.0f;
}

void MapPickup::setDestroyState(const bool state)
{
	if (state)
	{
		if (mPickupDestroy <= 0.0f)
		{
			mPickupDestroy = 0.001f;
		}
	}
	else
	{
		mPickupDestroy = 0.0f;
	}
}