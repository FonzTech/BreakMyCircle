#include "IAppStateCallback.h"
#include "../../RoomManager.h"

IAppStateCallback::IAppStateCallback()
{
    RoomManager::singleton->mAppStateCallbacks.insert(this);
}

IAppStateCallback::~IAppStateCallback()
{
    RoomManager::singleton->mAppStateCallbacks.erase(this);
}
