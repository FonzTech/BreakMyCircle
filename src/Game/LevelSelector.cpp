#include "LevelSelector.h"

#include <utility>
#include <Magnum/Math/Constants.h>

#include "../RoomManager.h"
#include "../InputManager.h"
#include "../AssetManager.h"
#include "../Common/CommonUtility.h"

std::shared_ptr<GameObject> LevelSelector::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate player object
	std::shared_ptr<LevelSelector> p = std::make_shared<LevelSelector>(parent);
	return p;
}

std::unordered_map<Int, std::array<Vector3, 6>> LevelSelector::sLevelButtonPositions = {
	std::make_pair(0, std::array<Vector3, 6>{
		Vector3(0.516384f, 1.2582f, -9.61229f),
		Vector3(4.63997f, 1.28248f, 8.91989f),
		Vector3(5.58304f, 1.2582f, -4.80427f),
		Vector3(-4.66971f, 1.2582f, -1.63549f),
		Vector3(5.58304f, 1.42328f, 1.26542f),
		Vector3(-3.12115f, 1.28248f, 8.91989f)
	})
};

LevelSelector::LevelSelector(const Int parentIndex) : GameObject()
{
	// Assign parent index
	mParentIndex = parentIndex;

	// Init members
	mPosition = Vector3(0.0f);
	mPrevMousePos = Vector2i(GO_LS_RESET_MOUSE_VALUE, -1);
	mScrollVelocity = Vector3(0.0f);
	mClickIndex = -1;
	mLevelButtonScaleAnim = 0.0f;

	// Create sky plane
	createSkyPlane();

	// Create overlays
	{
		std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST);
		o->setPosition({ -2.0f, 0.5f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ 1.0f, -1.0f });

		mScreenButtonAnim[0] = Constants::piHalf();
		mScreenButtons[0] = (std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true);

		mCallbacks[0] = []() {
			printf("You have clicked settings\n");
		};
	}

	// Set camera parameters
	{
		auto& layer = RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST];
		layer.cameraEye = mPosition + Vector3(0.0f, 10.0f, 20.0f);
		layer.cameraTarget = mPosition;
	}

	// Trigger scenery creation
	handleScrollableScenery();
}

LevelSelector::~LevelSelector()
{
	mScreenButtons[0] = nullptr;
	mSceneries.clear();
}


const Int LevelSelector::getType() const
{
	return GOT_LEVEL_SELECTOR;
}

