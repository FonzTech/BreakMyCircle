#include "LevelSelector.h"

#include <utility>
#include <Magnum/Math/Math.h>
#include <Magnum/Math/Constants.h>
#include <Magnum/GL/Renderer.h>

#include "../RoomManager.h"
#include "../InputManager.h"
#include "../AssetManager.h"
#include "../Common/CommonUtility.h"
#include "../Common/CustomRenderers/LSNumberRenderer.h"
#include "../Game/Player.h"

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
		Vector3(-0.079714f, 1.49128f, 6.6865f),
		Vector3(-5.99022f, 1.49128f, 6.6865f),
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

	mLevelStartedAnim = 0.0f;
	mLevelEndingAnim = false;

	mCurrentViewingLevelId = 0U;
	mMaxLevelId = 2U;
	mLevelState = GO_LS_LEVEL_INIT;

	// Create sky plane
	createSkyPlane();

	// Create overlay eye-candy drawables
	mLevelAnim = 0.0f;

	// Create "Settings" button
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_SETTINGS);
		o->setPosition({ -2.0f, 0.5f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ 1.0f, -1.0f });

		mScreenButtons[GO_LS_GUI_SETTINGS] = {
			(std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true),
			[this]() {
				// Avoid click on level ending
				if (mLevelEndingAnim)
				{
					return;
				}

				// Close level details screen
				if (mCurrentViewingLevelId != 0U)
				{
					mCurrentViewingLevelId = 0U;
				}
				// If already closed, open/close the settings window
				else if (mLevelState < GO_LS_LEVEL_FINISHED)
				{
					mSettingsOpened = !mSettingsOpened;
					if (mSettingsOpened)
					{
						if (!mPlayerPointer.expired())
						{
							((Player*)mPlayerPointer.lock().get())->mCanShoot = false;
						}
					}

					Debug{} << "You have" << (mSettingsOpened ? "opened" : "closed") << "SETTINGS";
				}
			},
			1.0f
		};
	}

	// Level panel (after the "Settings" button, so this panel appears on top of it)
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_LEVEL_PANEL);
		o->setPosition({ 2.0f, 2.0f });
		o->setSize({ 0.45f, 0.45f });
		o->setAnchor({ 0.0f, 0.0f });

		mLevelGuis[GO_LS_GUI_LEVEL_PANEL] = (std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true);
	}

	// Create "Replay" button
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_REPLAY);
		o->setPosition({ -2.0f, 0.5f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ 0.0f, 0.0f });

		mScreenButtons[GO_LS_GUI_REPLAY] = {
			(std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true),
			[this]() {
				Debug{} << "You have clicked REPLAY";
			},
			1.0f
		};
	}

	// Create "Next" button
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_NEXT);
		o->setPosition({ -2.0f, 0.5f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ 0.0f, 0.0f });

		mScreenButtons[GO_LS_GUI_NEXT] = {
			(std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true),
			[this]() {
				Debug{} << "You have clicked NEXT";

				// Restore level state to init
				mLevelState = GO_LS_LEVEL_RESTORING;
			},
			1.0f
		};
	}

	// Create "Share" button
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_SHARE);
		o->setPosition({ -2.0f, 0.5f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ 0.0f, 0.0f });

		mScreenButtons[GO_LS_GUI_SHARE] = {
			(std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true),
			[this]() {
				Debug{} << "You have clicked SHARE";
			},
			1.0f
		};
	}

	// Play button
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_PLAY);
		o->setPosition({ 2.0f, 2.0f });
		o->setSize({ 0.25f, 0.125f });
		o->setAnchor({ 0.0f, 0.0f });

		mScreenButtons[GO_LS_GUI_PLAY] = {
			(std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true),
			[this]() {
				// Check if a level is starting
				if (mLevelState > GO_LS_LEVEL_INIT)
				{
					return;
				}

				// Start the selected level
				Debug{} << "User wants to play level" << mCurrentViewingLevelId;

				mLevelEndingAnim = false;

				mCurrentViewingLevelId = 0U;
				mLevelState = GO_LS_LEVEL_STARTING;
			},
			1.0f
		};
	}

	// Create "Exit" button
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_EXIT);
		o->setPosition({ -2.0f, 0.5f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ 0.0f, 0.0f });

		mScreenButtons[GO_LS_GUI_EXIT] = {
			(std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true),
			[this]() {
				Debug{} << "You have clicked EXIT";

				// Set variable flag for level ending
				if (mLevelState == GO_LS_LEVEL_STARTED)
				{
					mLevelEndingAnim = true;
				}
			},
			1.0f
		};
	}

	// Three stars
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
		go->setScale(Vector2(1.25f));
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
	// Enable or disable player shooting
	if (!mPlayerPointer.expired())
	{
		((Player*)mPlayerPointer.lock().get())->mCanShoot = !mSettingsOpened && mSettingsAnim <= 0.001f;
	}

	// Manage level state
	manageLevelState();

