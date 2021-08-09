#include "LevelSelector.h"

#include <utility>
#include <Magnum/Math/Math.h>
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
		Vector3(-5.99022f, 1.49128f, 6.6865f),
		Vector3(-0.079714f, 1.49128f, 6.6865f),
		Vector3(6.51419f, 1.49128f, 6.6865f),
		Vector3(8.33671f, 1.35076f, 1.32174f),
		Vector3(7.63514f, 1.35076f, -3.6977f),
		Vector3(-4.86925f, 1.17022f, -3.6977f),
	})
};

LevelSelector::LevelSelector(const Int parentIndex) : GameObject(), mCbEaseInOut(Vector2(0.0f, 0.0f), Vector2(0.42f, 0.0f), Vector2(0.58f, 1.0f), Vector2(1.0f, 1.0f))
{
	// Assign parent index
	mParentIndex = parentIndex;

	// Init members
	mPosition = Vector3(0.0f);

	mPrevMousePos = Vector2i(GO_LS_RESET_MOUSE_VALUE, -1);
	mScrollVelocity = Vector3(0.0f);
	mClickIndex = -1;
	mLevelButtonScaleAnim = 0.0f;

	mSettingsOpened = false;
	mSettingsAnim = 0.0f;

	mCurrentViewingLevelId = 0U;

	// Create sky plane
	createSkyPlane();

	// Create button overlays
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_SETTINGS);
		o->setPosition({ -2.0f, 0.5f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ 1.0f, -1.0f });

		mScreenButtonAnim[GO_LS_GUI_SETTINGS] = 1.0f;
		mScreenButtons[GO_LS_GUI_SETTINGS] = (std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true);

		mCallbacks[GO_LS_GUI_SETTINGS] = [this]() {
			mSettingsOpened = !mSettingsOpened;
			Debug{} << "You have" << (mSettingsOpened ? "opened" : "closed") << "SETTINGS";
		};
	}

	// Create overlay eye-candy drawables
	mLevelAnim = 0.0f;

	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_LEVEL_PANEL);
		o->setPosition({ 2.0f, 2.0f });
		o->setSize({ 0.45f, 0.45f });
		o->setAnchor({ 0.0f, 0.0f });

		mLevelGuis[GO_LS_GUI_LEVEL_PANEL] = (std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true);
	}

	for (UnsignedInt i = 0; i < 3; ++i)
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_STAR_GRAY);
		o->setPosition({ 2.0f, 2.0f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ 0.0f, 0.0f });

		mLevelGuis[GO_LS_GUI_STAR + i] = (std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true);
	}

	// Create overlay text
	{
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST);
		go->mPosition = Vector3(2.0f, 2.0f, 0.0f);
		go->mColor = Color4(1.0f, 1.0f, 1.0f, 1.0f);
		go->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
		go->setText("Level X");

		mLevelTexts[GO_LS_TEXT_LEVEL] = (std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(go, true);
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
	mScreenButtons.clear();
	mLevelGuis.clear();
	mLevelTexts.clear();
	mSceneries.clear();
	mPickableObjectRefs.clear();
}


const Int LevelSelector::getType() const
{
	return GOT_LEVEL_SELECTOR;
}

