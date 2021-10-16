import Foundation
import UIKit
import Network
import SystemConfiguration

class Utility {
    public static var DEBUG = true;
    
    public static let API_URL = "https://breakmycircle.alfonsopauciello.com/api.php";
    
    public static let ADMOB_DEV_INTERSTITIAL = "ca-app-pub-3940256099942544/8691691433";
    public static let ADMOB_PROD_INTERSTITIAL = "ca-app-pub-3837498848455030/9483010950";
    
    public static let ADMOB_DEV_REWARDED = "ca-app-pub-3940256099942544/5354046379";
    public static let ADMOB_PROD_REWARDED = "ca-app-pub-3837498848455030/2119526254";
    
    public static func isConnectedToNetwork() -> Bool {
        var zeroAddress = sockaddr_in()
        zeroAddress.sin_len = UInt8(MemoryLayout<sockaddr_in>.size)
        zeroAddress.sin_family = sa_family_t(AF_INET)

        guard let defaultRouteReachability = withUnsafePointer(to: &zeroAddress, {
            $0.withMemoryRebound(to: sockaddr.self, capacity: 1) {
                SCNetworkReachabilityCreateWithAddress(nil, $0)
            }
        }) else {
            return false
        }

        var flags: SCNetworkReachabilityFlags = []
        if !SCNetworkReachabilityGetFlags(defaultRouteReachability, &flags) {
            return false
        }

        let isReachable = flags.contains(.reachable)
        let needsConnection = flags.contains(.connectionRequired)

        return (isReachable && !needsConnection)
    }
    
    public static func createTopViewController(afterWindowCreate: ((_: UIWindow) -> Void)?) -> UIViewController? {
        if #available(iOS 13.0, *) {
            let foregroundActiveScene = UIApplication.shared.connectedScenes.filter { $0.activationState == .foregroundActive }.first
            guard let foregroundWindowScene = foregroundActiveScene as? UIWindowScene else { return nil }

            let window = UIWindow(windowScene: foregroundWindowScene)
            if afterWindowCreate != nil {
                afterWindowCreate!(window)
            }

            window.rootViewController = UIViewController()
            window.windowLevel = .alert + 1
            window.makeKeyAndVisible()
            return window.rootViewController
        }
        return nil
    }
}
