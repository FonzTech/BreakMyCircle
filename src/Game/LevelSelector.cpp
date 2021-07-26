#include "LevelSelector.h"
#include "../RoomManager.h"
#include "../InputManager.h"

std::shared_ptr<GameObject> LevelSelector::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate player object
	std::shared_ptr<LevelSelector> p = std::make_shared<LevelSelector>(parent);
	return p;
}

LevelSelector::LevelSelector(const Int parentIndex) : GameObject()
{
	// Assign parent index
	mParentIndex = parentIndex;

	// Init members
	mPosition = Vector3(0.0f);
	mScrollVelocity = Vector3(0.0f);
	mClickIndex = -1;

	// Create overlays
	{
		std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST);
		o->setPosition({ -0.5f, 0.5f });
		o->setSize({ 0.1f, 0.1f });
		o->setAnchor({ 1.0f, -1.0f });

		mButtons[0] = (std::shared_ptr<OverlayGui>&) RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o, true);

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

	// Handle button clicks
	const auto& lbs = InputManager::singleton->mMouseStates[ImMouseButtons::Left];

	const Vector3 p(Float(InputManager::singleton->mMousePosition.x()), Float(InputManager::singleton->mMousePosition.y()), 0.0f);
	const Vector2 w(Float(RoomManager::singleton->mWindowSize.x()), Float(RoomManager::singleton->mWindowSize.y()));

	for (Int i = 0; i < 1; ++i)
	{
		const auto& b = mButtons[i]->getBoundingBox(w);
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
	// Reset scrolling behaviour
	else
	{
		mClickIndex = -1;

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
	const Int modelIndexes[] = {
		Int(Math::floor((mPosition.y() - 50.0f) / 50.0f)),
		Int(Math::floor(mPosition.y() / 50.0f)),
		Int(Math::floor((mPosition.y() + 50.0f) / 50.0f))
	};

	for (Int i = 0; i < 3; ++i)
	{
		// const Int modelIndex = (modelIndexes[i] % 1) + 1;
		const Int modelIndex = 0;

		const auto& it = mSceneries.find(modelIndexes[i]);
		if (it != mSceneries.end())
		{
			if (it->second->getModelIndex() == modelIndex)
			{
				continue;
			}
		}

		std::shared_ptr<Scenery> go = std::make_unique<Scenery>(GOL_PERSP_FIRST, modelIndex);
		mSceneries[modelIndexes[i]] = (std::shared_ptr<Scenery>&) RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST].push_back(go, true);
	}
}