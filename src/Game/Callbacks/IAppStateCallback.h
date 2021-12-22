#pragma once

#include <Magnum/Magnum.h>

using namespace Magnum;

class IAppStateCallback
{
public:
    explicit IAppStateCallback();
    ~IAppStateCallback();
    
    virtual void pauseApp() = 0;
    virtual void resumeApp() = 0;
};
