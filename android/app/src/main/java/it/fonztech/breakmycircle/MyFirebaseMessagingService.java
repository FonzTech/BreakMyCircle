package it.fonztech.breakmycircle;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Build;
import android.util.Log;

import androidx.core.app.NotificationCompat;

import com.google.firebase.messaging.FirebaseMessagingService;
import com.google.firebase.messaging.RemoteMessage;

import java.util.Map;

public class MyFirebaseMessagingService extends FirebaseMessagingService {
    protected static final String TAG = MyFirebaseMessagingService.class.getSimpleName();

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

            final Intent intent = new Intent(this, NotificationActivity.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);

            for (Map.Entry<String, String> entry : remoteMessage.getData().entrySet()) {
                intent.putExtra(entry.getKey(), entry.getValue());
            }

            final PendingIntent pendingIntent = PendingIntent.getActivity(this, Utility.REQUEST_OPEN_NOTIFICATION_ACTIVITY, intent, PendingIntent.FLAG_ONE_SHOT);

            final Uri defaultSoundUri = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);

            final String channelId = "notifications_" + getString(R.string.app_name);
            final NotificationCompat.Builder notificationBuilder = new NotificationCompat.Builder(MyFirebaseMessagingService.this, channelId)
                    .setSmallIcon(R.drawable.ic_notification)
                    .setContentTitle(remoteMessage.getNotification().getTitle())
                    .setContentText(remoteMessage.getNotification().getBody())
                    .setAutoCancel(true)
                    .setSound(defaultSoundUri)
                    .setContentIntent(pendingIntent);

            final NotificationManager notificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                int importance = NotificationManager.IMPORTANCE_HIGH;
                final NotificationChannel mChannel = new NotificationChannel(channelId, getString(R.string.app_name), importance);
                notificationManager.createNotificationChannel(mChannel);
            }

            notificationManager.notify(Utility.DEFAULT_NOTIFICATION_ID, notificationBuilder.build());
        }
    }
}