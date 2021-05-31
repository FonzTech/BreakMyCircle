#pragma once

#include <memory>
#include <unordered_set>

#include "GameObject.h"

using namespace Magnum;

class CollisionManager
{
public:
	explicit CollisionManager();

	std::unique_ptr<std::unordered_set<GameObject*>> checkCollision(const GameObject* go) const;
};