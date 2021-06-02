#include "CollisionManager.h"
#include "RoomManager.h"

CollisionManager::CollisionManager()
{
}

std::unique_ptr<std::unordered_set<GameObject*>> CollisionManager::checkCollision(const Range3D & bbox, const GameObject* go) const
{
	std::unique_ptr<std::unordered_set<GameObject*>> set = std::make_unique<std::unordered_set<GameObject*>>();
	auto& gos = RoomManager::singleton->mGameObjects;
	for (UnsignedInt i = 0; i < gos.size(); ++i)
	{
		if (go == gos[i].get())
		{
			continue;
		}
		else if (Math::intersects(bbox, gos[i]->bbox))
		{
			GameObject* p = gos[i].get();
			if (set->find(p) == set->end())
			{
				set->insert(gos[i].get());
			}
		}
	}

	return set;
}