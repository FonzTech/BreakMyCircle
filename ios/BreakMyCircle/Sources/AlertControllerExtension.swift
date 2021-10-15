import Foundation
import UIKit

fileprivate var alertWindows = [UIAlertController:UIWindow]()

extension UIAlertController {
    fileprivate func afterWindowCreate(window: UIWindow) {
        alertWindows[self] = window
    }

    func presentInNewWindow(animated: Bool, completion: (() -> Void)?) {
        Utility.createTopViewController(afterWindowCreate: afterWindowCreate)!.present(self, animated: animated, completion: completion)
    }

    open override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)
        alertWindows[self] = nil
    }

}
