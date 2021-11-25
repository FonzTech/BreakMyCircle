#import <Foundation/Foundation.h>
#import <SDL_uikitappdelegate.h>
#import "BridgingRoutines.h"

@implementation BridgingRoutines

- (NSDictionary*)getLaunchOption {
    SDLUIKitDelegate *delegate = (SDLUIKitDelegate *)[[UIApplication sharedApplication] delegate];
    return delegate._launchOption;
}

@end
