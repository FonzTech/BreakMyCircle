import Foundation
import GoogleMobileAds

class AppAdDelegate : NSObject, GADFullScreenContentDelegate {
    var presentFullScreen: (() -> Void)? = nil
    var dismissFullScreen: (() -> Void)? = nil
    
    func adDidPresentFullScreenContent(_ ad: GADFullScreenPresentingAd) {
        if presentFullScreen != nil {
            presentFullScreen!()
        }
        else {
            print("Ad presented")
        }
    }
    
    func adDidDismissFullScreenContent(_ ad: GADFullScreenPresentingAd) {
        if dismissFullScreen != nil {
            dismissFullScreen!()
        }
        else {
            print("Ad dismissed")
        }
    }
}
