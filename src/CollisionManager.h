#pragma once

#include <memory>
#include <unordered_set>
#include <functional>
#include <Corrade/Containers/Optional.h>

#include "GameObject.h"

using namespace Magnum;

class CollisionManager
{
public:
	explicit CollisionManager();

	std::unique_ptr<std::unordered_set<GameObject*>> checkCollision(const Range3D & bbox, const GameObject* go, const std::unordered_set<Int> & types, const Containers::Optional<std::function<bool(GameObject* collided)>> & verifier = Containers::NullOpt) const;
};