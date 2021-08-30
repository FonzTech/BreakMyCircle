#include "LevelSelector.h"

#include <utility>
#include <Magnum/Math/Math.h>
#include <Magnum/Math/Constants.h>
#include <Magnum/Math/Bezier.h>
#include <Magnum/GL/Renderer.h>

#include "../RoomManager.h"
#include "../InputManager.h"
#include "../AssetManager.h"
#include "../Common/CommonUtility.h"
#include "../Common/CustomRenderers/LSNumberRenderer.h"
#include "../Game/Player.h"
#include "../Game/LimitLine.h"
#include "../Game/Bubble.h"

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
		Vector3(-4.86925f, 1.17022f, -3.6977f)
	}),
	std::make_pair(1, std::array<Vector3, 6>{
		Vector3(6.9727f, 0.3f, 20.1046),
		Vector3(0.221052f, 0.3f, 12.0666),
		Vector3(-6.41381f, 0.3f, 5.51524f),
		Vector3(-7.93295f, 0.3f, -4.16931f),
		Vector3(-1.2867f, 0.3f, -11.4327f),
		Vector3(5.35956f, 0.3f, -19.2658f)
	}),
	std::make_pair(2, std::array<Vector3, 6>{
		Vector3(6.9727f, 0.3f, 20.1046),
		Vector3(0.221052f, 0.3f, 12.0666),
		Vector3(-6.41381f, 0.3f, 5.51524f),
		Vector3(-7.93295f, 0.3f, -4.16931f),
		Vector3(-1.2867f, 0.3f, -11.4327f),
		Vector3(5.35956f, 0.3f, -19.2658f)
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

	// Level info
	{
		mLevelInfo.currentViewingLevelId = 0U;
		mLevelInfo.state = GO_LS_LEVEL_INIT;
		mLevelInfo.nextLevelAnim = 0.0f;
		mLevelInfo.numberOfRetries = 0;
		mLevelInfo.score = 0;
		mLevelInfo.lastLevelPos = Vector3(0.0f);
	}

	// Cached variables for GUI
	{
		mTimer = { 0.0f, 0 };
		mCoins = { 0.0f, 0 };
	}

	// Powerup view
	{
		mPuView.startX = GO_LS_RESET_MOUSE_VALUE;
		mPuView.scrollX = 0.0f;

		for (UnsignedInt i = 0; i < GO_LS_MAX_POWERUP_COUNT; ++i)
		{
			mPuView.counts[GO_LS_GUI_POWERUP + i] = { 0, -1 };
		}
	}

	// Animation factors
	for (UnsignedInt i = 0; i < sizeof(mLevelGuiAnim) / sizeof(mLevelGuiAnim[0]); ++i)
	{
		mLevelGuiAnim[i] = 0.0f;
	}

	// Create sky plane
	createSkyPlane();

	// Create overlay eye-candy drawables
	mLevelAnim = 0.0f;

	// Create GUIs
	createGuis();

	// Create texts
	createTexts();

	// Create powerup view
	createPowerupView();

	// Set camera parameters
	{
		const auto& ar = QuadraticBezier2D(Vector2(0.0f), Vector2(-0.5f, 1.0f), Vector2(1.0f)).value(RoomManager::singleton->getWindowAspectRatio())[1];
		auto& layer = RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST];
		layer.cameraEye = mPosition + Vector3(0.0f, 16.0f * ar, 20.0f * ar);
		layer.cameraTarget = mPosition;
	}

	// Load audios
	{
		const std::unordered_map<Int, std::string> tmpMap = {
			{ GO_LS_AUDIO_WIN, RESOURCE_AUDIO_SHOT_WIN },
			{ GO_LS_AUDIO_LOSE, RESOURCE_AUDIO_SHOT_LOSE },
			{ GO_LS_AUDIO_POWERUP, RESOURCE_AUDIO_POWERUP },
			{ GO_LS_AUDIO_PAUSE_IN, RESOURCE_AUDIO_PAUSE_IN },
			{ GO_LS_AUDIO_PAUSE_OUT, RESOURCE_AUDIO_PAUSE_OUT },
			{ GO_LS_AUDIO_EXPLOSION, RESOURCE_AUDIO_EXPLOSION },
			{ GO_LS_AUDIO_STAR, RESOURCE_AUDIO_STAR_PREFIX + std::string("1") },
			{ GO_LS_AUDIO_STAR + 1, RESOURCE_AUDIO_STAR_PREFIX + std::string("2") },
			{ GO_LS_AUDIO_STAR + 2, RESOURCE_AUDIO_STAR_PREFIX + std::string("3") }
		};

		for (const auto& it : tmpMap)
		{
			Resource<Audio::Buffer> buffer = CommonUtility::singleton->loadAudioData(it.second);
			mPlayables[it.first] = std::make_shared<Audio::Playable3D>(*mManipulator.get(), &RoomManager::singleton->mAudioPlayables);
			mPlayables[it.first]->source()
				.setBuffer(buffer)
				.setLooping(false);
		}
	}

	// Trigger scenery creation
	handleScrollableScenery();
}

LevelSelector::~LevelSelector()
{
	// Destroy all of the owned game objects
	for (auto& item : mLevelGuis)
	{
		item.second->mDestroyMe = true;
	}

	for (auto& item : mLevelTexts)
	{
		item.second->mDestroyMe = true;
	}

	for (auto& item : mSceneries)
	{
		item.second.scenery->mDestroyMe = true;
	}

	for (auto& item : mScreenButtons)
	{
		item.second.drawable->mDestroyMe = true;
	}

	// Destroy dialog, if present
	if (!mDialog.expired())
	{
		mDialog.lock()->mDestroyMe = true;
		mDialog.reset();
	}

	// Clear all maps
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
	// Handle white glow
	{
		Float* c = mLevelGuis[GO_LS_GUI_WHITEGLOW]->color();
		if (c[3] > 0.0f)
		{
			c[3] -= mDeltaTime;
			if (c[3] < 0.0f)
			{
				c[3] = 0.0f;
			}
		}
	}

	// Manage player "can shoot" state
	if (!mLevelInfo.playerPointer.expired())
	{
		Int canShoot = 0;
		if (!mDialog.expired())
		{
			canShoot = 2;
		}
		else if (mLevelInfo.state == GO_LS_LEVEL_STARTED && !mLevelInfo.playerPointer.expired())
		{
			canShoot = !mSettingsOpened && mSettingsAnim <= 0.001f ? 1 : 2;
		}

		// Enable or disable player shooting
		if (canShoot != 0)
		{
			((std::shared_ptr<Player>&)mLevelInfo.playerPointer.lock())->mCanShoot = canShoot == 1;
		}
	}

	// Check if any dialog is active
	if (!mDialog.expired())
	{
		return;
	}

	// Set light position for all sceneries
	for (auto& item : mSceneries)
	{
		item.second.scenery->setLightPosition(mPosition);
	}

	// Manage level state
	manageLevelState();

#if NDEBUG or _DEBUG
	if (InputManager::singleton->mMouseStates[ImMouseButtons::Right] == IM_STATE_RELEASED)
	{
		if (mLevelGuiAnim[0] == 0.0f)
		{
			for (auto& item : *RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].list)
			{
				item->mDestroyMe = true;
			}
		}
		else if (mLevelInfo.playerPointer.expired())
		{
			Debug{} << "Level room created";
			mLevelInfo.currentViewingLevelId = 1U;
			mScreenButtons[GO_LS_GUI_PLAY].callback(GO_LS_GUI_PLAY);
		}
		else
		{
			Debug{} << "Level state to Finished SUCCESS";
			finishCurrentLevel(true);
		}
	}
