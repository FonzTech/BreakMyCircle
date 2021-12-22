#include "IAppStateCallback.h"
#include "../../RoomManager.h"

IAppStateCallback::IAppStateCallback()
{
    RoomManager::singleton->mAppStateCallbacks.insert(this);
}

IAppStateCallback::~IAppStateCallback()
{
    auto& s = RoomManager::singleton->mAppStateCallbacks;
    s.erase(s.find(this));
}
