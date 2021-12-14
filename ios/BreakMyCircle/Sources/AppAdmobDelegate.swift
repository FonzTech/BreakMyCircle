import Foundation
import GoogleMobileAds

class AppAdmobDelegate : NSObject, GADFullScreenContentDelegate {
    public static let TYPE_FAILED_TO_PRESENT = 1
    public static let TYPE_PRESENTED = 2
    public static let TYPE_DISMISSED = 3
    
    var adCallback: ((_: Int, _: Error?) -> Void)? = nil
    
    func ad(_ ad: GADFullScreenPresentingAd, didFailToPresentFullScreenContentWithError error: Error) {
        if adCallback != nil {
            adCallback!(AppAdmobDelegate.TYPE_FAILED_TO_PRESENT, error)
        }
        else {
            print("Failed to present ad: \(error)")
        }
    }
    
    func adDidPresentFullScreenContent(_ ad: GADFullScreenPresentingAd) {
        if adCallback != nil {
            adCallback!(AppAdmobDelegate.TYPE_PRESENTED, nil)
        }
        else {
            print("Ad presented successfully")
        }
    }
    
    func adDidDismissFullScreenContent(_ ad: GADFullScreenPresentingAd) {
        if adCallback != nil {
            adCallback!(AppAdmobDelegate.TYPE_DISMISSED, nil)
        }
        else {
            print("Ad dismissed")
        }
    }
}