#endif

#ifdef GO_LS_SKY_PLANE_ENABLED
	// Update sky plane
	(*mSkyManipulator)
		.resetTransformation()
		.scale(Vector3(GO_LS_SCENERY_LENGTH, GO_LS_SCENERY_LENGTH, 1.0f))
		.translate(mPosition + Vector3(0.0f, 0.0f, -GO_LS_SKYPLANE_DISTANCE));
#endif

	// Check if there is any on-going action on top
	if (mLevelInfo.state == GO_LS_LEVEL_INIT && RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].list->size() > 0)
	{
		return;
	}

	// Overlay for common
	windowForCommon();

	// Overlay for settings
	manageBackendAnimationVariable(mSettingsAnim, 0.8f, mSettingsOpened && !mLevelEndingAnim);
	windowForSettings();

	// Overlay for current level viewing
	const bool& isViewingLevel = mLevelInfo.currentViewingLevelId != 0U || (mLevelInfo.state == GO_LS_LEVEL_FINISHED && mLevelInfo.repeatLevelId == 0U);
	const bool& isViewingSettings = mLevelInfo.state == GO_LS_LEVEL_STARTED && mSettingsOpened;
	manageBackendAnimationVariable(mLevelAnim, 0.8f, isViewingLevel);

	if (isViewingLevel || isViewingSettings)
	{
		const auto& ar = RoomManager::singleton->getWindowAspectRatio();
		const auto& lbs = InputManager::singleton->mMouseStates[ImMouseButtons::Left];

		if (lbs == IM_STATE_PRESSED)
		{
			if (mLevelInfo.state == GO_LS_LEVEL_INIT || isViewingSettings)
			{
				const auto& y = Float(InputManager::singleton->mMousePosition.y());
				const auto& bbox = mScreenButtons[GO_LS_GUI_POWERUP].drawable->getBoundingBox(RoomManager::singleton->getWindowSize());

				if (y > bbox.backBottomLeft().y() && y < bbox.backTopLeft().y())
				{
					mPuView.startX = InputManager::singleton->mMousePosition.x();
				}
			}
		}
		else if (lbs >= IM_STATE_PRESSING)
		{
			if (mPuView.startX != GO_LS_RESET_MOUSE_VALUE)
			{
				mPuView.scrollX += Float(InputManager::singleton->mMousePosition.x() - mPuView.startX) * 0.005f;
				mPuView.startX = InputManager::singleton->mMousePosition.x();
			}
		}
		else if (lbs == IM_STATE_RELEASED)
		{
			mPuView.startX = GO_LS_RESET_MOUSE_VALUE;
		}
		else
		{
			const Float xf = 0.5f - 0.25f * ar;
			const Float xi = Math::floor((mPuView.scrollX + xf * 0.5f) / xf);

			const Float xt = Math::clamp(xi * xf, Float(-GO_LS_MAX_POWERUP_COUNT + 1) * xf, 0.0f);
			const Float d = xt - mPuView.scrollX;
			mPuView.scrollX += d * 0.25f;
		}
	}
	else
	{
		mPuView.startX = GO_LS_RESET_MOUSE_VALUE;
	}

	windowForCurrentLevelView();

	// Control level ending
	if (mLevelEndingAnim)
	{
		bool closeAll = false;

		// Check for animation end for "Level" window
		if (mLevelAnim <= 0.0f)
		{
			// Repeat level, if required
			if (mLevelInfo.repeatLevelId != 0U)
			{
				// Prepare for replay
				if (mLevelInfo.state == GO_LS_LEVEL_FINISHED)
				{
					closeAll = true;
					prepareForReplay();
				}
			}
		}

		// Check for animation end for "Settings" window
		if (!closeAll && mSettingsAnim <= 0.0f)
		{
			// Repeat level, if required
			if (mLevelInfo.repeatLevelId != 0U)
			{
				// Prepare for replay
				if (mLevelInfo.state == GO_LS_LEVEL_STARTED)
				{
					closeAll = true;
					prepareForReplay();
				}
			}
			else
			{
				closeAll = true;
				finishCurrentLevel(false); // Finish current level with "Fail" message
			}
		}

		if (closeAll)
		{
			// Close settings
			mSettingsOpened = false;

			// End this transition
			mLevelEndingAnim = false;
		}
	}

	/*
	// Animation for overlay buttons
	for (auto it = mScreenButtons.begin(); it != mScreenButtons.end(); ++it)
	{
		it->second.animation += mDeltaTime;
		if (it->second.animation > 1.0f)
		{
			it->second.animation = 1.0f;
		}
	}
	*/

	// Handle button clicks
	const auto& lbs = InputManager::singleton->mMouseStates[ImMouseButtons::Left];

	const Vector3 p(Float(InputManager::singleton->mMousePosition.x()), Float(InputManager::singleton->mMousePosition.y()), 0.0f);
	const auto& w = RoomManager::singleton->getWindowSize();

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
			else if (lbs == IM_STATE_RELEASED && mClickIndex == it->first && it->second.callback(it->first))
			{
				break;
			}
		}
	}

	// Handle button scale animation 
	{
		// Handle scale value
		{
			const bool& mode = mLevelInfo.state == GO_LS_LEVEL_INIT;
			manageBackendAnimationVariable(mLevelButtonScaleAnim, mode ? 1.0f : 0.8f, mode);
		}

		// Apply transformations to all button drawables
		for (auto it = mSceneries.begin(); it != mSceneries.end(); ++it)
		{
			for (auto it2 = it->second.buttons.begin(); it2 != it->second.buttons.end(); ++it2)
			{
				// Set last level position
				if (it2->levelIndex == RoomManager::singleton->mSaveData.maxLevelId - 1)
				{
					mLevelInfo.lastLevelPos = it->second.scenery->mPosition + it2->position;
				}

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
		if (!isViewingLevel && !mSettingsOpened && mLevelInfo.state == GO_LS_LEVEL_INIT && !mLevelEndingAnim)
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
						const auto& it2 = mSceneries.find(it->second.sceneryIndex);
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
								if (spo->levelIndex < RoomManager::singleton->mSaveData.maxLevelId)
								{
									// Open level window
									clickLevelButton(&it2->second, spo);

									// Play sound
									playSfxAudio(GO_LS_AUDIO_PAUSE_OUT);
								}
								else
								{
									Debug{} << "Max level ID is" << RoomManager::singleton->mSaveData.maxLevelId << "and user has selected level ID" << spo->levelIndex;
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

		// Scroll scenery
		handleScrollableScenery();
	}
}

void LevelSelector::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
#ifdef GO_LS_SKY_PLANE_ENABLED
	if (baseDrawable == mSkyPlane.get())
	{
		((Shaders::Flat3D&)baseDrawable->getShader())
			.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
			.bindTexture(*baseDrawable->mTexture)
			.setColor(Color4{ 1.0f })
			.draw(*baseDrawable->mMesh);
	}
	else
#endif
	{
		UnsignedInt levelIndex = 0U;
		const auto& it = mPickableObjectRefs.find(baseDrawable->getObjectId());
		if (it != mPickableObjectRefs.end())
		{
			levelIndex = mSceneries[it->second.sceneryIndex].buttons[it->second.objectIndex].levelIndex;
		}

		const auto& color = levelIndex < RoomManager::singleton->mSaveData.maxLevelId ? 0xc0c0c0_rgbf : 0x404040_rgbf;
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

void LevelSelector::shootCallback(const Int state, const Color3 & preColor, const Color3 & postColor)
{
	switch (state)
	{
	case ISC_STATE_SHOOT_STARTED:
		break;

	case ISC_STATE_SHOOT_FINISHED:
		/*
			We have to delay the "Level Fail" state, because when a bubble is hit
			when a group is at the bottom, the game will think you failed the level.
			Instead, we waste "just a single" cycle to let the bubble explode, then
			make all the required checks to decide if the player has failed the level.
		*/
		mLevelInfo.delayedChecks = true;

		// Launch white glow
		if (preColor == BUBBLE_ELECTRIC)
		{
			mLevelGuis[GO_LS_GUI_WHITEGLOW]->color()[3] = 1.0f;
			playSfxAudio(GO_LS_AUDIO_EXPLOSION);
		}

		break;
	}
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
#ifdef GO_LS_SKY_PLANE_ENABLED
	Resource<GL::Mesh> resMesh = CommonUtility::singleton->getPlaneMeshForSpecializedShader<Shaders::Flat3D>(RESOURCE_MESH_PLANE_FLAT);
	Resource<GL::Texture2D> resTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_SKYBOX_1_PX);
	Resource<GL::AbstractShaderProgram, Shaders::Flat3D> resShader = CommonUtility::singleton->getFlat3DShader();

	mSkyManipulator = new Object3D{ mManipulator.get() };

	auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
	mSkyPlane = std::make_shared<GameDrawable<Shaders::Flat3D>>(*drawables, resShader, resMesh, resTexture);
	mSkyPlane->setParent(mSkyManipulator);
	mSkyPlane->setDrawCallback(this);
	mDrawables.emplace_back(mSkyPlane);
#endif
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
		const Int modelIndex = Int(Math::floor(Float(Math::abs(yp)) / 2.0f)) % 3;

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
			const UnsignedInt objectId = UnsignedInt(-yp) * 6U + UnsignedInt(i + 1) + 100U;
			const UnsignedInt levelIndex = objectId - 100U;

			mPickableObjectRefs[objectId] = { yp, Int(mSceneries[yp].buttons.size()) - 1 };

			bs.levelIndex = levelIndex;
			bs.objectId = objectId;
			bs.scale = 0.0f;

			// Create and save texture
			{
				const auto& nt = std::to_string(Int(bs.levelIndex));
				const auto& key = "tex_ls_" + nt;

				bs.texture = CommonUtility::singleton->manager.get<GL::Texture2D>(key);
				if (!bs.texture)
				{
					// Create renderer
					LSNumberRenderer nr(Vector2i(32), nt);

					// Render to texture
					// GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::OneMinusSourceAlpha, GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::DestinationAlpha);
					nr.renderTexture();
					GL::Texture2D & texture = nr.getRenderedTexture();
					// GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha, GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::DestinationAlpha);

					// Save to resource manager
					CommonUtility::singleton->manager.set(key, std::move(texture));
				}
			}

			// Load drawables
			AssetManager am(RESOURCE_SHADER_COLORED_PHONG, RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE, 1);
			am.loadAssets(*this, *mSceneries[yp].manipulator, RESOURCE_SCENE_LEVEL_BUTTON, this);

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

void LevelSelector::clickLevelButton(const LS_ScenerySelector * sc, const LS_PickableObject * po)
{
	Debug{} << "You have clicked level" << po->levelIndex;

	// Set current level identifier
	mLevelInfo.currentViewingLevelId = po->levelIndex;
	mLevelInfo.selectedLevelId = po->levelIndex;

	// Set text for selected level
	mLevelTexts[GO_LS_TEXT_LEVEL]->setText("Level " + std::to_string(po->levelIndex));

	// Set parameters for animation
	mLevelInfo.currentLevelPos = mPosition;
	mLevelInfo.nextLevelPos = sc->scenery->mPosition + po->position + Vector3(0.0f, 0.0f, -8.0f);
}

void LevelSelector::windowForCommon()
{
	const auto& dsl = mCbEaseInOut.value(mSettingsAnim + mLevelAnim)[1];
	const auto& d0 = mCbEaseInOut.value(mLevelGuiAnim[0])[1];
	const auto& d1 = mCbEaseInOut.value(mLevelGuiAnim[1])[1];
	const auto& ds = mCbEaseInOut.value(mLevelStartedAnim)[1];

	// Main panel
	mLevelGuis[GO_LS_GUI_LEVEL_PANEL]->setPosition({ 0.0f, 1.0f - dsl });

	// Coin icon and text
	{
		const auto& dx = 0.2f * d0;
		mLevelGuis[GO_LS_GUI_COIN]->setPosition({ -0.49f, 0.69f - dx });
		mLevelTexts[GO_LS_TEXT_COIN]->setPosition({ -0.5f, 0.7f - dx });
	}

	// Time icon and text
	{
		const auto& p1 = Vector2(0.49f, -0.74f + ds * 0.25f);
		const auto& p2 = mLevelInfo.state >= GO_LS_LEVEL_FINISHED ? Vector3(0.0f, -0.2f, 0.0f) * d1 : Vector3(0.0f);

		mLevelGuis[GO_LS_GUI_TIME]->setPosition(p1);
		mLevelTexts[GO_LS_TEXT_TIME]->setPosition(p1);
	}

	// Scroll back
	mScreenButtons[GO_LS_GUI_SCROLL_BACK].drawable->setPosition(Vector2(0.5f, -0.75f + 0.25f * mLevelGuiAnim[4]));
}

void LevelSelector::windowForSettings()
{
	const auto& ar = RoomManager::singleton->getWindowAspectRatio();
	const auto& ds = mCbEaseInOut.value(mSettingsAnim)[1];
	const auto& dl = mCbEaseInOut.value(mLevelAnim)[1];

	// Animation for "Settings" button
	{
		const auto& d2 = mCbEaseInOut.value(mLevelGuiAnim[0])[1];
		const auto& dsl = ds + dl;

		const auto& drawable = ((std::shared_ptr<OverlayGui>&)mScreenButtons[GO_LS_GUI_SETTINGS].drawable);

		// Position and Anchor
		{
			const auto& p1 = Vector2(-0.75f, -0.5f) + Vector2(0.25f, 0.0f) * d2;
			const auto& p2 = Vector2(0.5f, 0.8f) * dsl;
			const auto& p3 = mLevelInfo.state >= GO_LS_LEVEL_FINISHED ? Vector2(-0.5f, -1.0f) * dl : Vector2(0.0f);
			drawable->setPosition(p1 + p2 + p3);

			const auto& a1 = Vector2(1.0f);
			const auto& a2 = Vector2(-1.0f) * dsl;
			drawable->setAnchor(a1 + a2);
		}

		// Rotation
		drawable->setRotationInDegrees(360.0f * dsl);

		// Texture change
		const bool isSettings = drawable->getTextureResource().key() == ResourceKey(RESOURCE_TEXTURE_GUI_SETTINGS);
		if (dsl > 0.75f)
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

	// BG Music and SFX buttons
	{
		const Float y = mLevelInfo.state == GO_LS_LEVEL_INIT ? 1.2f - ds * 1.125f : (-1.4f + ds);
		mScreenButtons[GO_LS_GUI_BGMUSIC].drawable->setPosition(Vector2(-0.1f / ar, y));
		mScreenButtons[GO_LS_GUI_SFX].drawable->setPosition(Vector2(0.1f / ar, y));
	}

	// Vote Me text
	{
		const Float y = mLevelInfo.state == GO_LS_LEVEL_INIT ? 1.285f - ds * 1.35f : -2.0f;
		mScreenButtons[GO_LS_TEXT_VOTE_ME].drawable->setPosition(Vector2(0.0f, y));
	}

	// Other Apps text
	{
		const Float y = mLevelInfo.state == GO_LS_LEVEL_INIT ? 1.285f - ds * 1.425f : -2.0f;
		mScreenButtons[GO_LS_TEXT_OTHER_APPS].drawable->setPosition(Vector2(0.0f, y));
	}
}

void LevelSelector::windowForCurrentLevelView()
{
	const auto& ar = RoomManager::singleton->getWindowAspectRatio();

	const bool& isFinished = mLevelInfo.state >= GO_LS_LEVEL_FINISHED;
	const auto& d = mCbEaseInOut.value(mLevelAnim)[1];
	const auto& s = mCbEaseInOut.value(mSettingsAnim)[1];
	//const auto& p = mLevelInfo.state == GO_LS_LEVEL_STARTED ? s * 0.96f : 0.0f;

	// Score stars
	for (UnsignedInt i = 0; i < 3; ++i)
	{
		const auto& drawable = mLevelGuis[GO_LS_GUI_STAR + i];

		const Float xp = 0.2f / (ar / 0.5625f);
		const Float yp = mLevelInfo.score - 1 >= i ? 1.25f - d : 2.0f;
		drawable->setPosition({ -xp + xp * Float(i), yp });

		Float s;
		if (mLevelInfo.state >= GO_LS_LEVEL_FINISHED)
		{
			s = mLevelInfo.score >= i + 1 ? Math::clamp(mLevelGuiAnim[3] - Float(i) * 0.25f - 0.25f, 0.0f, 0.25f) * 0.4f : 0.0f;
		}
		else
		{
			s = mLevelInfo.score >= i + 1 ? 0.1f : 0.0f;
		}
		drawable->setSize(Vector2(s));
	}

	// Play button
	{
		const auto& p1 = isFinished ? Vector2{ 2.0f } : Vector2{ 0.0f, -1.5f + d * 1.25f };
		mScreenButtons[GO_LS_GUI_PLAY].drawable->setPosition(p1);
	}

	// Level text
	{
		const auto& lx = mLevelInfo.state == GO_LS_LEVEL_INIT ? s * 0.95f : s;
		mLevelTexts[GO_LS_TEXT_LEVEL]->setPosition({ 0.0f, 1.175f - d - lx });
	}

	// Powerup title text
	const bool& canShowPowerups = mLevelInfo.state <= GO_LS_LEVEL_STARTING || mLevelInfo.state == GO_LS_LEVEL_STARTED;
	const Float powerupY = mLevelInfo.state <= GO_LS_LEVEL_STARTING ? d : s;
	mLevelTexts[GO_LS_TEXT_POWERUP_TITLE]->setPosition({ canShowPowerups ? 0.0f : -1000.0f, 1.05f - powerupY });

	// Powerup buttons
	if (canShowPowerups)
	{
		const Float xf = 0.5f - 0.25f * ar;
		const Float yf = 0.91f - powerupY;
		for (UnsignedInt i = 0; i < GO_LS_MAX_POWERUP_COUNT; ++i)
		{
			const Float xp = canShowPowerups ? Float(i) * xf + mPuView.scrollX : -1000.0f;

			// Clickable icon
			{
				const Float alpha = Math::clamp(1.2f - Math::abs(xp) * 5.0f * ar, 0.0f, 1.0f);
				auto& item = (std::shared_ptr<OverlayGui>&)mScreenButtons[GO_LS_GUI_POWERUP + i].drawable;
				if (alpha > 0.0f)
				{
					item->setPosition({ xp, yf });
					item->color()[3] = alpha;
				}
				else
				{
					item->setPosition({ -2.0f, yf });
				}
			}

			// Counter
			{
				const Float alpha = Math::clamp(1.0f - Math::abs(xp) * 10.0f * ar, 0.0f, 1.0f);
				auto& item = mLevelTexts[GO_LS_TEXT_POWERUP_COUNT + i];
				if (alpha > 0.0f)
				{
					item->setPosition({ xp + 0.115f / ar, yf - 0.06f });
					item->mColor.data()[3] = alpha;
					item->mOutlineColor.data()[3] = alpha;
				}
				else
				{
					item->setPosition({ -2.0f, yf });
				}
			}
		}
	}

	// Navigation buttons for level
	{
		const bool& isStarted = mLevelInfo.state == GO_LS_LEVEL_STARTED;
		const auto& cd = -1.5f + d * 1.25f;
		const auto& cs = -1.5f + s * 1.25f;
		const auto& xd = 0.15f / ar;

		// Animation for "Replay" button
		{
			const auto& p1 = isFinished ? Vector2{ -xd, cd } : Vector2{ isStarted ? 0.0f : 2.0f };
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
			const auto& p1 = isFinished ? Vector2{ xd, cd } : Vector2{ 2.0f };
			mScreenButtons[GO_LS_GUI_SHARE].drawable->setPosition(p1);
		}

		// Animation for "Exit" button
		{
			const auto& p1 = mLevelInfo.state == GO_LS_LEVEL_STARTED ? Vector2{ -0.2f, cs } : Vector2{ 2.0f };
			mScreenButtons[GO_LS_GUI_EXIT].drawable->setPosition(p1);
		}
	}
}

void LevelSelector::manageLevelState()
{
	// Variables for later
	bool animate[3] = { false, false, false };

	// Control level state
	switch (mLevelInfo.state)
	{
	case GO_LS_LEVEL_INIT:

		// Animations
		if (mLevelButtonScaleAnim > 0.0f)
		{
			manageGuiLevelAnim(0, true);
		}

		// Animate when a level is clicked
		if (mLevelInfo.currentViewingLevelId != 0U)
		{
			const auto oldPosition = mPosition;
			mPosition = mLevelInfo.currentLevelPos + (mLevelInfo.nextLevelPos - mLevelInfo.currentLevelPos) * mLevelAnim;
			handleScrollableCameraPosition(mPosition - oldPosition);
			handleScrollableScenery();
		}

		// Check for distance between last level and current scroll position
		animate[2] = Math::abs((mPosition - mLevelInfo.currentLevelPos).length()) > 50.0f;

		break;

	case GO_LS_LEVEL_STARTING:

		// Create level room on animation end
		if (mLevelButtonScaleAnim <= 0.0f)
		{
			createLevelRoom();
		}

		break;

	case GO_LS_LEVEL_STARTED:

		// Animate camera
		animate[1] = true;

		// Decrement timer
		if (mTimer.value < 0.0f)
		{
			if (mLevelInfo.playerPointer.expired() || ((std::shared_ptr<Player>&)mLevelInfo.playerPointer.lock())->mCanShoot)
			{
				checkForLevelEnd();
			}
		}
		else if (!mSettingsOpened)
		{
			// Decrement only when "Start Level" animation has finished
			if (mLevelStartedAnim >= 1.0f && mTimer.value >= -1500.0f)
			{
				mTimer.value -= mDeltaTime;
			}
		}

		// Control animation
		if (mLevelStartedAnim < 0.0f) // Cycle waste
		{
			mLevelStartedAnim = 0.0f;
		}
		else
		{
			manageBackendAnimationVariable(mLevelStartedAnim, 1.0f, !mLevelEndingAnim);
		}

		// Update timer text
		{
			const Int timerInt(Math::floor(mTimer.value));
			if (mTimer.cached != timerInt)
			{
				// Update time counter
				mTimer.cached = timerInt;
				updateTimeCounter(mTimer.cached);

				// Update animation factor
				mLevelGuiAnim[2] = 1.0f;
			}
		}

		// Finish this level (after I wasted a cycle)
		if (mLevelInfo.delayedChecks)
		{
			mLevelInfo.delayedChecks = false;
			checkForLevelEnd();	
		}

		// Manage GUI level animation for timer
		manageBackendAnimationVariable(mLevelGuiAnim[2], 1.0f, false);

		{
			const Float x = mTimer.cached >= 0 && mTimer.cached <= 15 ? 1.0f - mLevelGuiAnim[2] * 0.5f : 1.0f;
			const auto& c = mLevelTexts[GO_LS_TEXT_TIME]->mColor.data();
			c[1] = x;
			c[2] = x;
		}

		break;

	case GO_LS_LEVEL_FINISHED:

		// Check for second layer (the gameplay one)
		if (mLevelStartedAnim <= 0.0f)
		{
			// Reset pointers
			mLevelInfo.playerPointer.reset();
			mLevelInfo.limitLinePointer.reset();

			// Delete all objects from the second layer
			for (auto& go : *RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].list)
			{
				go->mDestroyMe = true;
			}
		}

		// Animate camera
		animate[0] = true;
		animate[1] = true;

		// Animate stars
		if (mLevelGuiAnim[1] >= 0.99f)
		{
			manageGuiLevelAnim(3, true, 0.5f);

			// Play sound effects
			if (mLevelGuiAnim[3] >= 0.875f)
			{
				if (mLevelInfo.playedScore == 2)
				{
					playSfxAudio(GO_LS_AUDIO_STAR + 2);
					++mLevelInfo.playedScore;
				}
			}
			else if (mLevelGuiAnim[3] >= 0.625f)
			{
				if (mLevelInfo.playedScore == 1)
				{
					playSfxAudio(GO_LS_AUDIO_STAR + 1);
					++mLevelInfo.playedScore;
				}
			}
			else if (mLevelGuiAnim[3] >= 0.375f)
			{
				if (mLevelInfo.playedScore == 0)
				{
					playSfxAudio(GO_LS_AUDIO_STAR);
					++mLevelInfo.playedScore;
				}
			}
		}

		// Prevent player from shooting
		if (!mLevelInfo.playerPointer.expired())
		{
			// Prevent player from shooting
			((std::shared_ptr<Player>&)mLevelInfo.playerPointer.lock())->mCanShoot = false;
		}

		// Control animation
		manageBackendAnimationVariable(mLevelStartedAnim, 1.0f, false);

		break;

	case GO_LS_LEVEL_RESTORING:

		// Reset level state when animation has finished
		if (mLevelAnim <= 0.0f)
		{
			// Reset level state
			mLevelInfo.state = GO_LS_LEVEL_INIT;
		}

		// Animate jump to new level
		if (mLevelInfo.success)
		{
			const auto oldPosition = mPosition;
			mPosition = mLevelInfo.currentLevelPos + (mLevelInfo.nextLevelPos - mLevelInfo.currentLevelPos) * (1.0f - mLevelAnim);
			handleScrollableCameraPosition(mPosition - oldPosition);
			handleScrollableScenery();
		}

		break;
	}

	// Update coin counter
	{
		const auto& value = RoomManager::singleton->mSaveData.coinTotal + RoomManager::singleton->mSaveData.coinCurrent;
		if (mCoins.cached != value)
		{
			mCoins.value += (Float(value) - mCoins.value) * 0.25f;
			mCoins.cached = Int(mCoins.value + 0.01f); // Add a small factor to avoid floating-point precision errors

			const auto& str = std::to_string(mCoins.cached);
			mLevelTexts[GO_LS_TEXT_COIN]->setText(str);
		}
	}

	// Update powerup view counters
	if (mSettingsAnim > 0.01f || mLevelAnim > 0.01f)
	{
		const auto& pm = RoomManager::singleton->mSaveData.powerupAmounts;
		for (UnsignedInt i = 0; i < GO_LS_MAX_POWERUP_COUNT; ++i)
		{
			const auto& key = GO_LS_GUI_POWERUP + i;

			mPuView.counts[key].value = RoomManager::singleton->mSaveData.powerupAmounts[key];
			if (mPuView.counts[key].cached != mPuView.counts[key].value)
			{
				mPuView.counts[key].cached = mPuView.counts[key].value;
				mLevelTexts[key]->setText(std::to_string(mPuView.counts[key].cached) + "x");
			}
		}
	}

	// Animate level GUIs
	manageGuiLevelAnim(1, animate[0]);
	manageGuiLevelAnim(4, animate[2]);

	// Animate camera, if requested
	if (animate[1])
	{
		// Animation value
		const auto& d = mCbEaseInOut.value(mLevelStartedAnim)[1];

		// Get second perspective layer by reference
		auto& gol = RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND];

		// Move Z position by a certain animated value
		if (!mLevelInfo.playerPointer.expired())
		{
			const Float zd = ((std::shared_ptr<Player>&)mLevelInfo.playerPointer)->mCameraDist;
			gol.cameraEye.data()[2] = 1.0f + zd * d;
		}
	}
}

void LevelSelector::createLevelRoom()
{
	// Debug print
	Debug{} << "Creating level room for" << mLevelInfo.selectedLevelId;

	// Compute right difficulty factor
	const int32_t difficulty = 8;

	// Level is started
	RoomManager::singleton->createLevelRoom(shared_from_this(), 8, 7, difficulty);
	mLevelInfo.state = GO_LS_LEVEL_STARTED;
	mLevelInfo.score = 0;
	mLevelInfo.difficulty = difficulty;

	// Get player pointer
	for (const auto& go : *RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].list)
	{
		switch (go->getType())
		{
		case GOT_PLAYER:
			mLevelInfo.playerPointer = go;
			break;

		case GOT_LIMIT_LINE:
		{
			const std::shared_ptr<LimitLine>& line = (std::shared_ptr<LimitLine>&)go;
			if (line->getCustomType() == GO_LL_TYPE_RED)
			{
				mLevelInfo.limitLinePointer = go;
			}
		}
			break;

		default:
			if (!mLevelInfo.playerPointer.expired() && !mLevelInfo.limitLinePointer.expired())
			{
				break;
			}
		}
	}

	if (mLevelInfo.playerPointer.expired())
	{
		Error{} << "The player game object was not found after creating the room";
	}

	// Other variables
	mLevelStartedAnim = -1.0f; // Cycle waste
	mLevelGuiAnim[1] = -5.0f; // Cycle waste
}

void LevelSelector::finishCurrentLevel(const bool success)
{
	// Update level state
	mLevelInfo.success = success;

	// Add coins
	if (mLevelInfo.success)
	{
		// Reset number of retries
		mLevelInfo.numberOfRetries = 0;

		// Increment maximum level ID
		if (mLevelInfo.selectedLevelId == RoomManager::singleton->mSaveData.maxLevelId - 1)
		{
			++RoomManager::singleton->mSaveData.maxLevelId;
		}

		// Add coins picked-up in this level to total count
		RoomManager::singleton->mSaveData.coinTotal += RoomManager::singleton->mSaveData.coinCurrent;

		// Compute correct score
		mLevelInfo.score = computeScore();
		mLevelInfo.playedScore = 0;
	}

	// Reset temporary stats
	RoomManager::singleton->mSaveData.coinCurrent = 0;

	// Play success or failure sound for level end
	playSfxAudio(mLevelInfo.success ? GO_LS_AUDIO_WIN : GO_LS_AUDIO_LOSE);

	// Prevent player from shooting
	if (!mLevelInfo.playerPointer.expired())
	{
		((std::shared_ptr<Player>&)mLevelInfo.playerPointer.lock())->mCanShoot = false;
	}

	// Set level state as "Finished"
	mLevelInfo.state = GO_LS_LEVEL_FINISHED;

	// Update text
	mLevelTexts[GO_LS_TEXT_LEVEL]->setText("Level " + std::to_string(mLevelInfo.selectedLevelId) + "\n" + (mLevelInfo.success ? "Completed" : "Failed"));
}

void LevelSelector::prepareForReplay()
{
	// Reset texts
	mLevelTexts[GO_LS_TEXT_LEVEL]->setText("Level " + std::to_string(mLevelInfo.repeatLevelId));

	// Reset variable for "current viewing level window"
	mLevelInfo.currentViewingLevelId = mLevelInfo.repeatLevelId;

	// Trigger the level start for the same level
	mLevelInfo.state = GO_LS_LEVEL_INIT;

	++mLevelInfo.numberOfRetries;
	startLevel(mLevelInfo.currentViewingLevelId);
}

void LevelSelector::replayCurrentLevel()
{
	// Avoid multiple calls
	if (mSettingsAnim < 1.0f && mLevelAnim < 1.0f)
	{
		return;
	}

	// Restore state
	mSettingsOpened = false;
	mLevelInfo.repeatLevelId = mLevelInfo.selectedLevelId;

	// Trigger animation
	mLevelEndingAnim = true;
}

void LevelSelector::checkForLevelEnd()
{
	// Check for consistency
	if (mLevelInfo.playerPointer.expired())
	{
		Error{} << "weak_ptr for Player has expired. This should not happen.";
	}

	// Check if any bubble has reached the limit line
	bool lose = mTimer.value >= -1000.0f && mTimer.value < 0.0f;
	bool win = !lose;

	if (mLevelInfo.limitLinePointer.expired())
	{
		Error{} << "weak_ptr for LimitLine has expired. This should not happen.";
	}
	else if (!lose)
	{
		for (auto& go : *RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].list)
		{
			if (go->getType() == GOT_BUBBLE)
			{
				// Level is not won if there is one bubble at least
				win = false;

				// Check if there is any bubble below the red line
				if (go->mPosition.y() < mLevelInfo.limitLinePointer.lock()->mPosition.y())
				{
					lose = true;
					break;
				}
			}
		}
	}

	// Finish current level, if required
	if (lose)
	{
		finishCurrentLevel(false);
	}
	else if (win)
	{
		finishCurrentLevel(true);
	}
}

