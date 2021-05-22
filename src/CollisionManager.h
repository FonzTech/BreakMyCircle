#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "GameObject.h"

using namespace Magnum;

typedef std::unordered_map<Int, std::vector<std::weak_ptr<GameObject>>> BubbleRowsDS;

class CollisionManager
{
public:
	explicit CollisionManager();

	void addBubbleToRow(Int rowIndex, const std::weak_ptr<GameObject> & gameObject);
	std::shared_ptr<GameObject> checkBubbleCollision(const GameObject* bubble);
	Int getRowIndexByBubble(const GameObject* gameObject);

protected:
	// Bubbles rows
	BubbleRowsDS bubbleRows;
};