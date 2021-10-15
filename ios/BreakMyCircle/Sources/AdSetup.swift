import Foundation
import GoogleMobileAds

var isAdmobInitialized: Bool = false;

@_cdecl("ios_SetupAds")
func ios_SetupAds() {
    GADMobileAds.sharedInstance().requestConfiguration.tag(forChildDirectedTreatment: true)
    GADMobileAds.sharedInstance().start { (status) in
        isAdmobInitialized = true
    }
}