void LevelSelector::startLevel(const UnsignedInt levelId)
{
	// Check if a level is starting
	if (mLevelInfo.state > GO_LS_LEVEL_INIT)
	{
		return;
	}

	// Start the selected level
	Debug{} << "User wants to play level" << levelId;

	// Play sound
	playSfxAudio(GO_LS_AUDIO_PAUSE_OUT);

	// Set state for level

	mLevelEndingAnim = false;

	mLevelInfo.currentViewingLevelId = 0U;
	mLevelInfo.repeatLevelId = 0U;
	mLevelInfo.delayedChecks = false;
	mLevelInfo.state = GO_LS_LEVEL_STARTING;
	mLevelInfo.startingTime = 120.0f;

	mTimer = { mLevelInfo.startingTime, Int(mLevelInfo.startingTime) };
	// mCoins = { -0.001f, -1 };
	RoomManager::singleton->mSaveData.coinCurrent = 0;

	updateTimeCounter(mTimer.cached);

	// Reset GUI animation factors
	mLevelGuiAnim[3] = 0.0f;
}

Int LevelSelector::computeScore()
{
	const Float d(mLevelInfo.difficulty);
	if (mTimer.value > mLevelInfo.startingTime - d * 4.0f)
	{
		return 3;
	}
	else if (mTimer.value > mLevelInfo.startingTime - d * 2.0f)
	{
		return 2;
	}
	return 1;
}

