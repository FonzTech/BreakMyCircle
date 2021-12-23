#include "IAppStateCallback.h"
#include "../../RoomManager.h"

IAppStateCallback::IAppStateCallback()
{
	RoomManager::singleton->mAppStateCallbacks.insert(this);
}

IAppStateCallback::~IAppStateCallback()
{
#if defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_IOS_SIMULATOR)
    /*
        applicationWillTerminate is not called, on iOS, when
        the user kills the app through the task manager.
     */
    if (RoomManager::singleton == nullptr)
    {
        return;
    }
#endif
    
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
