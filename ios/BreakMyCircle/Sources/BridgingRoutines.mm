#import <Foundation/Foundation.h>
#import "BridgingRoutines.h"

#include <Magnum/Platform/Sdl2Application.h>
#include "Engine.h"

#if defined(CORRADE_TARGET_IOS_SIMULATOR) or defined(CORRADE_TARGET_IOS)
#include <MagnumPlugins/AnyImageImporter/importStaticPlugin.cpp>
#include <MagnumPlugins/AnySceneImporter/importStaticPlugin.cpp>
#include <MagnumPlugins/PngImporter/importStaticPlugin.cpp>
#include <MagnumPlugins/StbTrueTypeFont/importStaticPlugin.cpp>
#include <MagnumPlugins/StbVorbisAudioImporter/importStaticPlugin.cpp>
#include <MagnumPlugins/TinyGltfImporter/importStaticPlugin.cpp>
#endif

@implementation BridgingRoutines

- (void) engineEntrypoint : (NSString*)executablePath; {
    SDL_SetMainReady();
    
    int argc = 1;
    char* argv[1] = { strdup(executablePath.UTF8String) };
    
    const auto& arguments = Magnum::Platform::Sdl2Application::Arguments(argc, &argv[0]);
    Engine app(arguments);
    app.exec();
}

@end
