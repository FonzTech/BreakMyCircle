#pragma once

#include <memory>
#include <vector>
#include "GameObject.h"

class RoomManager
{
public:
	static std::shared_ptr<RoomManager> singleton;

	std::vector<std::shared_ptr<GameObject>> gameObjects;

	RoomManager();
	void clear();
};