package it.fonztech.breakmycircle;

import android.content.Context;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Build;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;
import java.util.Locale;

public final class Utility {
    public static final boolean DEBUG = false;
    public static final String BACKEND_URL = "https://breakmycircle.alfonsopauciello.com/";
    public static final String BACKEND_API = "api.php";
    public static final String BACKEND_FIREBASE = "firebase.php";
    public static final int REQUEST_OPEN_NOTIFICATION_ACTIVITY = 101;
    public static final int DEFAULT_NOTIFICATION_ID = 0;

    public static boolean isNetworkAvailable(final Context context) {
        final ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        final NetworkInfo activeNetworkInfo = connectivityManager.getActiveNetworkInfo();
        return activeNetworkInfo != null && activeNetworkInfo.isConnected();
    }

    public static StringBuilder getBasicApiPayload(final Context context) throws UnsupportedEncodingException, PackageManager.NameNotFoundException {
        final StringBuilder sb = new StringBuilder();
        sb.append("type=").append(URLEncoder.encode("android", StandardCharsets.UTF_8.name()));
        sb.append("&system=").append(URLEncoder.encode(Integer.toString(Build.VERSION.SDK_INT), StandardCharsets.UTF_8.name()));
        sb.append("&locale=").append(URLEncoder.encode(Locale.getDefault().getCountry(), StandardCharsets.UTF_8.name()));

        if (context != null) {
            sb.append("&version=").append(URLEncoder.encode(Integer.toString(context.getPackageManager().getPackageInfo(context.getPackageName(), 0).versionCode), StandardCharsets.UTF_8.name()));
        }
        else {
            sb.append("&version=").append(URLEncoder.encode("0", StandardCharsets.UTF_8.name()));
        }

        return sb;
    }
}