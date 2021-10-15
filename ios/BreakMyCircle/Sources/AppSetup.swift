import Foundation
import GoogleMobileAds
import SwiftHTTP

var appInfoTimer: Timer? = nil
var appStoreUrl: String = ""
var appDeveloperUrl: String = ""

var isAdmobInitialized: Bool = false
var playAdThreshold: Int = 3
var canShowAds: Bool = true

var gpExpire: Int64 = 1
var gpAmount: Int = 0

let adRewardedDelegate = AppAdDelegate()

@_cdecl("ios_SetupApp")
public func ios_SetupApp() {
    
    // Setup ads
    setupAds()
    adRewardedDelegate.dismissFullScreen = dismissRewardedAd
    
    // Load stored config
    appStoreUrl = UserDefaults.standard.string(forKey: "appStoreUrl") ?? ""
    appDeveloperUrl = UserDefaults.standard.string(forKey: "appDeveloperUrl") ?? ""
    
    // Get app info
    if Utility.isConnectedToNetwork() {
        getAppInfo()
    }
    else {
        appInfoTimer = Timer.scheduledTimer(withTimeInterval: 5.0, repeats: true) { timer in
            if Utility.isConnectedToNetwork() {
                appInfoTimer?.invalidate()
                getAppInfo()
            }
        }
    }
    
    Timer.scheduledTimer(withTimeInterval: 5.0, repeats: false) { timer in
        ios_WatchAdPowerup()
    }
}

@_cdecl("ios_GetPlayAdThreshold")
public func ios_GetPlayAdThreshold() -> Int {
    return playAdThreshold
}

@_cdecl("ios_GetGamePowerupExpire")
public func ios_GetGamePowerupExpire() -> Int64 {
    return gpExpire
}

@_cdecl("ios_GetGamePowerupAmount")
public func ios_GetGamePowerupAmount() -> Int {
    return gpAmount
}

@_cdecl("ios_ClearPowerupData")
public func ios_ClearPowerupData() {
    gpExpire = 0
    gpAmount = 0
}

@_cdecl("ios_WatchAdPowerup")
public func ios_WatchAdPowerup() {
    gpAmount = 0
    
    if canShowAds {
        let unitId = Utility.DEBUG ? Utility.ADMOB_DEV_REWARDED : Utility.ADMOB_PROD_REWARDED;
        GADRewardedInterstitialAd.load(
            withAdUnitID: unitId, request: GADRequest()
          ) { (ad, error) in
            if let error = error {
                print("Rewarded ad failed to load with error: \(error.localizedDescription)")
              return
            }
            print("Loading Succeeded")
            
            // Set delegate
            ad?.fullScreenContentDelegate = adRewardedDelegate
            
            // Try to display ad
            let root = UIApplication.shared.windows.first?.rootViewController?.presentedViewController
            if root != nil {
                ad?.present(fromRootViewController: root!) {
                    let reward = ad!.adReward
                    print("Reward received with type \(reward.type), amount \(reward.amount.intValue)")
                    
                    if (reward.type.lowercased() == "powerup") {
                        gpAmount = reward.amount.intValue
                    }
                }
            }
            else {
                gpExpire = 1
                showAdError()
            }
          }
    }
    else {
        gpExpire = 1
        showAdError()
    }
}

@_cdecl("ios_ShowInterstitial")
public func ios_ShowInterstitial() {
}

@_cdecl("ios_GameVoteMe")
public func ios_GameVoteMe() {
    openUrl(url: appStoreUrl)
}

@_cdecl("ios_GameOtherApps")
public func ios_GameOtherApps() {
    openUrl(url: appDeveloperUrl)
}

fileprivate func setupAds() {
    GADMobileAds.sharedInstance().requestConfiguration.tag(forChildDirectedTreatment: true)
    GADMobileAds.sharedInstance().start { (status) in
        isAdmobInitialized = true
    }
}

fileprivate func getAppInfo() {
    let body = [
        "type": "ios",
        "version": Bundle.main.appBuild,
        "locale": Locale.current.regionCode
    ]
    
    HTTP.POST(Utility.API_URL, parameters: body) { response in
        if let err = response.error {
            print("Error: \(err.localizedDescription)")
            return
        }
        
        do {
            let rawDict = try JSONSerialization.jsonObject(with: response.data, options: [.allowFragments]) as? [String:Any]
            if let jsonData = rawDict {
                // Mandatory data
                playAdThreshold = jsonData["playAdThreshold"] as! Int
                canShowAds = jsonData["canShowAds"] as! Bool
                
                // Store data
                let storeUrl = jsonData["storeUrl"]
                if storeUrl != nil {
                    appStoreUrl = storeUrl as! String
                    UserDefaults.standard.set(appStoreUrl, forKey: "appStoreUrl")
                }
                
                // Developer data
                let developerUrl = jsonData["developerUrl"]
                if developerUrl != nil {
                    appDeveloperUrl = developerUrl as! String
                    UserDefaults.standard.set(appStoreUrl, forKey: "appDeveloperUrl")
                }
            }
            else {
                print("Error while getting response from API")
            }
        } catch let error {
            print("Error while deserializing JSON: \(error)")
         }
    }
}

fileprivate func openUrl(url: String) {
    if url.count <= 0 {
        let alert = UIAlertController(title: Bundle.main.displayName, message: "Could not open the requested URL. Please, try again later.", preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "Got It", style: .cancel, handler: nil))
        alert.presentInNewWindow(animated: true, completion: nil)
    }
    else if let url = URL(string: url) {
        UIApplication.shared.open(url)
    }
}

fileprivate func dismissRewardedAd() {
    gpExpire = Int64(Date().timeIntervalSince1970 * 1000)
}

fileprivate func showAdError() {
    let alert = UIAlertController(title: Bundle.main.displayName, message: "This functionality is not available at this time. Please, try again later.", preferredStyle: .alert)
    alert.addAction(UIAlertAction(title: "Got It", style: .cancel, handler: nil))
    alert.presentInNewWindow(animated: true, completion: nil)
}
