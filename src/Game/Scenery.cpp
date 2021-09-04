#include "Scenery.h"

#include <Corrade/Containers/LinkedList.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/Primitives/Grid.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "../Common/CommonUtility.h"
#include "../AssetManager.h"
#include "../RoomManager.h"
#include "../InputManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> Scenery::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Get required data
	Int modelIndex;
	params.at("modelIndex").get_to(modelIndex);

	// Instantiate scenery object
	std::shared_ptr<Scenery> p = std::make_shared<Scenery>(parent, modelIndex);
	return p;
}

Scenery::Scenery(const Int parentIndex, const Int modelIndex) : GameObject(parentIndex)
{
	// Init members
	// mCubicBezier = std::make_unique<CubicBezier2D>(Vector2(0.0f, 0.0f), Vector2(0.11f, -0.02f), Vector2(0.0f, 1.01f), Vector2(1.0f));
	mParentIndex = parentIndex;
	mModelIndex = modelIndex;
	mLightPosition = Vector3(0.0f);
	mFrame = 0.0f;

	// Fill manipulator list
	mManipulatorList.push_back(new Object3D{ mManipulator.get() });
	mManipulatorList.push_back(new Object3D{ mManipulator.get() });
	mManipulatorList.push_back(new Object3D{ mManipulator.get() });

	// Apply transformations
	(*mManipulatorList[0])
		.resetTransformation()
		.translate(mPosition);

	// Load assets
	AssetManager am(RESOURCE_SHADER_COLORED_PHONG, RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE, 1);

	{
		std::string rk;
		switch (modelIndex)
		{
		case 0:
			(*mManipulatorList[1])
				.resetTransformation()
				.scale(Vector3(50.0f, 25.0f, 1.0f))
				.rotateX(90.0_degf)
				.translate(mPosition + Vector3(0.0f, 0.3f, 0.0f));

			createWaterDrawable();
			rk = RESOURCE_SCENE_WORLD_1;
			break;

		case 1:
			mManipulatorList[0]->translate(Vector3(0.0f, 0.3f, 0.0f));
			rk = RESOURCE_SCENE_WORLD_2;
			break;

		case 2:
			mManipulatorList[0]->translate(Vector3(0.0f, 0.3f, 0.0f));
			rk = RESOURCE_SCENE_WORLD_3;
			break;
		}

		am.loadAssets(*this, *mManipulatorList[0], rk, this);
	}

	// Load world wall
	{
		(*mManipulatorList[2])
			.resetTransformation()
			.translate(mPosition + Vector3(0.0f, 0.6f, -25.0f));

		am.loadAssets(*this, *mManipulatorList[2], RESOURCE_SCENE_WORLD_WALL, this);
	}

	/*
	{
		auto& p = RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST];
		p.cameraEye = Vector3(7.65094f, 11.6036f, 11.9944f);
		p.cameraTarget = Vector3(0.0f, 0.0f, 0.0f);
	}
	*/
}

Scenery::~Scenery()
{
	Debug{} << "Scenery with model index" << mModelIndex << "is destructed";
}

const Int Scenery::getType() const
{
	return GOT_SCENERY;
}

void Scenery::update()
{
	// Update frame
	mFrame += mDeltaTime;

	for (auto& wh : mWaterHolders)
	{
		wh.second.parameters.frame = mFrame;
	}

	// Debug camera move
#ifdef _DEBUG or NDEBUG
	{
		Vector3 delta;

		if (InputManager::singleton->mKeyStates[ImKeyButtons::Left] >= IM_STATE_PRESSED)
		{
			delta += Vector3(-1.0f, 0.0f, 0.0f);
		}

		if (InputManager::singleton->mKeyStates[ImKeyButtons::Right] >= IM_STATE_PRESSED)
		{
			delta += Vector3(1.0f, 0.0f, 0.0f);
		}

		if (InputManager::singleton->mKeyStates[ImKeyButtons::Up] >= IM_STATE_PRESSED)
		{
			delta += Vector3(0.0f, 1.0f, 0.0f);
		}

		if (InputManager::singleton->mKeyStates[ImKeyButtons::Down] >= IM_STATE_PRESSED)
		{
			delta += Vector3(0.0f, -1.0f, 0.0f);
		}

		if (InputManager::singleton->mKeyStates[ImKeyButtons::LeftShift] >= IM_STATE_PRESSED)
		{
			delta += Vector3(0.0f, 0.0f, -1.0f);
		}

		if (InputManager::singleton->mKeyStates[ImKeyButtons::LeftCtrl] >= IM_STATE_PRESSED)
		{
			delta += Vector3(0.0f, 0.0f, 1.0f);
		}

		const bool isEye = InputManager::singleton->mKeyStates[ImKeyButtons::Tab] >= IM_STATE_PRESSED;

		auto& p1 = RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST];
		auto* p2 = isEye ? &p1.cameraEye : &p1.cameraTarget;
		*p2 += delta * mDeltaTime * 10.0f;
	}
