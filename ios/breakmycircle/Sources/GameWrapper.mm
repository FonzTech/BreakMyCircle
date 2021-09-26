#import "GameWrapper.h"
#import "Engine.h"

@implementation GameWrapper

- (void) startApp {
    int argc = 0;
    char* argv[0];
    Engine app(Engine::Arguments{argc, argv});
    app.exec();
}

@end