void LevelSelector::update()
{
#if NDEBUG or _DEBUG
	if (InputManager::singleton->mMouseStates[ImMouseButtons::Right] == IM_STATE_RELEASED)
	{
		// RoomManager::singleton->prepareRoom(false);
		RoomManager::singleton->createLevelRoom();
	}
#endif

	// Update sky plane
	(*mSkyManipulator)
		.resetTransformation()
		.scale(Vector3(GO_LS_SCENERY_LENGTH, GO_LS_SCENERY_LENGTH, 1.0f))
		.translate(mPosition + Vector3(0.0f, 0.0f, -GO_LS_SCENERY_LENGTH_DOUBLE));

	// Check if there is any on-going action on top
	if (RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].list->size() > 0)
	{
		return;
	}

	// Overlay for common
	windowForCommon();

	// Overlay for settings
	manageBackendAnimationVariable(mSettingsAnim, 0.8f, mSettingsOpened);
	windowForSettings();

	// Overlay for current level viewing
	const bool isViewing = mCurrentViewingLevelId != 0U;
	manageBackendAnimationVariable(mLevelAnim, 0.8f, isViewing);
	windowForCurrentLevelView();

	// Animation for overlay buttons
	for (UnsignedInt i = 0; i < 1; ++i)
	{
		mScreenButtonAnim[i] -= mDeltaTime;
		if (mScreenButtonAnim[i] < 0.0f)
		{
			mScreenButtonAnim[i] = 0.0f;
		}
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
		// Increment scale value
		mLevelButtonScaleAnim += mDeltaTime;
		if (mLevelButtonScaleAnim > 1.0f)
		{
			mLevelButtonScaleAnim = 1.0f;
		}

		// Create common scaling vector
		const Vector3 sv(mLevelButtonScaleAnim);

		// Apply transformations to all button drawables
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
						.scale(sv)
						.translate(it2->position);
				}
			}
		}
	}

	// Handle scrollable scenery
	if (lbs == IM_STATE_PRESSED)
	{
		if (!isViewing && !mSettingsOpened)
		{
			mPrevMousePos = InputManager::singleton->mMousePosition;
			mClickStartTime = std::chrono::system_clock::now();
		}
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
			const UnsignedInt oid = InputManager::singleton->mClickedObjectId;
			if (oid != 0U)
			{
				const std::chrono::duration<double> diff = std::chrono::system_clock::now() - mClickStartTime;
				if (diff.count() < GO_LS_CLICK_TAP_MAX_DELAY)
				{
					const auto& it = mPickableObjectRefs.find(oid);
					if (it != mPickableObjectRefs.end())
					{
						const auto& it2 = mSceneries.find(it->second);
						if (it2 == mSceneries.end())
						{
							Error{} << "Scenery from PickableObjectRef" << it2->first << "was not found. This should not happen";
						}
						else
						{
							// Check for clicked level index in the referenced scenery
							LS_PickableObject* spo = nullptr;
							for (auto& po : it2->second.buttons)
							{
								if (po.objectId == oid)
								{
									spo = &po;
								}
							}

							// Trigger level selection
							if (spo != nullptr)
							{
								clickLevelButton(spo->levelIndex);
							}
							else
							{
								Error{} << "No corresponding object for identifier" << oid << "was found in scenery" << it2->first;
							}
						}
					}
				}
			}
		}
	}

	if (!mScrollVelocity.isZero())
	{
		// Scroll scenery
		handleScrollableScenery();

		// Increment by delta
		const auto mOldPosition = mPosition;
		mPosition += mScrollVelocity;

		// Limit on various axis
		{
			const auto& data = mPosition.data();

			// Limit X axis
			if (data[0] < -25.0f)
			{
				data[0] = -25.0f;
			}
			else if (data[0] > 25.0f)
			{
				data[0] = 25.0f;
			}

			// Limit Z axis
			if (data[2] > 10.0f)
			{
				data[2] = 10.0f;
			}
		}

		// Scroll camera
		handleScrollableCameraPosition(mPosition - mOldPosition);
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

constexpr void LevelSelector::manageBackendAnimationVariable(Float & variable, const Float factor, const bool increment)
{
	if (increment)
	{
		// Advance animation
		variable += factor * mDeltaTime;
		if (variable > 1.0f)
		{
			variable = 1.0f;
		}
	}
	else
	{
		// Reduce animation
		variable -= factor * mDeltaTime;
		if (variable < 0.0f)
		{
			variable = 0.0f;
		}
	}
}

void LevelSelector::createSkyPlane()
{
	Resource<GL::Mesh> resMesh = CommonUtility::singleton->getPlaneMeshForFlatShader();
	Resource<GL::Texture2D> resTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_SKYBOX_1_PX);
	Resource<GL::AbstractShaderProgram, Shaders::Flat3D> resShader = CommonUtility::singleton->getFlat3DShader();

	mSkyManipulator = new Object3D{ mManipulator.get() };

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
		Int(Math::floor((mPosition.z() - GO_LS_SCENERY_LENGTH) / GO_LS_SCENERY_LENGTH)),
		Int(Math::floor(mPosition.z() / GO_LS_SCENERY_LENGTH)),
		Int(Math::floor((mPosition.z() + GO_LS_SCENERY_LENGTH) / GO_LS_SCENERY_LENGTH))
	};

	// Erase scenes out of view
	for (auto it = mSceneries.begin(); it != mSceneries.end();)
	{
		bool erase = false;

		if (yps.find(it->first) == yps.end())
		{
			Debug{} << "Erasing scenery" << it->first;

			// Erase pickable objects
			for (UnsignedInt i = 0; i < 6; ++i)
			{
				const auto key = UnsignedInt(it->first) * 6U + UnsignedInt(i + 1);
				mPickableObjectRefs.erase(key);
			}

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
							Debug{} << "A drawable for scenery" << it->first << "is going to be freed";
							mDrawables.erase(itd);
						}
					}
				}
			}

			// Mark scene as "to be erased"
			erase = true;
		}

		// Erase this scenery AT THE END
		if (erase)
		{
			it->second.scenery->mDestroyMe = true;
			it = mSceneries.erase(it);
		}
		else
		{
			++it;
		}
	}

	// Create visible sceneries
	for (const Int yp : yps)
	{
		// Avoid backward positions
		if (yp > 0)
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
		const Vector3 tp = Vector3(0.0f, 0.0f, GO_LS_SCENERY_LENGTH * Float(yp));

		mSceneries[yp].manipulator = new Object3D(mManipulator.get());
		mSceneries[yp].manipulator->translate(tp);

		// Create scenery
		{
			std::shared_ptr<Scenery> go = std::make_unique<Scenery>(GOL_PERSP_FIRST, modelIndex);
			go->mManipulator->transform(mSceneries[yp].manipulator->transformation());
			go->mPosition = tp;
			mSceneries[yp].scenery = (std::shared_ptr<Scenery>&) RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST].push_back(go, true);

			Debug{} << "Scenery with model index" << modelIndex << "is created at position" << yp;
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
			
			// Compute object identifier
			const UnsignedInt objectId = UnsignedInt(yp) * 6U + UnsignedInt(i + 1);
			mPickableObjectRefs[objectId] = yp;

			bs.levelIndex = objectId;
			bs.objectId = objectId;

			// Load drawables
			AssetManager am(RESOURCE_SHADER_COLORED_PHONG, RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE, 1);
			am.loadAssets(*this, *mSceneries[yp].manipulator, "scenes/level_button.glb", this);

			// Apply the same transformations
			bs.position = sLevelButtonPositions[modelIndex][i];

			// Create all required drawables
			for (UnsignedInt j = 0; j < 3; ++j)
			{
				const std::shared_ptr<BaseDrawable>& bd = mDrawables[mDrawables.size() - j - 1];
				bd->setObjectId(objectId);
				bs.drawables.emplace_back(bd);
			}
		}
	}
}

