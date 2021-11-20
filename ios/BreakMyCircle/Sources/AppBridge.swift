import Foundation
import GoogleMobileAds
import FirebaseCore
import FirebaseMessaging
import SwiftHTTP

var appInfoTimer: Timer? = nil
var appStoreUrl: String = ""
var appDeveloperUrl: String = ""

var isAdmobInitialized: Bool = false
var playAdThreshold: Int = 3
var canShowAds: Bool = true

var gpExpire: Int64 = 1
var gpAmount: Int = 0

var rewardedAd: GADRewardedInterstitialAd? = nil
var interstitialAd: GADInterstitialAd? = nil
let admobDelegate = AppAdmobDelegate()
let appNotificationHandler = AppNotificationHandler()

@_cdecl("ios_SetupApp")
public func ios_SetupApp() {
    
    // Setup ads
    setupAds()
    setupFirebase()
    admobDelegate.rewardedAdCallback = adCallback
    
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
        let unitId = Utility.DEBUG ? Utility.ADMOB_DEV_REWARDED : Utility.ADMOB_PROD_REWARDED
        GADRewardedInterstitialAd.load(
            withAdUnitID: unitId, request: GADRequest()
          ) { (ad, error) in
            if let error = error {
                print("Rewarded ad failed to load with error: \(error.localizedDescription)")
                showAdError()
                return
            }
            print("Loading Succeeded")
            
            // Set delegate
            rewardedAd = ad
            rewardedAd?.fullScreenContentDelegate = admobDelegate
            
            // Try to display ad
            let root = UIApplication.shared.topViewController()
            if root != nil {
                ad?.present(fromRootViewController: root!) {
                    let reward = ad!.adReward
                    print("Reward received with type \(reward.type), amount \(reward.amount.intValue)")
                    
                    if Utility.DEBUG {
                        gpAmount = 1
                    }
                    else if reward.type.lowercased() == "powerup" {
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
    if canShowAds {
        let unitId = Utility.DEBUG ? Utility.ADMOB_DEV_INTERSTITIAL : Utility.ADMOB_PROD_INTERSTITIAL
        GADInterstitialAd.load(
            withAdUnitID: unitId, request: GADRequest()
        ) { (ad, error) in
            if let error = error {
                print("Failed to load interstitial ad with error: \(error.localizedDescription)")
                return
            }
            print("Loading Succeeded")
         
            // Set delegate
            interstitialAd = ad
            interstitialAd?.fullScreenContentDelegate = admobDelegate
         
             // Try to display ad
             let root = UIApplication.shared.topViewController()
             if root != nil {
                ad?.present(fromRootViewController: root!)
             }
             else {
                 showAdError()
             }
        }
    }
    else {
        print("Could not load interstitial ad")
    }
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

fileprivate func setupFirebase() {
    // Setup firebase
    FirebaseApp.configure()
    
    // Setup delegates
    UNUserNotificationCenter.current().delegate = appNotificationHandler
    Messaging.messaging().delegate = appNotificationHandler
    
    // Request authorization
    let userNotificationCenter = UNUserNotificationCenter.current()
    userNotificationCenter.getNotificationSettings { (notificationSettings) in
        switch notificationSettings.authorizationStatus {
        case .authorized,
             .ephemeral,
             .provisional:
            notificationSetup()
            
        case .denied:
            print("The app isn't authorized to schedule or receive notifications.")
            
        case .notDetermined:
            print("The user hasn't yet made a choice about whether the app is allowed to schedule notifications.")
            
        default:
            print("Unknown case for notification status: ", notificationSettings.authorizationStatus.rawValue)
        }
    }
}

fileprivate func notificationAuthorizeRequest() {
    let authOptions: UNAuthorizationOptions = [.alert, .badge, .sound]
    UNUserNotificationCenter.current().requestAuthorization(
        options: authOptions,
        completionHandler: {
            granted, error in
            #if DEBUG
            print("Notification - Granted: \(granted)")
            print("Notification - Error:", error ?? "<No error>")
            #endif
        }
    )
}
    
fileprivate func notificationSetup() {
    UIApplication.shared.registerForRemoteNotifications()
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
        if #available(iOS 13.0, *) {
            let alert = UIAlertController(title: Bundle.main.displayName, message: "Could not open the requested URL. Please, try again later.", preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "Got It", style: .cancel, handler: nil))
            alert.presentInNewWindow(animated: true, completion: nil)
        }
    }
    else if let url = URL(string: url) {
        UIApplication.shared.open(url)
    }
}

fileprivate func adCallback(type: Int, error: Error?) {
    print("Rewarded Interstitial Ad dismissed")
    
    switch (type)
    {
    case AppAdmobDelegate.TYPE_FAILED_TO_PRESENT:
        print("Rewarded Ad failed to present")
        
    case AppAdmobDelegate.TYPE_PRESENTED:
        print("Rewarded Ad presented")
        
        
    case AppAdmobDelegate.TYPE_DISMISSED:
        print("Rewarded Ad dismissed")
        
    default:
        print("No action to take for type \(type)")
    }
    
    gpExpire = Int64(Date().timeIntervalSince1970 * 1000)
    interstitialAd = nil
}

fileprivate func showAdError() {
    if #available(iOS 13.0, *) {
        let alert = UIAlertController(title: Bundle.main.displayName, message: "This functionality is not available at this time. Please, try again later.", preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "Got It", style: .cancel, handler: nil))
        alert.presentInNewWindow(animated: true, completion: nil)
    }
}
