import Foundation
import UIKit

fileprivate var alertWindows = [UIAlertController:UIWindow]()

extension UIAlertController {
    fileprivate func afterWindowCreate(window: UIWindow) {
        alertWindows[self] = window
    }

    func presentInNewWindow(animated: Bool, completion: (() -> Void)?) {
        let v = Utility.createTopViewController(afterWindowCreate: afterWindowCreate)
        if v != nil {
            v!.present(self, animated: animated, completion: completion)
        }
        else {
            print("Could not create TopViewController to present an AlertController")
        }
    }

    open override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)
        alertWindows[self] = nil
    }

}
