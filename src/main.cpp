#include "Engine.h"

#if defined(CORRADE_TARGET_IOS_SIMULATOR) or defined(CORRADE_TARGET_IOS)
#include <MagnumPlugins/AnyImageImporter/importStaticPlugin.cpp>
#include <MagnumPlugins/AnySceneImporter/importStaticPlugin.cpp>
#include <MagnumPlugins/PngImporter/importStaticPlugin.cpp>
#include <MagnumPlugins/StbTrueTypeFont/importStaticPlugin.cpp>
#include <MagnumPlugins/StbVorbisAudioImporter/importStaticPlugin.cpp>
#include <MagnumPlugins/TinyGltfImporter/importStaticPlugin.cpp>
#endif

using namespace Magnum;

MAGNUM_APPLICATION_MAIN(Engine)