void LevelSelector::update()
{
	// Check if there is any on-going action on top
	if (RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].list->size() > 0)
	{
		return;
	}

	// Animation for overlay buttons
	for (UnsignedInt i = 0; i < 1; ++i)
	{
		mScreenButtonAnim[0] -= mDeltaTime;
		if (mScreenButtonAnim[0] < 0.0f)
		{
			mScreenButtonAnim[0] = 0.0f;
		}

		mScreenButtons[i]->setPosition(Vector2(-0.5f, 0.5f) - Vector2(Math::sin(Rad(mScreenButtonAnim[0])), 0.0f));
	}

	// Handle button clicks
	const auto& lbs = InputManager::singleton->mMouseStates[ImMouseButtons::Left];

	const Vector3 p(Float(InputManager::singleton->mMousePosition.x()), Float(InputManager::singleton->mMousePosition.y()), 0.0f);
	const Vector2 w(Float(RoomManager::singleton->mWindowSize.x()), Float(RoomManager::singleton->mWindowSize.y()));

	for (Int i = 0; i < 1; ++i)
	{
		const auto& b = mScreenButtons[i]->getBoundingBox(w);
		if (b.contains(p))
		{
			if (lbs == IM_STATE_PRESSED)
			{
				mClickIndex = i;
				break;
			}
			else if (lbs == IM_STATE_RELEASED && mClickIndex == i)
			{
				mCallbacks[i]();
				break;
			}
		}
	}

	// Handle button scale animation 
	{
		mLevelButtonScaleAnim += mDeltaTime;
		if (mLevelButtonScaleAnim > 1.0f)
		{
			mLevelButtonScaleAnim = 1.0f;
		}

		mLevelButtonScaleAnim = 1.0f;
		const Vector3 sv(mLevelButtonScaleAnim);

		for (auto it = mSceneries.begin(); it != mSceneries.end(); ++it)
		{
			for (auto it2 = it->second.buttons.begin(); it2 != it->second.buttons.end(); ++it2)
			{
				for (auto dp : it2->drawables)
				{
					if (dp.expired())
					{
						Error{} << "weak_ptr for" << it2->levelIndex << " has expired. This should not happen";
						continue;
					}

					(*dp.lock())
						.resetTransformation()
						.scale(sv);
				}
			}
		}
	}

	// Handle scrollable scenery
	if (lbs == IM_STATE_PRESSED)
	{
		mPrevMousePos = InputManager::singleton->mMousePosition;
		mClickStartTime = std::chrono::system_clock::now();
	}
	else if (lbs >= IM_STATE_PRESSED)
	{
		// Update mouse delta
		if (mPrevMousePos.x() > GO_LS_RESET_MOUSE_VALUE)
		{
			const Vector2i mouseDelta = InputManager::singleton->mMousePosition - mPrevMousePos;
			if (mouseDelta.isZero())
			{
				mScrollVelocity = { 0.0f };
			}
			else
			{
				const Vector3 scrollDelta = Vector3(Float(mouseDelta.x()), 0.0f, Float(mouseDelta.y())) * -0.03f;
				mScrollVelocity = scrollDelta;
			}

			// Update previous mouse state
			mPrevMousePos = InputManager::singleton->mMousePosition;
		}
	}
	// Reset scrolling behaviour
	else
	{
		// Reset mouse value
		mPrevMousePos.data()[0] = GO_LS_RESET_MOUSE_VALUE;

		// Reset click index
		mClickIndex = -1;

		// Handle scroll inertia
		if (!mScrollVelocity.isZero())
		{
			for (UnsignedInt i = 0; i < 3; ++i)
			{
				if (mScrollVelocity[i] < -GO_LS_MAX_SCROLL_VELOCITY_MAX)
				{
					mScrollVelocity[i] = -GO_LS_MAX_SCROLL_VELOCITY_MAX;
				}
				else if (mScrollVelocity[i] > GO_LS_MAX_SCROLL_VELOCITY_MAX)
				{
					mScrollVelocity[i] = GO_LS_MAX_SCROLL_VELOCITY_MAX;
				}
			}

			Vector3 scrollDelta = mScrollVelocity;
			for (UnsignedInt i = 0; i < 3; ++i)
			{
				if (scrollDelta[i] < -GO_LS_MAX_SCROLL_VELOCITY)
				{
					scrollDelta[i] = -GO_LS_MAX_SCROLL_VELOCITY;
				}
				else if (scrollDelta[i] > GO_LS_MAX_SCROLL_VELOCITY)
				{
					scrollDelta[i] = GO_LS_MAX_SCROLL_VELOCITY;
				}
				else if (std::abs(scrollDelta[i]) < GO_LS_MAX_SCROLL_THRESHOLD)
				{
					mScrollVelocity[i] = 0.0f;
					scrollDelta[i] = 0.0f;
				}
			}

			mScrollVelocity -= scrollDelta;
		}

		// Check for click release
		if (lbs == IM_STATE_RELEASED)
		{
			const auto& oid = InputManager::singleton->mClickedObjectId;
			if (oid != 0U)
			{
				const std::chrono::duration<double> diff = std::chrono::system_clock::now() - mClickStartTime;
				if (diff.count() < 0.3)
				{
					const auto& it = mPickableObjectPointers.find(oid);
					if (it != mPickableObjectPointers.end())
					{
						clickLevelButton(it->second->levelIndex);
					}
				}
			}
		}
	}

	if (!mScrollVelocity.isZero())
	{
		mPosition += mScrollVelocity;
		handleScrollableCameraPosition(mScrollVelocity);
		handleScrollableScenery();
	}
}

void LevelSelector::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (baseDrawable == mSkyPlane.get())
	{
		((Shaders::Flat3D&)baseDrawable->getShader())
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindTexture(*baseDrawable->mTexture)
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		((Shaders::Phong&)baseDrawable->getShader())
			.setLightPosition(camera.cameraMatrix().transformPoint(mPosition + Vector3(0.0f, 6.0f, 0.0f)))
			.setLightColor(0xffffffff_rgbaf)
			.setSpecularColor(0xffffff00_rgbaf)
			.setAmbientColor(0x444444ff_rgbaf)
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
			.setObjectId(baseDrawable->getObjectId())
			.draw(*baseDrawable->mMesh);
	}
}

void LevelSelector::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void LevelSelector::createSkyPlane()
{
	Resource<GL::Mesh> resMesh = CommonUtility::singleton->getPlaneMeshForFlatShader();
	Resource<GL::Texture2D> resTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_SKYBOX_1_PX);
	Resource<GL::AbstractShaderProgram, Shaders::Flat3D> resShader = CommonUtility::singleton->getFlat3DShader();

	mSkyManipulator = new Object3D{ mManipulator.get() };
	mSkyManipulator->scale(Vector3(50.0f, 50.0f, 1.0f));
	mSkyManipulator->translate(Vector3(0.0f, 0.0f, -100.0f));

	auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
	mSkyPlane = std::make_shared<TexturedDrawable<Shaders::Flat3D>>(*drawables, resShader, resMesh, resTexture);
	mSkyPlane->setParent(mSkyManipulator);
	mSkyPlane->setDrawCallback(this);
	mDrawables.emplace_back(mSkyPlane);
}

