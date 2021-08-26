#include "ElectricBall.h"

#include "Bubble.h"
#include "../Common/CommonUtility.h"
#include "../RoomManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> ElectricBall::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate bubble
	std::shared_ptr<ElectricBall> p = std::make_shared<ElectricBall>(parent);
	return p;
}

ElectricBall::ElectricBall(const Int parentIndex) : GameObject(parentIndex)
{
	// Assign members
	mParentIndex = parentIndex;

	// Create sphere
	{
		// Load asset
		CommonUtility::singleton->createGameSphere(this, *mManipulator, BUBBLE_ELECTRIC);
	}

	// Create lightnings
	{
		Resource<GL::Texture2D> resTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_LIGHTNING);

		for (UnsignedInt i = 0; i < 4; ++i)
		{
			mListManipulators[i] = new Object3D{ mManipulator.get() };

			std::shared_ptr<GameDrawable<SpriteShader>> td = CommonUtility::singleton->createSpriteDrawable(mParentIndex, *mListManipulators[i], resTexture, this);
			mDrawables.emplace_back(td);

			switch (i)
			{
			case 0:
				(*mListManipulators[i])
					.scale(Vector3(2.0f, 20.0f, 1.0f))
					.translate(Vector3(0.0f, -21.0f, 0.8f));
				break;

			case 1:
				(*mListManipulators[i])
					.scale(Vector3(2.0f, 20.0f, 1.0f))
					.translate(Vector3(0.0f, 21.0f, 0.8f));
				break;

			case 2:
				(*mListManipulators[i])
					.scale(Vector3(2.0f, 20.0f, 1.0f))
					.rotateZ(Deg(-90.0f))
					.translate(Vector3(-21.0f, 0.0f, 0.8f));
				break;

			case 3:
				(*mListManipulators[i])
					.scale(Vector3(2.0f, 20.0f, 1.0f))
					.rotateZ(Deg(-90.0f))
					.translate(Vector3(21.0f, 0.0f, 0.8f));
				break;
			}
		}
	}

	// Create lightorb 
	{
		// Load assets
		Resource<GL::Mesh> resMesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<Shaders::Flat3D>(RESOURCE_MESH_PLANE_FLAT);
		Resource<GL::Texture2D> resTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_LIGHTORB);
		Resource<GL::AbstractShaderProgram, Shaders::Flat3D> resShader = CommonUtility::singleton->getFlat3DShader();

		// Create child manipulator
		mListManipulators[4] = new Object3D{ mManipulator.get() };

		// Create drawable
		auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
		std::shared_ptr<GameDrawable<Shaders::Flat3D>> d = std::make_shared<GameDrawable<Shaders::Flat3D>>(*drawables, resShader, resMesh, resTexture);
		d->setParent(mListManipulators[4]);
		d->setDrawCallback(this);
		mDrawables.emplace_back(d);
	}

	// Create shader data wrapper
	mWrapper.parameters.index = 0.0f;
	mWrapper.parameters.total = 8.0f;
	mWrapper.parameters.rows = 1.0f;
	mWrapper.parameters.columns = 8.0f;
	mWrapper.speed = 12.0f;
}

const Int ElectricBall::getType() const
{
	return GOT_ELECTRIC_BALL;
}

void ElectricBall::update()
{
	// Apply transformations
	(*mListManipulators[4])
		.resetTransformation()
		.scale(Vector3(4.0f, 4.0f, 1.0f))
		.translate(Vector3(0.0f, 0.0f, 0.4f));

	(*mManipulator)
		.resetTransformation()
		.translate(mPosition);

	// Manage sprite animation
	mWrapper.parameters.index += mDeltaTime * mWrapper.speed;
	while (mWrapper.parameters.index >= mWrapper.parameters.total)
	{
		mWrapper.parameters.index -= mWrapper.parameters.total;
	}
}

void ElectricBall::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (baseDrawable == mDrawables[0].get())
	{
		((Shaders::Phong&) baseDrawable->getShader())
			.setLightPosition(mPosition)
			.setLightColor(0x808080_rgbf)
			.setSpecularColor(0xffffff00_rgbaf)
			.setAmbientColor(0xc0c0c0_rgbf)
			.setDiffuseColor(0x808080_rgbf)
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
			.draw(*baseDrawable->mMesh);
	}
	else if (baseDrawable == mDrawables[5].get())
	{
		((Shaders::Flat3D&) baseDrawable->getShader())
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindTexture(*baseDrawable->mTexture)
			.setColor(0xffffffff_rgbaf)
			.setAlphaMask(0.001f)
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		((SpriteShader&)baseDrawable->getShader())
			.bindTexture(*baseDrawable->mTexture)
			.setTransformationMatrix(transformationMatrix)
			.setProjectionMatrix(camera.projectionMatrix())
			.setColor(0xffffffff_rgbaf)
			.setIndex(mWrapper.parameters.index)
			.setRows(mWrapper.parameters.rows)
			.setColumns(mWrapper.parameters.columns)
			.draw(*baseDrawable->mMesh);
	}
}

void ElectricBall::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}