void LevelSelector::manageGuiLevelAnim(const UnsignedInt index, const bool increment, const Float factor)
{
	if (mLevelGuiAnim[index] < 0.0f)
	{
		mLevelGuiAnim[index] += 1.0f;
	}
	else
	{
		manageBackendAnimationVariable(mLevelGuiAnim[index], factor, increment);
	}
}

void LevelSelector::updateTimeCounter(const Int value)
{
	const auto& str = mTimer.value <= -1999.0f ? "stop" : value >= 0 ? std::to_string(value) + "s" : ":(";
	mLevelTexts[GO_LS_TEXT_TIME]->setText(str);
}

void LevelSelector::closeDialog()
{
	if (mDialog.expired())
	{
		Debug{} << "Dialog pointer has already expired";
	}
	else
	{
		mDialog.lock()->closeDialog();
		mDialog.reset();
	}
}

void LevelSelector::createPowerupView()
{
	for (UnsignedInt i = 0; i < GO_LS_MAX_POWERUP_COUNT; ++i)
	{
		// Get correct icon for this powerup
		std::string tn;
		switch (i)
		{
		case 0:
			tn = RESOURCE_TEXTURE_GUI_PU_BOMB;
			break;

		case 1:
			tn = RESOURCE_TEXTURE_GUI_PU_PLASMA;
			break;

		case 2:
			tn = RESOURCE_TEXTURE_GUI_PU_TIME;
			break;

		case 3:
			tn = RESOURCE_TEXTURE_GUI_PU_ELECTRIC;
			break;
		}

		// Clickable icon
		{
			const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, tn);
			o->setPosition({ 2.0f, 2.0f });
			o->setSize({ 0.15f, 0.15f });
			o->setAnchor({ 0.0f, 0.0f });

			mScreenButtons[GO_LS_GUI_POWERUP + i] = {
				(std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true),
				[&](UnsignedInt index) {
					if ((mLevelAnim < 0.95f && mSettingsAnim < 0.95f) || ((std::shared_ptr<OverlayGui>&)mScreenButtons[index].drawable)->color()[3] < 0.95f || !mDialog.expired())
					{
						return false;
					}

					Debug{} << "You have clicked POWERUP" << index;

					// Build dialog
					std::string title, message;
					switch (index)
					{
					case GO_LS_GUI_POWERUP:
						title = "BOMB POWERUP";
						message = "Use a bomb as a\nprojectile. Make your\nway through lots of\nbubbles and such.";
						break;

					case GO_LS_GUI_POWERUP + 1:
						title = "PLASMA POWERUP";
						message = "Multi-color projectile\nwhich changes color\nonce it hits a bubble.";
						break;

					case GO_LS_GUI_POWERUP + 2:
						title = "TIME POWERUP";
						message = "Freeze the time\nfor the entire\nlevel. Do whatever\nyou want.";
						break;

					case GO_LS_GUI_POWERUP + 3:
						title = "ELECTRIC POWERUP";
						message = "Delete up to three\nbubbles with the\nsame color once it\nhits a bubble.";
						break;
					}

					message = title + "\n\n" + message + "\n\nYou can earn more\npowerups by watching\nrewarded video ads.";
					const std::shared_ptr<Dialog> o = std::make_shared<Dialog>(GOL_ORTHO_FIRST, message.length());
					o->setTextPosition({ 0.0f, 0.4f, 0.0f });
					o->setMessage(message);

					if (mLevelInfo.state == GO_LS_LEVEL_STARTED)
					{
						const bool& isEnough = RoomManager::singleton->mSaveData.powerupAmounts[index] > 0;
						const std::string& text = isEnough ? "Yes" : "Watch Ad";
						o->addAction(text, [this,index]() {
							Debug{} << "You have clicked YES to USE POWERUP";

							// Check if there is one left to use, at least
							auto& pm = RoomManager::singleton->mSaveData.powerupAmounts;
							const auto& it = pm.find(index);
							if (it->second <= 0)
							{
								watchAdForPowerup(index);
								return;
							}

							// Use powerup
							pm[it->first] = it->second - 1;
							usePowerup(index);

							// Play sound
							playSfxAudio(GO_LS_AUDIO_POWERUP);

							// Close dialog and settings window
							closeDialog();
							mScreenButtons[GO_LS_GUI_SETTINGS].callback(GO_LS_GUI_SETTINGS);
						},
							Vector3(0.0f, -0.1f, 0.0f)
						);
					}

					{
						const bool isNo = mLevelInfo.state == GO_LS_LEVEL_STARTED;
						const std::string & text = isNo ? "No" : "OK";
						o->addAction(text, [this]() {
							Debug{} << "You have clicked NO to USE POWERUP";
							closeDialog();
						},
							Vector3(0.0f, isNo ? -0.1f : -0.2f, 0.0f)
						);
					}

					mDialog = (std::shared_ptr<Dialog>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true);
					return true;
				}
			};
		}

		// Counter
		{
			const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::MiddleRight, 10);
			go->mPosition = Vector3(2.0f, 2.0f, 0.0f);
			go->mColor = Color4(1.0f, 1.0f, 1.0f, 1.0f);
			go->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
			go->setSize(Vector2(1.0f));
			go->setText("-");

			mLevelTexts[GO_LS_TEXT_POWERUP_COUNT + i] = (std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(go, true);
		}
	}
}

