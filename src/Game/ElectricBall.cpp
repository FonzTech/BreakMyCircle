#include "ElectricBall.h"

#include "Bubble.h"
#include "../Common/CommonUtility.h"
#include "../Graphics/GameDrawable.h"
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
			mPieces[i].manipulator = new Object3D{ mManipulator.get() };
			mPieces[i].angleCurrent = Deg(2.0f);
			mPieces[i].angleLimit = Deg(1.0f);

			std::shared_ptr<GameDrawable<SpriteShader>> td = std::static_pointer_cast<GameDrawable<SpriteShader>>(CommonUtility::singleton->createSpriteDrawable(mParentIndex, *mPieces[i].manipulator, resTexture, this));
			mDrawables.emplace_back(td);
		}
	}

	// Create lightorb
	{
		// Load assets
		Resource<GL::Mesh> resMesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<Shaders::Flat3D::Position, Shaders::Flat3D::TextureCoordinates>(RESOURCE_MESH_PLANE_FLAT);
		Resource<GL::Texture2D> resTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_LIGHTORB);
		Resource<GL::AbstractShaderProgram, Shaders::Flat3D> resShader = CommonUtility::singleton->getFlat3DShader();

		// Create child manipulator
		mOrbManipulator = new Object3D{ mManipulator.get() };

		// Create drawable
		auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
		std::shared_ptr<GameDrawable<Shaders::Flat3D>> d = std::make_shared<GameDrawable<Shaders::Flat3D>>(*drawables, resShader, resMesh, resTexture);
		d->setParent(mOrbManipulator);
		d->setDrawCallback(this);
		mDrawables.emplace_back(d);
	}

	// Create shader data wrapper
	mWrapper.parameters.index = 0.0f;
	mWrapper.parameters.total = 8.0f;
	mWrapper.parameters.rows = 1.0f;
	mWrapper.parameters.columns = 8.0f;
	mWrapper.speed = 12.0f;

	// Load audio
	{
		Resource<Audio::Buffer> buffer = CommonUtility::singleton->loadAudioData(RESOURCE_AUDIO_ELECTRIC);
		mPlayables[0] = std::make_shared<Audio::Playable3D>(*mManipulator.get(), &RoomManager::singleton->mAudioPlayables);
		mPlayables[0]->source()
			.setBuffer(buffer)
			.setLooping(true);
	}
}

const Int ElectricBall::getType() const
{
	return GOT_ELECTRIC_BALL;
}

void ElectricBall::update()
{
	// Apply transformations
	(*mOrbManipulator)
		.resetTransformation()
		.scale(Vector3(3.0f, 3.0f, 1.0f))
		.rotateZ(Deg(Math::floor(mWrapper.parameters.index) * 45.0f))
		.translate(Vector3(0.0f, 0.0f, 0.6f));

	(*mManipulator)
		.resetTransformation()
		.translate(mPosition);

	for (UnsignedInt i = 0; i < 4; ++i)
	{
		// Advance animation
		mPieces[i].angleCurrent += Deg(mDeltaTime * 8.0f) + Deg(std::rand() % 10 > 8 ? 3.0f : 0.0f);

		// Check for animation end
		if (mPieces[i].angleCurrent > mPieces[i].angleLimit)
		{
			const Deg angle(Float(std::rand() % 15) - 15.0f);
			mPieces[i].angleCurrent = angle + Deg(Float(i) * 90.0f);
			mPieces[i].angleLimit = mPieces[i].angleCurrent + Deg(15.0f + Float(std::rand() % 15));
		}

		const Rad rads(mPieces[i].angleCurrent - Deg(90.0f));
		(*mPieces[i].manipulator)
			.resetTransformation()
			.scale(Vector3(2.0f, 20.0f, 1.0f))
			.rotateZ(mPieces[i].angleCurrent)
			.translate(Vector3(Math::cos(rads) * -21.0f, Math::sin(rads) * -21.0f, 0.3f));
	}

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
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.setColor(0xffffffff_rgbaf)
			.setIndex(mWrapper.parameters.index)
			.setRows(mWrapper.parameters.rows)
			.setColumns(mWrapper.parameters.columns)
			.draw(*baseDrawable->mMesh);
	}
}