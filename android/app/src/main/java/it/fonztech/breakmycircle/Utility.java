package it.fonztech.breakmycircle;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

public final class Utility {
    public static final boolean DEBUG = true;
    public static final String BACKEND_URL = "https://breakmycircle.alfonsopauciello.com/";
    public static final int REQUEST_OPEN_NOTIFICATION_ACTIVITY = 101;
    public static final int DEFAULT_NOTIFICATION_ID = 0;

    public static final boolean isNetworkAvailable(final Context context) {
        final ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        final NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
        return activeNetworkInfo != null && activeNetworkInfo.isConnected();
    }
}