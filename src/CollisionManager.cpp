#include "CollisionManager.h"
#include "RoomManager.h"

CollisionManager::CollisionManager()
{
}

std::shared_ptr<GameObject> CollisionManager::checkCollision(const GameObject* go)
{
	auto& gos = RoomManager::singleton->mGameObjects;
	for (UnsignedInt i = 0; i < gos.size(); ++i)
	{
		if (go == gos[i].get())
		{
			continue;
		}
		else if (Math::intersects(go->bbox, gos[i]->bbox))
		{
			return gos[i];
		}
	}

	return nullptr;
}