void LevelSelector::usePowerup(const UnsignedInt index)
{
	Debug{} << "Performing action for powerup" << index;

	if (mLevelInfo.playerPointer.expired())
	{
		Error{} << "Player pointer has expired. This should not happen";
		return;
	}

	const std::shared_ptr<Player> player = (std::shared_ptr<Player>&)mLevelInfo.playerPointer.lock();

	switch (index)
	{
	case GO_LS_GUI_POWERUP:
		player->setPrimaryProjectile(BUBBLE_BOMB);
		break;

	case GO_LS_GUI_POWERUP + 1U:
		player->setPrimaryProjectile(BUBBLE_PLASMA);
		break;

	case GO_LS_GUI_POWERUP + 2U:
		mTimer.value = -2000.0f;
		break;

	case GO_LS_GUI_POWERUP + 3U:
		player->setPrimaryProjectile(BUBBLE_ELECTRIC);
		break;

	default:
		Debug{} << "No action to take for powerup" << index;
	}
}

void LevelSelector::watchAdForPowerup(const UnsignedInt index)
{
	Debug{} << "Trigger REWARDED AD for Powerup" << index;
}

void LevelSelector::createTexts()
{

	// Level number text
	{
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::TopCenter, 40);
		go->mPosition = Vector3(2.0f, 2.0f, 0.0f);
		go->mColor = Color4(1.0f, 1.0f, 1.0f, 1.0f);
		go->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
		go->setSize(Vector2(1.25f));
		go->setText("Settings");

		mLevelTexts[GO_LS_TEXT_LEVEL] = (std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(go, true);
	}

	// Time counter text
	{
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::LineRight, 10);
		go->mPosition = Vector3(2.0f, 2.0f, 0.0f);
		go->mColor = Color4(1.0f, 1.0f, 1.0f, 1.0f);
		go->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
		go->setAnchor(Vector2(-0.9f, 0.65f));
		go->setSize(Vector2(1.125f));
		go->setText("0s");

		mLevelTexts[GO_LS_TEXT_TIME] = (std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(go, true);
	}

	// Coin counter text
	{
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::TopLeft, 40);
		go->mPosition = Vector3(2.0f, 2.0f, 0.0f);
		go->mColor = Color4(1.0f, 1.0f, 1.0f, 1.0f);
		go->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
		go->setAnchor(Vector2(3.5f, -1.0f));
		go->setSize(Vector2(1.15f));
		go->setText("0");

		mLevelTexts[GO_LS_TEXT_COIN] = (std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(go, true);
	}

	// Powerup text
	{
		const std::string& text = "Your Powerups";
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::MiddleCenter, UnsignedInt(text.length()));
		go->mPosition = Vector3(2.0f, 2.0f, 0.0f);
		go->mColor = Color4(0.9f, 0.9f, 0.9f, 1.0f);
		go->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
		go->setSize(Vector2(1.0f));
		go->setText(text);

		mLevelTexts[GO_LS_TEXT_POWERUP_TITLE] = (std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(go, true);
	}

	// Vote This App text
	{
		const std::string& text = "> Vote This App";
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::MiddleCenter, UnsignedInt(text.length()));
		go->mPosition = Vector3(2.0f, 2.0f, 0.0f);
		go->mColor = Color4(0.6f, 1.0f, 0.6f, 1.0f);
		go->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
		go->setSize(Vector2(1.0f));
		go->setText(text);

		mScreenButtons[GO_LS_TEXT_VOTE_ME] = {
			(std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(go, true),
			[this](UnsignedInt index) {
			// Avoid inconsistencies
			if (!mSettingsOpened)
			{
				return false;
			}

			Debug{} << "You have clicked VOTE ME";
			return true;
		}
		};
	}

	// Other Apps text
	{
		const std::string& text = "> Other Apps";
		const std::shared_ptr<OverlayText> go = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::MiddleCenter, UnsignedInt(text.length()));
		go->mPosition = Vector3(2.0f, 2.0f, 0.0f);
		go->mColor = Color4(1.0f, 0.6f, 0.6f, 1.0f);
		go->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
		go->setSize(Vector2(1.0f));
		go->setText(text);

		mScreenButtons[GO_LS_TEXT_OTHER_APPS] = {
			(std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(go, true),
			[this](UnsignedInt index) {
			// Avoid inconsistencies
			if (!mSettingsOpened)
			{
				return false;
			}

			Debug{} << "You have clicked OTHER APPS";
			return true;
		}
		};
	}
}