void LevelSelector::clickLevelButton(const UnsignedInt id)
{
	Debug{} << "You have clicked level" << id;

	// Set current level identifier
	mCurrentViewingLevelId = id;

	// Set text for selected level
	mLevelTexts[GO_LS_TEXT_LEVEL]->setText("Level " + std::to_string(id));
}

void LevelSelector::windowForCommon()
{
	const auto& d = mCbEaseInOut.value(mSettingsAnim + mLevelAnim)[1];

	// Main panel
	mLevelGuis[GO_LS_GUI_LEVEL_PANEL]->setPosition({ 0.0f, 1.0f - d });
}

void LevelSelector::windowForSettings()
{
	const auto& d = mCbEaseInOut.value(mSettingsAnim)[1];

	// Animation for settings window
	{
		const auto& d2 = mCbEaseInOut.value(mScreenButtonAnim[GO_LS_GUI_SETTINGS])[1];
		mScreenButtons[GO_LS_GUI_SETTINGS]->setPosition(Vector2(-0.5f, 0.5f) - Vector2(d2, 0.0f) + Vector2(0.5f, -0.15f) * d);
		mScreenButtons[GO_LS_GUI_SETTINGS]->setAnchor(Vector2( 1.0f, -1.0f ) * (1.0f - d));
		mScreenButtons[GO_LS_GUI_SETTINGS]->setRotationInDegrees(360.0f * d);

		const bool isSettings = mScreenButtons[GO_LS_GUI_SETTINGS]->getTextureResource().key() == ResourceKey(RESOURCE_TEXTURE_GUI_SETTINGS);
		if (d > 0.5f)
		{
			if (isSettings)
			{
				mScreenButtons[GO_LS_GUI_SETTINGS]->setTexture(RESOURCE_TEXTURE_GUI_BACK_ARROW);
			}
		}
		else
		{
			if (!isSettings)
			{
				mScreenButtons[GO_LS_GUI_SETTINGS]->setTexture(RESOURCE_TEXTURE_GUI_SETTINGS);
			}
		}
	}
}

void LevelSelector::windowForCurrentLevelView()
{
	const auto& d = mCbEaseInOut.value(mLevelAnim)[1];

	// Score stars
	for (UnsignedInt i = 0; i < 3; ++i)
	{
		mLevelGuis[GO_LS_GUI_STAR + i]->setPosition({ -0.2f + 0.2f * Float(i), 1.25f - d });
	}

	// Level texts
	mLevelTexts[GO_LS_TEXT_LEVEL]->setPosition({ 0.0f, 1.15f - d, 0.0f });
}