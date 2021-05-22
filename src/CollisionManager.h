#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "GameObject.h"

using namespace Magnum;

class CollisionManager
{
public:
	explicit CollisionManager();

	std::shared_ptr<GameObject> checkCollision(const GameObject* go);
};