void LevelSelector::createGuis()
{
	// White glow quad
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_WHITE);
		o->setPosition({ 0.0f, 0.0f });
		o->setSize({ 1.0f, 1.0f });
		o->setAnchor({ 0.0f, 0.0f });
		o->setColor({ 1.0f, 1.0f, 1.0f, 0.0f });

		mLevelGuis[GO_LS_GUI_WHITEGLOW] = (std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true);
	}

	// Create "Settings" button
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_SETTINGS);
		o->setPosition({ -2.0f, 0.5f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ 0.0f, -0.0f });

		mScreenButtons[GO_LS_GUI_SETTINGS] = {
			(std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true),
			[this](UnsignedInt index) {
			// Avoid inconsistencies
			if (mLevelEndingAnim)
			{
				return false;
			}

			// Close level details screen
			if (mLevelInfo.currentViewingLevelId != 0U)
			{
				// Reset current viewing level ID
				mLevelInfo.currentViewingLevelId = 0U;

				// Play sound
				playSfxAudio(GO_LS_AUDIO_PAUSE_OUT);
			}
			// If already closed, open/close the settings window
			else if (mLevelInfo.state < GO_LS_LEVEL_FINISHED)
			{
				Int index;

				mSettingsOpened = !mSettingsOpened;
				if (mSettingsOpened)
				{
					// Game logic
					index = GO_LS_AUDIO_PAUSE_IN;
					if (!mLevelInfo.playerPointer.expired())
					{
						((std::shared_ptr<Player>&)mLevelInfo.playerPointer.lock())->mCanShoot = false;
					}

					// Set pause text
					if (mLevelInfo.state <= GO_LS_LEVEL_INIT)
					{
						mLevelTexts[GO_LS_TEXT_LEVEL]->setText("Settings");
					}
				}
				else
				{
					index = GO_LS_AUDIO_PAUSE_OUT;
				}

				// Play sound
				playSfxAudio(GO_LS_AUDIO_PAUSE_OUT);

				// Debug print
				Debug{} << "You have" << (mSettingsOpened ? "opened" : "closed") << "SETTINGS";
			}

			return true;
		}
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
			[this](UnsignedInt index) {
				Debug{} << "You have clicked REPLAY";

				if (!mDialog.expired())
				{
					return false;
				}

				const std::shared_ptr<Dialog> o = std::make_shared<Dialog>(GOL_ORTHO_FIRST);
				o->setMessage("Do you really\nwant to restart\nthis level?");
				o->addAction("Yes", [this]() {
					Debug{} << "You have clicked YES to REPLAY";
					if (mLevelInfo.state == GO_LS_LEVEL_STARTED || mLevelInfo.state == GO_LS_LEVEL_FINISHED)
					{
						closeDialog();
						replayCurrentLevel();
					}
				});
				o->addAction("No", [this]() {
					Debug{} << "You have clicked NO to REPLAY";
					closeDialog();
				});

				mDialog = (std::shared_ptr<Dialog>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true);
				return true;
			}
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
			[this](UnsignedInt index) {
				Debug{} << "You have clicked NEXT";

				// Get position to animate to  (next level)
				if (mLevelInfo.success)
				{
					// Get next index
					Int yi, yf;

					{
						const Float sli(Float(mLevelInfo.selectedLevelId - 1U));
						yi = Int(Math::floor(sli / 6.0f));
						yf = Int(Int(sli) - yi * 6) + 1;
					}

					// Wrap index to next scenery
					if (yf >= 6)
					{
						++yi;
						yf = 0;
					}

					// Iterator check before continuing
					const auto& bs = mSceneries.find(-yi);
					if (bs == mSceneries.end())
					{
						Error{} << "No Scenery found with index" << yi;
						return false;
					}

					// Check for consistency
					if (yf >= bs->second.buttons.size())
					{
						Error{} << "Button index" << yf << "is greater than size" << bs->second.buttons.size();
						return false;
					}
					const auto& bp = bs->second.buttons.at(yf).position;

					// Set parameters for later animation
					mLevelInfo.currentLevelPos = mPosition;

					const Vector3 sp = bs->second.scenery->mPosition;
					mLevelInfo.nextLevelPos = Vector3(sp.x(), 0.0f, sp.z()) + Vector3(bp.x(), 0.0f, bp.z() - 8.0f);
				}

				// Restore level state to init
				mLevelInfo.state = GO_LS_LEVEL_RESTORING;

				// Update stats
				RoomManager::singleton->mSaveData.coinCurrent = 0;
				return true;
			}
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
			[this](UnsignedInt index) {
				Debug{} << "You have clicked SHARE";
				return true;
			}
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
			[this](UnsignedInt index) {
				mLevelInfo.numberOfRetries = 0;
				startLevel(mLevelInfo.currentViewingLevelId);
				return true;
			}
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
			[this](UnsignedInt index) {
				Debug{} << "You have clicked EXIT";

				if (!mDialog.expired())
				{
					return false;
				}

				const std::shared_ptr<Dialog> o = std::make_shared<Dialog>(GOL_ORTHO_FIRST);
				o->setMessage("Do you really\nwant to exit\nthis level?");
				o->addAction("Yes", [this]() {
					Debug{} << "You have clicked YES to EXIT";

					// Set variable flag for level ending
					if (mLevelInfo.state == GO_LS_LEVEL_STARTED)
					{
						closeDialog();
						mLevelEndingAnim = true;
					}
				});
				o->addAction("No", [this]() {
					Debug{} << "You have clicked NO to EXIT";
					closeDialog();
				});

				mDialog = (std::shared_ptr<Dialog>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true);
				return true;
			}
		};
	}

	// Create "BG Music" button
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_BGMUSIC_ON);
		o->setPosition({ -2.0f, 0.5f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ 0.0f, 0.0f });

		mScreenButtons[GO_LS_GUI_BGMUSIC] = {
			(std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true),
			[this](UnsignedInt index) {
				Debug{} << "You have clicked BGMUSIC";

				const Float level = RoomManager::singleton->getBgMusicGain() >= 0.1f ? 0.0f : 0.25f;
				RoomManager::singleton->setBgMusicGain(level);

				((std::shared_ptr<OverlayGui>&)mScreenButtons[GO_LS_GUI_BGMUSIC].drawable)->setTexture(RoomManager::singleton->getBgMusicGain() > 0.01f ? RESOURCE_TEXTURE_GUI_BGMUSIC_ON : RESOURCE_TEXTURE_GUI_BGMUSIC_OFF);
				return true;
			}
		};
	}

	// Create "SFX" button
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_SFX_ON);
		o->setPosition({ -2.0f, 0.5f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ 0.0f, 0.0f });

		mScreenButtons[GO_LS_GUI_SFX] = {
			(std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true),
			[this](UnsignedInt index) {
				Debug{} << "You have clicked SFX";

				const Float level = RoomManager::singleton->getSfxGain() >= 0.99f ? 0.0f : 1.0f;
				RoomManager::singleton->setSfxGain(level);

				((std::shared_ptr<OverlayGui>&)mScreenButtons[GO_LS_GUI_SFX].drawable)->setTexture(RoomManager::singleton->getSfxGain() > 0.01f ? RESOURCE_TEXTURE_GUI_SFX_ON : RESOURCE_TEXTURE_GUI_SFX_OFF);
				return true;
			}
		};
	}

	// Three stars
	for (UnsignedInt i = 0; i < 3; ++i)
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_STAR);
		o->setPosition({ 2.0f, 2.0f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ 0.0f, 0.0f });

		mLevelGuis[GO_LS_GUI_STAR + i] = (std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true);
	}

	// Coin icon
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_COIN);
		o->setPosition({ 2.0f, 2.0f });
		o->setSize({ 0.0625f, 0.0625f });
		o->setAnchor(Vector2(1.0f, -1.0f));

		mLevelGuis[GO_LS_GUI_COIN] = (std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true);
	}

	// Timer icon
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_TIME);
		o->setPosition({ 2.0f, 2.0f });
		o->setSize({ 0.08f, 0.08f });
		o->setAnchor({ -1.0f, 1.0f });

		mLevelGuis[GO_LS_GUI_TIME] = (std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true);
	}

	// Scroll Back text
	{
		const std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST, RESOURCE_TEXTURE_GUI_SCROLL_BACK);
		o->setPosition({ 2.0f, 2.0f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ -1.0f, 1.0f });

		mScreenButtons[GO_LS_GUI_SCROLL_BACK] = {
			(std::shared_ptr<OverlayText>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true),
			[this](UnsignedInt index) {
			// Avoid inconsistencies
			if (mSettingsOpened || mLevelAnim > 0.01f || mLevelInfo.state != GO_LS_LEVEL_INIT)
			{
				return false;
			}

			Debug{} << "You have clicked SCROLL BACK";

			mLevelGuis[GO_LS_GUI_WHITEGLOW]->color()[3] = 1.0f;

			const auto oldPosition = mPosition;
			mPosition = Vector3(mLevelInfo.lastLevelPos.x(), 0.0f, mLevelInfo.lastLevelPos.z() - 8.0f);
			handleScrollableCameraPosition(mPosition - oldPosition);
			handleScrollableScenery();

			return true;
		}
		};
	}
}