#include "LevelSelector.h"

#include <utility>
#include <Magnum/Math/Constants.h>

#include "../RoomManager.h"
#include "../InputManager.h"
#include "../AssetManager.h"
#include "../Common/CommonUtility.h"
#include "../Graphics/TexturedDrawable.h"

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

	// Create overlays
	{
		std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST);
		o->setPosition({ -2.0f, 0.5f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ 1.0f, -1.0f });

		mButtonAnim[0] = Constants::piHalf();
		mScreenButtons[0] = (std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true);

		mCallbacks[0] = []() {
			printf("You have clicked settings\n");
		};
	}

	// Load required scenes
	{
		AssetManager am(RESOURCE_SHADER_COLORED_PHONG, RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE, 1);
		am.loadAssets(*this, *mManipulator, "scenes/level_button.glb", this);

		for (UnsignedInt i = 0; i < mDrawables.size(); ++i)
		{
			mButtonDrawables.insert(mDrawables[i].get());
		}
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
	mButtonDrawables.clear();
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
		mButtonAnim[0] -= mDeltaTime;
		if (mButtonAnim[0] < 0.0f)
		{
			mButtonAnim[0] = 0.0f;
		}

		mScreenButtons[i]->setPosition(Vector2(-0.5f, 0.5f) - Vector2(Math::sin(Rad(mButtonAnim[0])), 0.0f));
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

	// Handle scrollable scenery
	if (lbs == IM_STATE_PRESSED)
	{
		mPrevMousePos = InputManager::singleton->mMousePosition;
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
	if (mButtonDrawables.find(baseDrawable) != mButtonDrawables.end())
	{
		return;
	}
	
	((Shaders::Phong&)baseDrawable->getShader())
		.setLightPosition(camera.cameraMatrix().transformPoint(mPosition + Vector3(0.0f, 6.0f, 0.0f)))
		.setLightColor(0xffffffff_rgbaf)
		.setSpecularColor(0xffffff00_rgbaf)
		.setAmbientColor(0x444444ff_rgbaf)
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
		.setObjectId(100)
		.draw(*baseDrawable->mMesh);
}

void LevelSelector::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
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
			mSceneries.erase(it->first);
		}
	}

	// Create visible sceneries
	for (const Int yp : yps)
	{
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

		// Create scenery
		std::shared_ptr<Scenery> go = std::make_unique<Scenery>(GOL_PERSP_FIRST, modelIndex);
		mSceneries[yp].scenery = (std::shared_ptr<Scenery>&) RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST].push_back(go, true);

		// Create level selectors
#if NDEBUG or _DEBUG
		if (mSceneries[yp].buttons.size())
		{
			Fatal{} << "Buttons vector for position" << yp << "should be empty, but it was not.";
		}
#endif

		// Create four drawables
		for (Int i = 0; i < 6; ++i)
		{
			mSceneries[yp].buttons.push_back(LS_ButtonSelector());
			auto& bs = mSceneries[yp].buttons.back();

			for (const auto& bd : mButtonDrawables)
			{
				// Replicate single drawable
				std::shared_ptr<TexturedDrawable<Shaders::Phong>> td = std::make_shared<TexturedDrawable<Shaders::Phong>>((TexturedDrawable<Shaders::Phong>*)bd);
				td->setParent(bd->parent());
				td->setDrawCallback(this);

				// Apply the same transformations
				bs.drawables = td;
				bs.position = sLevelButtonPositions[modelIndex][i];
				bs.index = i;

				td->setTransformation(Matrix4::translation(bs.position));

				// Make parent object the owner for these drawables
				mDrawables.emplace_back(td);
			}
		}
	}
}