#endif

	/*
	// Animation for eye camera
	if (mAnimateInGameCamera)
	{
		auto* p = &RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].cameraEye[2];
		*p = mCubicBezier->value(Math::min(mFrame * 0.25f, 1.0f))[1] * 44.0f;
	}
	*/
}

void Scenery::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	const auto& it = mWaterHolders.find(baseDrawable);
	if (it != mWaterHolders.end())
	{
		((WaterShader&)baseDrawable->getShader())
			.setTransformationMatrix(transformationMatrix)
			.setProjectionMatrix(camera.projectionMatrix())
			.setFrame(it->second.parameters.frame)
			.setSpeed(it->second.parameters.speed)
			.setSize(it->second.parameters.size)
			.setHorizonColorUniform(it->second.parameters.horizonColor)
			.bindDisplacementTexture(*it->second.parameters.displacementTexture)
			.bindWaterTexture(*it->second.parameters.waterTexture)
			.bindEffectsTexture(*it->second.parameters.effectsTexture)
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		// Draw through shader
		auto& shader = (Shaders::Phong&) baseDrawable->getShader();

		shader
			.setLightPosition(Vector3(0.0f, 6.0f, 5.0f) - mLightPosition)
			.setLightColor(0xc0c0c000_rgbaf)
			.setSpecularColor(0xc0c0c000_rgbaf)
			.setDiffuseColor(0x808080ff_rgbaf)
			.setAmbientColor(0xffffffff_rgbaf)
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.setObjectId(0U);

		if (baseDrawable->mTexture != nullptr)
		{
			shader.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr);
		}

		shader.draw(*baseDrawable->mMesh);
	}
}

void Scenery::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void Scenery::createWaterDrawable()
{
	// Get mesh and shader
	Resource<GL::Mesh> resMesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<WaterShader>(RESOURCE_MESH_PLANE_WATER);
	Resource<GL::AbstractShaderProgram, WaterShader> resShader = CommonUtility::singleton->getWaterShader();

	// Create new water object
	auto wdh = WaterDrawableHolder();

	// Common texture
	const auto& waterTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_WATER_TEXTURE);

	auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
	wdh.drawable = std::make_shared<GameDrawable<WaterShader>>(*drawables, resShader, resMesh, waterTexture);
	wdh.drawable->setParent(mManipulatorList[1]);
	wdh.drawable->setDrawCallback(this);

	wdh.parameters = {
		CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_WATER_DISPLACEMENT),
		waterTexture,
		CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_WORLD_1_WEM),
		0.0f,
		2.0f,
		Vector2(6.0f, 3.0f),
		Color3(1.0f)
	};

	mWaterHolders[wdh.drawable.get()] = std::move(wdh);
}

void Scenery::createWaterDrawable(const WaterDrawableHolder & fromWdh)
{
	// Create new water object
	auto wdh = WaterDrawableHolder();

	auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
	wdh.drawable = std::make_shared<GameDrawable<WaterShader>>(*drawables, fromWdh.drawable->mShader, fromWdh.drawable->mMesh, fromWdh.drawable->mTexture);
	wdh.drawable->setParent(mManipulatorList[1]);
	wdh.drawable->setDrawCallback(this);

	wdh.parameters = fromWdh.parameters;

	mWaterHolders[wdh.drawable.get()] = std::move(wdh);
}

const Int Scenery::getModelIndex() const
{
	return mModelIndex;
}

const void Scenery::setLightPosition(const Vector3 & lightPosition)
{
	mLightPosition = lightPosition;
}

const void Scenery::animateInGameCamera()
{
	mAnimateInGameCamera = true;
}