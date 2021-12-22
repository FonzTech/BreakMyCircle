#pragma once

class IAppStateCallback
{
public:
    explicit IAppStateCallback();
    ~IAppStateCallback();
    
    virtual void pauseApp() = 0;
    virtual void resumeApp() = 0;
};
