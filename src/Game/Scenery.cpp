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

Scenery::Scenery(const Int parentIndex, const Int modelIndex, const Int subType) : GameObject(parentIndex)
{
	// Init members
	// mCubicBezier = std::make_unique<CubicBezier2D>(Vector2(0.0f, 0.0f), Vector2(0.11f, -0.02f), Vector2(0.0f, 1.01f), Vector2(1.0f));
	mParentIndex = parentIndex;
	mModelIndex = modelIndex;
	mSubType = subType;
	mLightPosition = Vector3(0.0f);
	mAlphaCheckTimer = 5.0f;
	mFrame = 0.0f;

	mAnim = { 1.0f, 0.0f, 0.0f, 0.0f };

	// Fill manipulator list
	mManipulatorList.push_back(std::move(new Object3D{ mManipulator.get() }));
	mManipulatorList.push_back(std::move(new Object3D{ mManipulator.get() }));
	mManipulatorList.push_back(std::move(new Object3D{ mManipulator.get() }));

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

			rk = RESOURCE_SCENE_WORLD_1;

			mAnim.inc = 0.5f;
			mAnim.rotateFactor = 5.0f;
			mAnim.scaleFactor = 1.0f;

			break;

		case 1:
			rk = RESOURCE_SCENE_WORLD_2;

			mAnim.inc = 0.5f;
			mAnim.rotateFactor = 2.5f;
			mAnim.scaleFactor = 0.25f;

			break;

		case 2:
			rk = RESOURCE_SCENE_WORLD_3;

			mAnim.inc = 0.5f;
			mAnim.rotateFactor = 5.0f;
			mAnim.scaleFactor = 0.05f;

			break;

		case 3:
			rk = RESOURCE_SCENE_WORLD_4;

			mAnim.inc = 3.0f;
			mAnim.rotateFactor = 5.0f;
			mAnim.scaleFactor = 0.05f;

			mSunShader = CommonUtility::singleton->getSunShader();
			mSunAlphaMap = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_WATER_DISPLACEMENT);

			mStarRoadShader = CommonUtility::singleton->getStarRoadShader();
			mStarRoadAlphaMap = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_STARROAD_ALPHAMAP);

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

	// Get wind-animated objects
	{
		const std::unordered_set<std::string> names1 = { "PalmTreeBarkV", "PalmTreeLeavesVT", "PalmV", "CactusV", "PineVT", "PineTrunkV", "SaturnGlobeV" };
		const std::unordered_set<std::string> names2 = { "LeavesVT", "BushesVT", "BushLeavesVT", "TreeVT" };

		for (const auto& item : mDrawables)
		{
			const auto& label = item->mMesh->label();

			// Saturn Globe
			if (label == "SaturnGlobeV")
			{
				if (mSubType == 0)
				{
					item->mTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_MOON);
				}
			}
			else if (label == "SunGlobeV")
			{
				mSun = item;
			}

			// Star Road
			if (label == "RoadSideVT")
			{
				mStarRoad = item;
			}
			// Saturn Ring
			else if (label == "SaturnRingVT")
			{
				(*item)
					.resetTransformation()
					.rotateX(Deg(mSubType == 0 ? 0.0f : -5.0f));

				if (mSubType == 0)
				{
					item->translate(Vector3(0.0f, -500.0f, 0.0f));
				}
			}
			// Rotate
			else if (names1.find(label) != names1.end())
			{
				mWindRotateObjects.emplace_back(item);
			}
			// Scale
			else if (names2.find(label) != names2.end())
			{
				mWindScaleObjects.emplace_back(item);
			}
		}
	}

	// Create transparent drawables AFTER
	if (modelIndex == 0)
	{
		createWaterDrawable();
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
	// Alpha check timer
	mAlphaCheckTimer -= mDeltaTime;
	if (mAlphaCheckTimer <= 0.0f)
	{
		mAlphaCheckTimer = 5.0f;
		for (auto& item : mDrawables)
		{
			if (CommonUtility::singleton->stringEndsWith(item->mMesh->label(), "T"))
			{
				item->pushToFront();
			}
		}
	}

	// Animate wind objects
	mAnim.frame += mDeltaTime * mAnim.inc;
	{
		const Float angle = mModelIndex == 3 ? Float(Deg(mAnim.frame)) : Math::sin(Rad(mAnim.frame));
		for (auto& item : mWindRotateObjects)
		{
			if (item.expired())
			{
				continue;
			}

			const auto& d = item.lock();

			if (mModelIndex == 3)
			{
				(*d)
					.resetTransformation()
					.rotateY(Deg(angle * mAnim.rotateFactor))
					.rotateX(Deg(15.0f))
					.rotateY(Deg(45.0f));

				if (mSubType == 0 && d->mMesh->label() == "SaturnGlobeV")
				{
					d->translate(Vector3(0.0f, 0.0f, 10.0f));
				}
			}
			else
			{
				(*d)
					.resetTransformation()
					.rotateX(Deg(angle * mAnim.rotateFactor));
			}
		}

		for (auto& item : mWindScaleObjects)
		{
			if (item.expired())
			{
				continue;
			}

			const auto& d = item.lock();
			(*d)
				.resetTransformation()
				.scale(Vector3(1.0f, 1.0f, angle * mAnim.scaleFactor + 0.75f));
		}
	}

	// Update frame
	mFrame += mDeltaTime * 0.5f;
	while (mFrame > 30.0f)
	{
		mFrame -= 30.0f;
	}

	for (auto& wh : mWaterHolders)
	{
		wh.second.parameters.frame = mFrame;
	}

	// Debug camera move
#ifdef DEBUG
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
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.setFrame(it->second.parameters.frame)
			.setSpeed(it->second.parameters.speed)
			.setSize(it->second.parameters.size)
			.setHorizonColorUniform(it->second.parameters.horizonColor)
			.bindDisplacementTexture(*it->second.parameters.displacementTexture)
			.bindWaterTexture(*it->second.parameters.waterTexture)
			.bindEffectsTexture(*it->second.parameters.effectsTexture)
			.draw(*baseDrawable->mMesh);
	}
	else if (!mStarRoad.expired() && mStarRoad.lock().get() == baseDrawable)
	{
		(*mStarRoadShader)
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindDisplacementTexture(*baseDrawable->mTexture)
			.bindAlphaMapTexture(*mStarRoadAlphaMap)
			.setIndex(mFrame)
			.draw(*baseDrawable->mMesh);
	}
	else if (!mSun.expired() && mSun.lock().get() == baseDrawable)
	{
		(*mSunShader)
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindDisplacementTexture(*mSunAlphaMap)
			.bindColorTexture(*baseDrawable->mTexture)
			.setIndex(mFrame)
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

void Scenery::createWaterDrawable()
{
	// Get mesh and shader
	Resource<GL::Mesh> resMesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<WaterShader::Position, WaterShader::TextureCoordinates>(RESOURCE_MESH_PLANE_WATER);
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

Int Scenery::getModelIndex() const
{
	return mModelIndex;
}

void Scenery::setLightPosition(const Vector3 & lightPosition)
{
	mLightPosition = lightPosition;
}

void Scenery::animateInGameCamera()
{
	mAnimateInGameCamera = true;
}
