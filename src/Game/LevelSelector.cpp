#include "LevelSelector.h"
#include "OverlayGui.h"
#include "../RoomManager.h"

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
	mScroll = 0.0f;

	// Create overlays
	std::shared_ptr<OverlayGui> o = std::make_shared<OverlayGui>(GOL_ORTHO_FIRST);
	o->setPosition({ -0.5f, 0.5f });
	o->setSize({ 0.1f, 0.1f });
	o->setAnchor({ 1.0f, -1.0f });
	RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(o);
}


const Int LevelSelector::getType() const
{
	return GOT_LEVEL_SELECTOR;
}

void LevelSelector::update()
{
}

void LevelSelector::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
}

void LevelSelector::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}