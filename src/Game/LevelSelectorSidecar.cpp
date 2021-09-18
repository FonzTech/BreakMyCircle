#include "LevelSelectorSidecar.h"

#include "../AssetManager.h"
#include "../RoomManager.h"
#include "../Common/CommonUtility.h"

using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> LevelSelectorSidecar::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate bubble
	std::shared_ptr<LevelSelectorSidecar> p = std::make_shared<LevelSelectorSidecar>(parent, 0U);
	return p;
}

LevelSelectorSidecar::LevelSelectorSidecar(const Int parentIndex, const UnsignedInt levelIndex) : GameObject(parentIndex), mLevelIndex(levelIndex), mScale(1.0f)
{
	// Assign members
	mParentIndex = parentIndex;
	mGlowManipulator = nullptr;

	// Load drawables
	AssetManager am(RESOURCE_SHADER_COLORED_PHONG, RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE, 1);
	am.loadAssets(*this, *mManipulator, RESOURCE_SCENE_LEVEL_BUTTON, this);
}

LevelSelectorSidecar::~LevelSelectorSidecar()
{
}

const Int LevelSelectorSidecar::getType() const
{
	return GOT_LEVEL_SELECTOR_SIDECAR;
}

void LevelSelectorSidecar::update()
{
	(*mManipulator)
		.resetTransformation()
		.scale(mScale)
		.translate(mPosition);
}

void LevelSelectorSidecar::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (mGlowManipulator != nullptr && baseDrawable->mMesh->label() == "GlowV")
	{
		(*mFlat3DShader)
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindTexture(*baseDrawable->mTexture)
			.setColor(Color4{ 1.0f })
			.setAlphaMask(0.001f)
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		const auto& color = mLevelIndex < RoomManager::singleton->mSaveData.maxLevelId ? 0xc0c0c0_rgbf : 0x404040_rgbf;
		((Shaders::Phong&)baseDrawable->getShader())
			.setLightPosition(camera.cameraMatrix().transformPoint(mPosition + Vector3(0.0f, 6.0f, 0.0f)))
			.setLightColor(0xc0c0c0_rgbf)
			.setSpecularColor(0x000000_rgbf)
			.setDiffuseColor(color)
			.setAmbientColor(color)
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
			.setObjectId(baseDrawable->getObjectId())
			.draw(*baseDrawable->mMesh);
	}
}

void LevelSelectorSidecar::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void LevelSelectorSidecar::setScale(const Vector3 & scale)
{
	mScale = scale;
}

void LevelSelectorSidecar::setParameters(Resource<GL::Texture2D> & texture, UnsignedInt objectId)
{
	// Apply custom texture
	for (auto& drawable : mDrawables)
	{
		// Set the correct texture for the "platform" mesh
		if (drawable->mMesh->label() == GO_LS_MESH_PLATFORM)
		{
			drawable->mTexture = texture;
		}

		// Set various properties
		drawable->setObjectId(objectId);
	}
}

void LevelSelectorSidecar::setGlow(const bool enabled)
{
	if (enabled)
	{
		if (mGlowManipulator != nullptr)
		{
			mGlowManipulator->scale(Vector3(1.0f));
			mGlowDrawable.lock()->pushToFront();
		}
		else
		{
			mFlat3DShader = CommonUtility::singleton->getFlat3DShader();
			mGlowManipulator = new Object3D{ mManipulator.get() };

			AssetManager().loadAssets(*this, *mGlowManipulator, RESOURCE_SCENE_LEVEL_GLOW, this);
			mGlowDrawable = *(mDrawables.end() - 1);
		}
	}
	else if (mGlowManipulator != nullptr)
	{
		mGlowManipulator->scale(Vector3(0.0f));
	}
}