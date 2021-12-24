import Foundation
import UIKit

fileprivate var alertWindows = [UIAlertController:UIWindow]()

extension UIAlertController {
    public func presentInNewWindow(animated: Bool, completion: (() -> Void)?) {
        let v = Utility.createTopViewController(afterWindowCreate: afterWindowCreate)
        if v != nil {
            v!.present(self, animated: animated, completion: completion)
        }
        else {
            print("Could not create TopViewController to present an AlertController")
        }
    }
    
    public func dismissCallback() {
        alertWindows[self]?.rootViewController?.dismiss(animated: true, completion: nil)
        alertWindows[self] = nil
    }
    
    fileprivate func afterWindowCreate(window: UIWindow) {
        alertWindows[self] = window
    }
}
