#pragma once

#include <Magnum/Magnum.h>

#ifdef CORRADE_TARGET_ANDROID
#include <Magnum/Platform/AndroidApplication.h>
#else
#include <Magnum/Platform/Sdl2Application.h>
#endif

using namespace Magnum;

class IAppStateCallback
{
public:
    explicit IAppStateCallback();
    ~IAppStateCallback();
    
    virtual void pauseApp() = 0;
    virtual void resumeApp() = 0;
    virtual void viewportChange(Platform::Sdl2Application::ViewportEvent* event) = 0;
};
