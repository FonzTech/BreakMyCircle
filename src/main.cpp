#ifdef CORRADE_TARGET_ANDROID
#include <Magnum/Platform/AndroidApplication.h>
#else
#include <Magnum/Platform/Sdl2Application.h>
#endif

#include "Engine.h"

using namespace Magnum;

MAGNUM_APPLICATION_MAIN(Engine)