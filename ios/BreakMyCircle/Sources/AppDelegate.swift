import UIKit

class AppDelegate: UIResponder, UIApplicationDelegate {
  var window: UIWindow?

  func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication .LaunchOptionsKey: Any]?) -> Bool {
    // Check if user has launched the app from a Push Notification
    if let launchOptions = launchOptions {
        if let data = launchOptions[UIApplication.LaunchOptionsKey.remoteNotification] {
            #if DEBUG
            print("LaunchOptions Data:", data)
            #endif
        }
    }
    
    #if DEBUG
    print("Application has started")
    #endif
    
    return true
  }
}