#if NDEBUG or _DEBUG
	if (InputManager::singleton->mMouseStates[ImMouseButtons::Right] == IM_STATE_RELEASED)
	{
		if (mPlayerPointer.expired())
		{
			Debug{} << "Level room created";

			createLevelRoom();
		}
		else
		{
			Debug{} << "Level state to Finished";
			finishCurrentLevel(false);
		}
	}
#endif

	// Update sky plane
	(*mSkyManipulator)
		.resetTransformation()
		.scale(Vector3(GO_LS_SCENERY_LENGTH, GO_LS_SCENERY_LENGTH, 1.0f))
		.translate(mPosition + Vector3(0.0f, 0.0f, -GO_LS_SCENERY_LENGTH_DOUBLE));

	// Check for second layer (the gameplay one)
	if (mLevelState == GO_LS_LEVEL_FINISHED && mLevelStartedAnim <= 0.0f)
	{
		// Delete all objects from the second layer
		for (auto& go : *RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].list)
		{
			go->mDestroyMe = true;
		}
	}

	// Check if there is any on-going action on top
	if (mLevelState == GO_LS_LEVEL_INIT && RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].list->size() > 0)
	{
		return;
	}

	// Overlay for common
	windowForCommon();

	// Overlay for settings
	manageBackendAnimationVariable(mSettingsAnim, 0.8f, mSettingsOpened && !mLevelEndingAnim);
	windowForSettings();

	// Overlay for current level viewing
	const bool isViewing = mCurrentViewingLevelId != 0U || mLevelState == GO_LS_LEVEL_FINISHED;
	manageBackendAnimationVariable(mLevelAnim, 0.8f, isViewing);
	windowForCurrentLevelView();

	// Control level ending
	if (mLevelEndingAnim)
	{
		// Check for animation end
		if (mSettingsAnim <= 0.0f)
		{
			// Close settings
			mSettingsOpened = false;

			// End this transition
			mLevelEndingAnim = false;

			// Finish current level with "Fail" message
			finishCurrentLevel(false);
		}
	}

	// Animation for overlay buttons
	for (auto it = mScreenButtons.begin(); it != mScreenButtons.end(); ++it)
	{
		it->second.animation -= mDeltaTime;
		if (it->second.animation < 0.0f)
		{
			it->second.animation = 0.0f;
		}
	}

	// Handle button clicks
	const auto& lbs = InputManager::singleton->mMouseStates[ImMouseButtons::Left];

	const Vector3 p(Float(InputManager::singleton->mMousePosition.x()), Float(InputManager::singleton->mMousePosition.y()), 0.0f);
	const Vector2 w(Float(RoomManager::singleton->mWindowSize.x()), Float(RoomManager::singleton->mWindowSize.y()));

	for (auto it = mScreenButtons.begin(); it != mScreenButtons.end(); ++it)
	{
		const auto& b = it->second.drawable->getBoundingBox(w);
		if (b.contains(p))
		{
			if (lbs == IM_STATE_PRESSED)
			{
				mClickIndex = it->first;
				break;
			}
			else if (lbs == IM_STATE_RELEASED && mClickIndex == it->first)
			{
				it->second.callback();
				break;
			}
		}
	}

	// Handle button scale animation 
	{
		// Handle scale value
		{
			const bool& mode = mLevelState == GO_LS_LEVEL_INIT;
			manageBackendAnimationVariable(mLevelButtonScaleAnim, mode ? 1.0f : 0.8f, mode);
		}

		// Apply transformations to all button drawables
		for (auto it = mSceneries.begin(); it != mSceneries.end(); ++it)
		{
			for (auto it2 = it->second.buttons.begin(); it2 != it->second.buttons.end(); ++it2)
			{
				// Control animation
				{
					const auto& pz = it2->position.z() + it->second.scenery->mPosition.z();
					const auto& c = mPosition.z() > pz - 25.0f && mPosition.z() < pz + 25.0f;
					manageBackendAnimationVariable(it2->scale, 1.0f, c);
				}

				// Create common scaling vector
				const Vector3 sv(it2->scale * mLevelButtonScaleAnim);

				// Apply transformations to all drawables for this pickable object
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
		if (!isViewing && !mSettingsOpened && mLevelState == GO_LS_LEVEL_INIT && !mLevelEndingAnim)
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
								if (spo->levelIndex < mMaxLevelId)
								{
									clickLevelButton(spo->levelIndex);
								}
								else
								{
									Debug{} << "Max level ID is" << mMaxLevelId << "and user has selected level ID" << spo->levelIndex;
								}
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
			.setColor(Color4{ 1.0f })
			.draw(*baseDrawable->mMesh);
	}
	else
	{
		const auto& color = baseDrawable->getObjectId() < mMaxLevelId ? 0xc0c0c0_rgbf : 0x404040_rgbf;
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
			const UnsignedInt objectId = UnsignedInt(-yp) * 6U + UnsignedInt(i + 1);
			mPickableObjectRefs[objectId] = yp;

			bs.levelIndex = objectId;
			bs.objectId = objectId;
			bs.scale = 0.0f;
			bs.selectable = false;

			// Create and save texture
			{
				const auto& nt = std::to_string(Int(objectId));
				const auto& key = "tex_ls_" + nt;

				bs.texture = CommonUtility::singleton->manager.get<GL::Texture2D>(key);
				if (!bs.texture)
				{
					// Create renderer
					LSNumberRenderer nr(mParentIndex, Vector2i(32), nt);

					// Render to texture
					GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
					GL::Texture2D & texture = nr.getRenderedTexture(true);
					GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);

					// Save to resource manager
					CommonUtility::singleton->manager.set(key, std::move(texture));
				}
			}

			// Load drawables
			AssetManager am(RESOURCE_SHADER_COLORED_PHONG, RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE, 1);
			am.loadAssets(*this, *mSceneries[yp].manipulator, "scenes/level_button.glb", this);

			// Apply the same transformations
			bs.position = sLevelButtonPositions[modelIndex][i];

			// Create all required drawables
			for (UnsignedInt j = 0; j < 3; ++j)
			{
				// Get drawable from the last three ones
				const std::shared_ptr<BaseDrawable>& bd = mDrawables[mDrawables.size() - j - 1];

				// Set the correct texture for the "platform" mesh
				if (bd->mMesh->label() == GO_LS_MESH_PLATFORM)
				{
					bd->mTexture = bs.texture;
				}

				// Set various properties
				bd->setObjectId(objectId);

				// Save reference for later handling
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
	mSelectedLevelId = id;

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
	const auto& dl = mCbEaseInOut.value(mLevelAnim)[1];

	// Animation for "Settings" button
	{
		const auto& d2 = mCbEaseInOut.value(mScreenButtons[GO_LS_GUI_SETTINGS].animation)[1];
		const auto& d3 = mLevelState > GO_LS_LEVEL_INIT ? mCbEaseInOut.value(1.0f - mLevelButtonScaleAnim)[1] : 0.0f;
		const auto& d4 = mLevelState == GO_LS_LEVEL_RESTORING ? mCbEaseInOut.value(1.0f - mLevelAnim)[1] : 0.0f;
		const auto& dp = mLevelState < GO_LS_LEVEL_FINISHED ? d + dl : 0.0f;

		const auto& drawable = mScreenButtons[GO_LS_GUI_SETTINGS].drawable;

		// Position
		{
			const auto& p1 = Vector2(-0.5f, 0.5f) - Vector2(d2, 0.0f); // Left to right
			const auto& p2 = (mLevelState == GO_LS_LEVEL_STARTED ? Vector2(0.1f, -0.05f) : Vector2(0.5f, -0.1f)) * dp; // Upper-left to mid-upper-center
			const auto& p3 = Vector2(0.0f, 0.25f) * d2; // Upper-left to outside-top
			const auto& p4 = Vector2(0.0f, -1.0f) * d3;
			const auto& p5 = mLevelState == GO_LS_LEVEL_STARTED ? Vector2(0.5f, 0.85f) * d : Vector2(0.0f);
			const auto& p6 = Vector2(-0.1f, 0.94f) * d4;
			const auto& p7 = mLevelState >= GO_LS_LEVEL_FINISHED ? Vector2(0.0f, -0.2f) * dl : Vector2(0.0f);
			drawable->setPosition(p1 + p2 + p3 + p4 + p5 + p6 + p7);
		}

		// Anchor
		{
			const auto& p1 = Vector2(1.0f, -1.0f) * (1.0f - dp - d3);
			const auto& p2 = Vector2(1.0f, 1.0f) * (d3 * (1.0f - d));
			const auto& p3 = mLevelState == GO_LS_LEVEL_RESTORING ? Vector2(1.0f, -1.0f) * (1.0f - dl) : Vector2(0.0f);
			drawable->setAnchor(p1 + p2 + p3);
		}

		// Rotation
		drawable->setRotationInDegrees(360.0f * (dp + d + d4));

		// Texture change
		const bool isSettings = drawable->getTextureResource().key() == ResourceKey(RESOURCE_TEXTURE_GUI_SETTINGS);
		if (dp > 0.5f || d > 0.5f)
		{
			if (isSettings)
			{
				drawable->setTexture(RESOURCE_TEXTURE_GUI_BACK_ARROW);
			}
		}
		else
		{
			if (!isSettings)
			{
				drawable->setTexture(RESOURCE_TEXTURE_GUI_SETTINGS);
			}
		}
	}
}

void LevelSelector::windowForCurrentLevelView()
{
	const bool& isFinished = mLevelState >= GO_LS_LEVEL_FINISHED;
	const auto& d = mCbEaseInOut.value(mLevelAnim)[1];
	const auto& s = mCbEaseInOut.value(mSettingsAnim)[1];

	// Score stars
	for (UnsignedInt i = 0; i < 3; ++i)
	{
		mLevelGuis[GO_LS_GUI_STAR + i]->setPosition({ -0.2f + 0.2f * Float(i), 1.25f - d });
	}

	// Play button
	{
		const auto& p1 = isFinished ? Vector2{ 2.0f } : Vector2{ 0.0f, -1.5f + d * 1.25f };
		mScreenButtons[GO_LS_GUI_PLAY].drawable->setPosition(p1);
	}

	// Level texts
	{
		const auto& p = mLevelState == GO_LS_LEVEL_STARTED ? s * 0.96f : 0.0f;
		mLevelTexts[GO_LS_TEXT_LEVEL]->setPosition({ 0.0f, 1.15f - (d + p), 0.0f });
	}

	{
		const bool& isStarted = mLevelState == GO_LS_LEVEL_STARTED;
		const auto& cd = -1.5f + d * 1.275f;
		const auto& cs = -1.5f + s * 1.275f;

		// Animation for "Replay" button
		{
			const auto& p1 = isFinished ? Vector2{ -0.3f, cd } : Vector2{ isStarted ? 0.0f : 2.0f };
			const auto& p2 = isStarted ? Vector2{ 0.2f, cs } : Vector2{ 0.0f };
			mScreenButtons[GO_LS_GUI_REPLAY].drawable->setPosition(p1 + p2);
		}

		// Animation for "Next" button
		{
			const auto& p1 = isFinished ? Vector2{ 0.0f, cd } : Vector2{ 2.0f };
			mScreenButtons[GO_LS_GUI_NEXT].drawable->setPosition(p1);
		}

		// Animation for "Share" button
		{
			const auto& p1 = isFinished ? Vector2{ 0.3f, cd } : Vector2{ 2.0f };
			mScreenButtons[GO_LS_GUI_SHARE].drawable->setPosition(p1);
		}

		// Animation for "Exit" button
		{
			const auto& p1 = mLevelState == GO_LS_LEVEL_STARTED ? Vector2{ -0.2f, cs } : Vector2{ 2.0f };
			mScreenButtons[GO_LS_GUI_EXIT].drawable->setPosition(p1);
		}
	}
}

void LevelSelector::createLevelRoom()
{
	// Level is started
	RoomManager::singleton->createLevelRoom();
	mLevelState = GO_LS_LEVEL_STARTED;

	// Get player pointer
	for (const auto& go : *RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].list)
	{
		if (go->getType() == GOT_PLAYER)
		{
			mPlayerPointer = go;
			break;
		}
	}

	if (mPlayerPointer.expired())
	{
		Error{} << "The player game object was not found after creating the room";
	}

	// Other variables
	mLevelStartedAnim = -1.0f; // Cycle waste
}

void LevelSelector::manageLevelState()
{
	// Variables for later
	bool animateCamera = false;

	// Control level state
	switch (mLevelState)
	{
	case GO_LS_LEVEL_STARTING:

		// Create level room on animation end
		if (mLevelButtonScaleAnim <= 0.0f)
		{
			createLevelRoom();
		}

		break;

	case GO_LS_LEVEL_STARTED:

		// Animate camera
		animateCamera = true;

		// Control animation
		if (mLevelStartedAnim < 0.0f) // Cycle waste
		{
			mLevelStartedAnim = 0.0f;
		}
		else
		{
			manageBackendAnimationVariable(mLevelStartedAnim, 1.0f, !mLevelEndingAnim);
		}

		break;

	case GO_LS_LEVEL_FINISHED:

		// Animate camera
		animateCamera = true;

		// Prevent player from shooting
		if (!mPlayerPointer.expired())
		{
			// Prevent player from shooting
			((Player*)mPlayerPointer.lock().get())->mCanShoot = false;
		}

		// Control animation
		manageBackendAnimationVariable(mLevelStartedAnim, 1.0f, false);

		break;

	case GO_LS_LEVEL_RESTORING:
		// Reset level state when animation has finished
		if (mLevelAnim <= 0.0f)
		{
			mLevelState = GO_LS_LEVEL_INIT;
		}
		break;
	}

	// Animate camera, if requested
	if (animateCamera)
	{
		const auto& d = mCbEaseInOut.value(mLevelStartedAnim)[1];

		auto& gol = RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND];
		gol.cameraEye = { 8.0f, -19.0f, 1.0f + 40.0f * d };
	}
}

void LevelSelector::finishCurrentLevel(const bool success)
{
	// Set level state as "Finished"
	mLevelState = GO_LS_LEVEL_FINISHED;

	// Update text
	mLevelTexts[GO_LS_TEXT_LEVEL]->setText("Level " + std::to_string(mSelectedLevelId) + " " + (success ? "Completed" : "Failed"));
}