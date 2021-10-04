#include "CollisionManager.h"
#include "RoomManager.h"

CollisionManager::CollisionManager()
{
}

std::unique_ptr<std::unordered_set<GameObject*>> CollisionManager::checkCollision(const Range3D & bbox, const GameObject* go, const std::unordered_set<Int> & types, const Containers::Optional<std::function<bool(GameObject* collided)>> & verifier) const
{
	std::unique_ptr<std::unordered_set<GameObject*>> set = std::make_unique<std::unordered_set<GameObject*>>();

	auto& gos = RoomManager::singleton->mGoLayers[go->mParentIndex].list;

	for (auto& gi : *gos)
	{
		if (gi->mDestroyMe || go == gi.get() || types.find(gi->getType()) == types.end())
		{
			continue;
		}
		else if (Math::intersects(bbox, gi->mBbox))
		{
			if (verifier == Containers::NullOpt || (*verifier)(gi.get()))
			{
				GameObject* p = gi.get();
				if (set->find(p) == set->end())
				{
					set->insert(p);
				}
			}
		}
	}

	return set;
}
