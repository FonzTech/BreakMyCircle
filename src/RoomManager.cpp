#include "RoomManager.h"

std::shared_ptr<RoomManager> RoomManager::singleton = nullptr;

RoomManager::RoomManager()
{
}

void RoomManager::clear()
{
	gameObjects.clear();
}