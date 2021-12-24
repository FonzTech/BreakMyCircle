import Foundation
import GoogleMobileAds
import FirebaseCore
import FirebaseMessaging
import SwiftHTTP
import AppTrackingTransparency
import AdSupport

var appInfoTimer: Timer? = nil
var appStoreUrl: String = ""
var appDeveloperUrl: String = ""
var appNotifSettings: String = ""
var requestIdfaOnResume = false

var isAdmobInitialized: Bool = false
var playAdThreshold: Int = 1
var canShowAds: Bool = true

var gpSafeMinigame: Int = 0
var gpExpire: Int64 = 1
var gpAmount: Int = 0

var rewardedAd: GADRewardedInterstitialAd? = nil
var interstitialAd: GADInterstitialAd? = nil
let admobDelegate = AppAdmobDelegate()
let appNotificationHandler = AppNotificationHandler()

let NOTIFICATION_CHECK_INTERVAL = Int64(Utility.DEBUG ? 15 : 86400)
let PREFS_LAST_NOTIFICATION_CHECK = "lastNotificationCheck"
let PREFS_CONFIG_VERSION = "appConfigVersion"
let DENIED_NOTIFICATIONS_MESSAGE = "You denied to receive notifications. You will not be able to obtain powerups through push notifications. To fix this, go to Settings and enable notifications."

