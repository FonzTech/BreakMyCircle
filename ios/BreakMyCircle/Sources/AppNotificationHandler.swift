import Foundation
import UserNotifications
import FirebaseMessaging
import SwiftHTTP

class AppNotificationHandler : NSObject, UNUserNotificationCenterDelegate {
    func userNotificationCenter(_ center: UNUserNotificationCenter, willPresent notification: UNNotification, withCompletionHandler completionHandler: @escaping (UNNotificationPresentationOptions) -> Void) {
        completionHandler(.alert)
        
        #if DEBUG
        print("Notification WillPresent:", notification)
        #endif
    }

    func userNotificationCenter(_ center: UNUserNotificationCenter, didReceive response: UNNotificationResponse, withCompletionHandler completionHandler: @escaping () -> Void) {
        completionHandler()
        
        #if DEBUG
        print("Notification DidResponse:", response)
        #endif
    }
}

extension AppNotificationHandler: MessagingDelegate {
    func messaging(_ messaging: Messaging, didReceiveRegistrationToken fcmToken: String?) {
        print("Firebase registration token: \(String(describing: fcmToken))")

        if fcmToken != nil {
            var body = Utility.getBasicApiPayload()
            body["token"] = fcmToken!
            
            HTTP.POST(Utility.API_BASE + Utility.API_FIREBASE, parameters: body) { response in
                if let err = response.error {
                    print("Error: \(err.localizedDescription)")
                    return
                }
                
                do {
                    let rawDict = try JSONSerialization.jsonObject(with: response.data, options: [.allowFragments]) as? [String:Any]
                    if rawDict != nil {
                        print("Response from Token Receiver:", rawDict!)
                    }
                    else {
                        print("Response from Token Receiver is null")
                    }
                } catch let error {
                    print("Error while deserializing JSON: \(error)")
                 }
            }
        }
    }
}