void LevelSelector::handleScrollableCameraPosition(const Vector3 & delta)
{
	auto& p = RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST];
	p.cameraEye += delta;
	p.cameraTarget += delta;
}

void LevelSelector::handleScrollableScenery()
{
	// Get visible positions
	const std::unordered_set<Int> yps = {
		Int(Math::floor((mPosition.y() - 50.0f) / 50.0f)),
		Int(Math::floor(mPosition.y() / 50.0f)),
		Int(Math::floor((mPosition.y() + 50.0f) / 50.0f))
	};

	// Erase scenes out of view
	for (auto it = mSceneries.begin(); it != mSceneries.end(); ++it)
	{
		if (yps.find(it->first) == yps.end())
		{
			// Erase this scenery
			mSceneries.erase(it->first);

			// Erase pickable objects
			mPickableObjectPointers.erase(it->first);

			// Delete drawable references for this scenery
			for (auto it2 = it->second.buttons.begin(); it2 != it->second.buttons.end(); ++it2)
			{
				for (auto it3 = it2->drawables.begin(); it3 != it2->drawables.end(); ++it3)
				{
					if (!(*it3).expired())
					{
						const auto& itd = std::find(mDrawables.begin(), mDrawables.end(), (*it3).lock());
						if (itd != mDrawables.end())
						{
							Debug{} << "Drawable at", &(*itd), "was erased";
							mDrawables.erase(itd);
						}
					}
				}
			}
		}
	}

	// Create visible sceneries
	for (const Int yp : yps)
	{
		// Avoid negative positions
		if (yp < 0.0f)
		{
			continue;
		}

		// Avoid scenes which are already what they should be (island, forest, desert, etc...)
		// const Int modelIndex = (yp[i] % 1) + 1;
		const Int modelIndex = 0;

		const auto& it = mSceneries.find(yp);
		if (it != mSceneries.end())
		{
			if (it->second.scenery->getModelIndex() == modelIndex)
			{
				continue;
			}
		}

		// Create new data structure scenery with level selectors
		mSceneries[yp] = LS_ScenerySelector();

		// Create manipulator (avoid putting this object's drawables into another object's manipulator)
		const Vector3 tp = Vector3(0.0f, 0.0f, -50.0f * Float(yp));

		mSceneries[yp].manipulator = new Object3D(mManipulator.get());
		mSceneries[yp].manipulator->translate(tp);

		// Create scenery
		{
			std::shared_ptr<Scenery> go = std::make_unique<Scenery>(GOL_PERSP_FIRST, modelIndex);
			go->mManipulator->transform(mSceneries[yp].manipulator->transformation());
			go->mPosition = tp;
			mSceneries[yp].scenery = (std::shared_ptr<Scenery>&) RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST].push_back(go, true);
		}

		// Create level selectors
#if NDEBUG or _DEBUG
		if (mSceneries[yp].buttons.size())
		{
			Error{} << "Buttons vector for position" << yp << "should be empty, but it was not.";
		}
#endif

		// Create four drawables
		for (Int i = 0; i < 6; ++i)
		{
			// Create button selector
			mSceneries[yp].buttons.push_back(LS_PickableObject());
			auto& bs = mSceneries[yp].buttons.back();

			// Keep this button pointer for fast lookup
			const UnsignedInt objectId = UnsignedInt(yp) * 6U + UnsignedInt(i);
			mPickableObjectPointers[objectId] = &bs;

			// Load drawables
			AssetManager am(RESOURCE_SHADER_COLORED_PHONG, RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE, 1);
			am.loadAssets(*this, *mSceneries[yp].manipulator, "scenes/level_button.glb", this);

			for (UnsignedInt i = 0; i < 3; ++i)
			{
				const std::shared_ptr<BaseDrawable>& bd = mDrawables[mDrawables.size() - 1];
				bd->setObjectId(objectId);
				bs.drawables.emplace_back(bd);
			}

			// Apply the same transformations
			bs.position = sLevelButtonPositions[modelIndex][i];
			bs.levelIndex = objectId;

			/*
			(*td)
				.resetTransformation()
				.translate(bs.position)
				.scale(Vector3(mLevelButtonScaleAnim));
			*/
		}
	}
}

void LevelSelector::clickLevelButton(const UnsignedInt id)
{
	Debug{} << "You have clicked level" << id;
}