@_cdecl("ios_SetupApp")
public func ios_SetupApp() {
    // Process launch option
    let launchOption: [AnyHashable: Any]? = BridgingRoutines().getLaunchOption()
    processLaunchOption(launchOption)
    
    // Setup ads
    setupAds()
    setupFirebase()
    admobDelegate.adCallback = adCallback
    
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

@_cdecl("ios_WillEnterBackground")
public func ios_WillEnterBackground() {
}

@_cdecl("ios_DidEnterForeground")
public func ios_DidEnterForeground() {
    if requestIdfaOnResume {
        requestIdfaOnResume = false
        requestIdentifierForAdvertising()
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

@_cdecl("ios_GetLaunchOptionValue")
public func ios_GetLaunchOptionValue(key: UnsafePointer<CChar>?) -> Int {
    if key == nil {
        return 0
    }
    
    let str = String(cString: key!)
    if str == "game_safeminigame" {
        return gpSafeMinigame
    }
    return 0
}

@_cdecl("ios_WatchAdPowerup")
public func ios_WatchAdPowerup() {
    // Reset state
    gpAmount = 0
    gpExpire = 0
    
    // Check if ads are enabled
    if canShowAds {
        let unitId = Utility.DEBUG ? Utility.ADMOB_DEV_REWARDED : Utility.ADMOB_PROD_REWARDED
        GADRewardedInterstitialAd.load(
            withAdUnitID: unitId, request: GADRequest()
          ) { (ad, error) in
            if let error = error {
                print("Rewarded ad failed to load with error: \(error.localizedDescription)")
                cancelAdFlow()
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
                print("No parent window for rewarded ad")
                cancelAdFlow()
                showAdError()
            }
          }
    }
    else {
        print("No rewarded ad can be shown")
        cancelAdFlow()
        showAdError()
    }
}

@_cdecl("ios_ShowInterstitial")
public func ios_ShowInterstitial() {
    // Reset state
    gpExpire = 0
    
    // Check if ads are enabled
    if canShowAds {
        let unitId = Utility.DEBUG ? Utility.ADMOB_DEV_INTERSTITIAL : Utility.ADMOB_PROD_INTERSTITIAL
        GADInterstitialAd.load(
            withAdUnitID: unitId, request: GADRequest()
        ) { (ad, error) in
            if let error = error {
                print("Failed to load interstitial ad with error: \(error.localizedDescription)")
                cancelAdFlow()
                showAdError()
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
                print("No parent window for interstitial ad")
                cancelAdFlow()
                showAdError()
             }
        }
    }
    else {
        print("Could not load interstitial ad")
        cancelAdFlow()
        showAdError()
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

/*
 Get IDFA string (without permission)
 */
fileprivate func getIdfaString(isIos14: Bool) -> String? {
    if #available(iOS 14, *) {
        if ATTrackingManager.trackingAuthorizationStatus != .authorized {
            print("Advertising Tracking is NOT authorized")
            return nil
        }
    }
    else if ASIdentifierManager.shared().isAdvertisingTrackingEnabled == false {
        print("Advertising Tracking is Disabled")
        return nil
    }
    return ASIdentifierManager.shared().advertisingIdentifier.uuidString
}

/*
 Get IDFA string for advertising. Permission is requested, if necessary.
 */
fileprivate func requestIdentifierForAdvertising() {
    DispatchQueue.main.asyncAfter(deadline: .now() + 1.0, execute: {
        if #available(iOS 14, *) {
            ATTrackingManager.requestTrackingAuthorization { status in
                switch status {
                case .authorized:
                    print("ATT Authorized")
                    print("IDFA is:", getIdfaString(isIos14: true) ?? "<nil>")
                    
                case .denied:
                    print("ATT Denied")
                    
                case .notDetermined:
                    print("ATT Not Determined")
                    
                case .restricted:
                    print("ATT Restricted")
                    print("IDFA is:", getIdfaString(isIos14: true) ?? "<nil>")
                    
                @unknown default:
                    print("ATT Unknown")
                }
            }
        }
        else {
            print("IDFA is:", getIdfaString(isIos14: false) ?? "<nil>")
        }
    })
}

/*
 Setup Google AdMob
 */
fileprivate func setupAds() {
    GADMobileAds.sharedInstance().requestConfiguration.tag(forChildDirectedTreatment: true)
    GADMobileAds.sharedInstance().start { (status) in
        isAdmobInitialized = true
    }
}

/*
 Process launch options, such as data provided by notification tap.
 */
fileprivate func processLaunchOption(_ launchOption: [AnyHashable: Any]?) {
    // Check if launch options are provided
    if launchOption == nil {
        return
    }
    
    // Check for payload type
    if let payload = launchOption![UIApplication.LaunchOptionsKey.remoteNotification] as? [NSObject : AnyObject] {
        // Check for Safe Minigame
        if let safeMinigame = payload[NSString(string: "game_safeminigame")] {
            // Check for provided value
            if let value = (safeMinigame as? NSString)?.intValue {
                gpSafeMinigame = Int(value)
            }
        }
    }
}

/*
 Firebase setup and notification permission request (it's done here because its dependant
 on Firebase, since notification are implemented using that framework).
 */
fileprivate func setupFirebase() {
    // Get notification intent URL
    appNotifSettings = UserDefaults.standard.string(forKey: "appNotifSettings") ?? ""
    
    // Setup firebase
    FirebaseApp.configure()
    
    // Setup delegates
    UNUserNotificationCenter.current().delegate = appNotificationHandler
    Messaging.messaging().delegate = appNotificationHandler
    
    // Request authorization
    let userNotificationCenter = UNUserNotificationCenter.current()
    userNotificationCenter.getNotificationSettings {
        notificationSettings in
        
        var message: String? = nil
        var reqTracking = true
        
        switch notificationSettings.authorizationStatus {
        case .authorized,
             .ephemeral,
             .provisional:
            
            #if DEBUG
            print("Notifications are going to be setup:", notificationSettings.authorizationStatus.rawValue)
            #endif
            
            DispatchQueue.main.async {
                notificationSetup()
            }
            
        case .denied:
            #if DEBUG
            print("User has denied notifications")
            #endif
            message = DENIED_NOTIFICATIONS_MESSAGE
            
        case .notDetermined:
            #if DEBUG
            print("User has NOT made a choice regarding notifications yet")
            #endif
            
            reqTracking = false
            notificationAuthorizeRequest()
            
        default:
            #if DEBUG
            print("Unknown case for notification status:", notificationSettings.authorizationStatus.rawValue)
            #endif
        }
        
        if #available(iOS 13.0, *) {
            if message != nil && appNotifSettings.count > 0 {
                
                // Check if interval has passed since last dialog alert for denied notifications
                let curTime = Int64(Date().timeIntervalSince1970)
                let lastCheck = Int64(UserDefaults.standard.integer(forKey: PREFS_LAST_NOTIFICATION_CHECK))
                if lastCheck == 0 || curTime > lastCheck + NOTIFICATION_CHECK_INTERVAL {
                    // Save last check to prefs
                    UserDefaults.standard.set(curTime, forKey: PREFS_LAST_NOTIFICATION_CHECK)
                    
                    // Show alert dialog
                    reqTracking = false
                    DispatchQueue.main.asyncAfter(deadline: .now() + 6) {
                        let alert = UIAlertController(title: Bundle.main.displayName, message: message, preferredStyle: .alert)
                        
                        if appNotifSettings.count > 0 {
                            alert.addAction(UIAlertAction(title: "Go To Settings", style: .default) {
                                _ in
                                alert.dismissCallback()
                                openUrl(url: appNotifSettings)
                                requestIdfaOnResume = true
                            })
                        }
                        
                        alert.addAction(UIAlertAction(title: "I Don't Care", style: .cancel) {
                            _ in
                            alert.dismissCallback()
                            requestIdentifierForAdvertising()
                        })
                        
                        alert.presentInNewWindow(animated: true, completion: nil)
                    }
                }
            }
        }
        
        if reqTracking {
            requestIdentifierForAdvertising()
        }
    }
}

/*
 Request authorization for notifications
 */
fileprivate func notificationAuthorizeRequest() {
    let authOptions: UNAuthorizationOptions = [.alert, .sound]
    UNUserNotificationCenter.current().requestAuthorization(
        options: authOptions,
        completionHandler: {
            granted, error in
            #if DEBUG
            print("Notification - Granted: \(granted)")
            print("Notification - Error:", error ?? "<No error>")
            #endif
            
            var reqTracking = true
            if !granted && appNotifSettings.count > 0 {
                if #available(iOS 13.0, *) {
                    reqTracking = false
                    DispatchQueue.main.async {
                        let alert = UIAlertController(title: Bundle.main.displayName, message: DENIED_NOTIFICATIONS_MESSAGE, preferredStyle: .alert)
                        
                        if appNotifSettings.count > 0 {
                            alert.addAction(UIAlertAction(title: "Go To Settings", style: .default) {
                                _ in
                                alert.dismissCallback()
                                openUrl(url: appNotifSettings)
                                requestIdfaOnResume = true
                            })
                        }
                        
                        alert.addAction(UIAlertAction(title: "I Understand", style: .cancel) {
                            _ in
                            alert.dismissCallback()
                            requestIdentifierForAdvertising()
                        })
                        
                        alert.presentInNewWindow(animated: true, completion: nil)
                    }
                }
            }
            
            if reqTracking {
                requestIdentifierForAdvertising()
            }
        }
    )
}
    
/*
 Notification setup
 */
fileprivate func notificationSetup() {
    UIApplication.shared.registerForRemoteNotifications()
}

/*
 Get app info and apply to this game behaviour.
 */
fileprivate func getAppInfo() {
    var body = Utility.getBasicApiPayload()
    body["configVersion"] = UserDefaults.standard.integer(forKey: PREFS_CONFIG_VERSION)
    
    HTTP.POST(Utility.API_BASE + Utility.API_MAIN, parameters: body) { response in
        if let err = response.error {
            print("Error: \(err.localizedDescription)")
            return
        }
        
        do {
            let rawDict = try JSONSerialization.jsonObject(with: response.data, options: [.allowFragments]) as? [String:Any]
            if let jsonData = rawDict {
                // Config version
                let configVersion = jsonData["configVersion"] as! Int
                UserDefaults.standard.set(configVersion, forKey: PREFS_CONFIG_VERSION)
                
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
                
                // Notification settings
                let notifButton = jsonData["notifButton"]
                if notifButton != nil {
                    appNotifSettings = notifButton as! String
                    UserDefaults.standard.set(notifButton, forKey: "appNotifSettings")
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

/*
 Open URL in browser. Show error to the user if the provided URL is not valid.
 */
fileprivate func openUrl(url: String) {
    if url.count <= 0 {
        if #available(iOS 13.0, *) {
            DispatchQueue.main.async {
                let alert = UIAlertController(title: Bundle.main.displayName, message: "Could not open the requested URL. Please, try again later.", preferredStyle: .alert)
                alert.addAction(UIAlertAction(title: "Got It", style: .cancel) {
                    _ in
                    alert.dismissCallback()
                })
                alert.presentInNewWindow(animated: true, completion: nil)
            }
        }
    }
    else if let url = URL(string: url) {
        UIApplication.shared.open(url)
    }
}

/*
 Wrapper callback to execute designated behaviour, based on ad load status, such as
 success, error, special use-cases, etc...
 */
fileprivate func adCallback(type: Int, error: Error?) {
    switch (type)
    {
    case AppAdmobDelegate.TYPE_FAILED_TO_PRESENT:
        print("Fullscreen Ad failed to present")
        cancelAdFlow()
        
    case AppAdmobDelegate.TYPE_PRESENTED:
        print("Fullscreen Ad presented")
        
    case AppAdmobDelegate.TYPE_DISMISSED:
        print("Fullscreen Ad dismissed")
        gpExpire = Int64(Date().timeIntervalSince1970 * 1000)
        interstitialAd = nil
        rewardedAd = nil
        
    default:
        print("No action to take for type \(type)")
    }
}

/*
 Cancel the entire ad flow, for both interstitial and rewarded ones,
 by setting a special value to an intent variable, so the designated
 game object will fetch it and behave accordingly.
 */
fileprivate func cancelAdFlow() {
    gpExpire = 1
    interstitialAd = nil
    rewardedAd = nil
}

/*
 Show generic alert to the user, regarding the availability of the ad functionality.
 */
fileprivate func showAdError() {
    if #available(iOS 13.0, *) {
        DispatchQueue.main.async {
            let alert = UIAlertController(title: Bundle.main.displayName, message: "This functionality is not available at this time. Please, try again later.", preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "Got It", style: .cancel) {
                _ in
                alert.dismissCallback()
            })
            alert.presentInNewWindow(animated: true, completion: nil)
        }
    }
}
