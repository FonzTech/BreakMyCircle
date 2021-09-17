package it.fonztech.breakmycircle;

import android.util.Log;

import com.google.firebase.messaging.FirebaseMessagingService;
import com.google.firebase.messaging.RemoteMessage;

public class MyFirebaseMessagingService extends FirebaseMessagingService {
    protected static final String TAG = MyFirebaseMessagingService.class.getSimpleName();

    protected String mToken;

    @Override
    public final void onNewToken(@SuppressWarnings("NullableProblems") final String token) {
        Log.d(TAG, "Refreshed token: " + token);
        new TokenSender(token).start();
    }

    @Override
    public final void onMessageReceived(final RemoteMessage remoteMessage) {
        Log.d(TAG, "From: " + remoteMessage.getFrom());
        if (remoteMessage.getData().size() > 0) {
            Log.d(TAG, "Message data payload: " + remoteMessage.getData());
        }

        if (remoteMessage.getNotification() != null) {
            Log.d(TAG, "Message Notification Body: " + remoteMessage.getNotification().getBody());
        }
    }
}