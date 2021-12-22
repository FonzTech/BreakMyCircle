#include "IAppStateCallback.h"
#include "../../RoomManager.h"

UnsignedInt IAppStateCallback::staticAppStateCounter = 0;

IAppStateCallback::IAppStateCallback()
{
    mAppStateCallbackId = staticAppStateCounter;
    ++staticAppStateCounter;
    
    RoomManager::singleton->mAppStateCallbacks[mAppStateCallbackId] = this;
}

IAppStateCallback::~IAppStateCallback()
{
    auto& s = RoomManager::singleton->mAppStateCallbacks;
    const auto& it = s.find(mAppStateCallbackId);
    if (it != s.end())
    {
        s.erase(it);
    }
    else
    {
        Debug{} << "App State callback object not present for: " << mAppStateCallbackId;
    }
}
