#include "IAppStateCallback.h"
#include "../../RoomManager.h"

IAppStateCallback::IAppStateCallback()
{
	RoomManager::singleton->mAppStateCallbacks.insert(this);
}

IAppStateCallback::~IAppStateCallback()
{
    auto& s = RoomManager::singleton->mAppStateCallbacks;
    const auto& it = s.find(this);
    if (it != s.end())
    {
        s.erase(it);
    }
    else
    {
        Debug{} << "App State callback object not present for " << this;
    }
}
