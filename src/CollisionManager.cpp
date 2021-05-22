#include "CollisionManager.h"

CollisionManager::CollisionManager()
{
}

void CollisionManager::addBubbleToRow(Int rowIndex, const std::weak_ptr<GameObject> & gameObject)
{
	bubbleRows[rowIndex].push_back(gameObject);
}

std::shared_ptr<GameObject> CollisionManager::checkBubbleCollision(const GameObject* bubble)
{
	const Int rowIndex = getRowIndexByBubble(bubble);
	const BubbleRowsDS::const_iterator it = bubbleRows.find(rowIndex);
	if (it == bubbleRows.end())
	{
		return nullptr;
	}

	const auto& list = it->second;
	for (auto& wpb : list)
	{
		if (wpb.expired())
		{
			continue;
		}

		const auto& b2 = wpb.lock();
		if (bubble == b2.get())
		{
			continue;
		}

		if (Math::intersects(bubble->bbox, b2->bbox))
		{
			return b2;
		}
	}

	return nullptr;
}

Int CollisionManager::getRowIndexByBubble(const GameObject* gameObject)
{
	return Int(std::abs(gameObject->position.y()) * 0.5f);
}