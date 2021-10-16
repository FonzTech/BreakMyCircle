package it.fonztech.breakmycircle;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;

import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;
import java.util.Locale;

import javax.net.ssl.HttpsURLConnection;

public class MainActivity extends EngineActivity implements Runnable, DialogInterface.OnClickListener {
    private static final String TAG = EngineActivity.class.getSimpleName();

    protected AlertDialog mDialog;

    protected static final String URL_DEFAULT_VOTE_ME = "https://play.google.com/store/apps/details?id=it.fonztech.breakmycircle";
    protected static final String URL_DEFAULT_OTHER_APPS = "https://play.google.com/store/apps/developer?id=FonzTech";

    protected Handler handler;
    protected String mStoreUrl;
    protected String mDeveloperUrl;

    protected final Runnable configGetterRunnable = new Runnable() {
        @Override
        public void run() {
            if (Utility.isNetworkAvailable(MainActivity.this)) {
                new Thread(MainActivity.this).start();
            }
            else if (handler != null) {
                handler.postDelayed(configGetterRunnable, 5000L);
            }
        }
    };

    @Override
    protected final void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        {
            final SharedPreferences prefs = getSharedPreferences(CONFIG_PREFERENCES, Context.MODE_PRIVATE);
            mPlayAdThreshold = prefs.getInt("playAdThreshold", 3);
            mStoreUrl = prefs.getString("storeUrl", URL_DEFAULT_VOTE_ME);
            mDeveloperUrl = prefs.getString("developerUrl", URL_DEFAULT_OTHER_APPS);
        }

        handler = new Handler();
        configGetterRunnable.run();
    }

    @Override
    protected final void onDestroy() {
        super.onDestroy();

        if (handler != null) {
            handler.removeCallbacksAndMessages(null);
            handler = null;
        }
    }

    @Override
    public void onBackPressed() {
        if (mDialog != null) {
            mDialog.cancel();
            mDialog = null;
        }
        else {
            mDialog = new AlertDialog.Builder(this)
                    .setTitle(R.string.app_name)
                    .setMessage(R.string.exit_alert)
                    .setPositiveButton(R.string.yes, this)
                    .setNegativeButton(R.string.no, this)
                    .create();
            mDialog.show();
        }
    }

    @Override
    protected final void setRewardedInfo(final String type, final int amount) {
        if ("powerup".equalsIgnoreCase(type)) {
            setPowerupInfo(amount);
        }
        else if (ADS_NOT_AVAILABLE_TYPE.equalsIgnoreCase(type)) {
            setPowerupInfo(0);
        }
        else {
            setPowerupInfo(Utility.DEBUG ? 1 : amount);
        }
    }

    @Override
    protected final String getBackendUrl() {
        return Utility.BACKEND_URL + "api.php";
    }

    @Override
    public final void run() {
        HttpsURLConnection c = null;
        try {
            final URL url = new URL(getBackendUrl());
            c = (HttpsURLConnection) url.openConnection();
            c.setReadTimeout(15000);
            c.setConnectTimeout(30000);
            c.setDoInput(true);
            c.setDoOutput(true);

            {
                final SharedPreferences prefs = getSharedPreferences(CONFIG_PREFERENCES, Context.MODE_PRIVATE);
                final String configVersion = Integer.toString(prefs.getInt("configVersion", 0));

                final StringBuilder sb = new StringBuilder();
                sb.append("configVersion=").append(URLEncoder.encode(configVersion, StandardCharsets.UTF_8.name()));
                sb.append("&type=").append(URLEncoder.encode("android", StandardCharsets.UTF_8.name()));
                sb.append("&locale=").append(URLEncoder.encode(Locale.getDefault().getCountry(), StandardCharsets.UTF_8.name()));
                sb.append("&version=").append(URLEncoder.encode(Integer.toString(getPackageManager().getPackageInfo(getPackageName(), 0).versionCode), StandardCharsets.UTF_8.name()));

                final OutputStream os = c.getOutputStream();
                os.write(sb.toString().getBytes());
                os.close();
            }

            final ByteArrayOutputStream baos = new ByteArrayOutputStream();
            final InputStream is = c.getInputStream();
            final byte[] buf = new byte[1024];
            int len;
            while ((len = is.read(buf)) > 0) {
                baos.write(buf, 0, len);
            }
            is.close();
            baos.close();

            final JSONObject json = new JSONObject(new String(baos.toByteArray()));

            if (Utility.DEBUG) {
                Log.d(TAG, "API response is: " + json);
            }

            mCanShowAds = json.getBoolean("canShowAds");
            mPlayAdThreshold = json.getInt("playAdThreshold");

            if (json.has("storeUrl")) {
                mStoreUrl = json.getString("storeUrl");
            }

            if (json.has("developerUrl")) {
                mDeveloperUrl = json.getString("developerUrl");
            }

            {
                final SharedPreferences prefs = getSharedPreferences(CONFIG_PREFERENCES, Context.MODE_PRIVATE);
                final SharedPreferences.Editor editor = prefs.edit();

                editor.putInt("playAdThreshold", mPlayAdThreshold);
                editor.putString("storeUrl", mStoreUrl);
                editor.putString("developerUrl", mDeveloperUrl);

                editor.commit();
            }

            getIntent().putExtra(GAME_PLAY_AD_THRESHOLD, Integer.toString(mPlayAdThreshold));
        }
        catch (final Exception e) {
            e.printStackTrace();
        }
        finally {
            if (c != null) {
                c.disconnect();
            }
        }
    }

    @Override
    public void onClick(DialogInterface dialogInterface, int i) {
        switch (i) {
            case AlertDialog.BUTTON_POSITIVE:
                super.onBackPressed();
                break;

            case AlertDialog.BUTTON_NEGATIVE:
                onBackPressed();
                break;
        }
    }

    @SuppressWarnings("unused")
    protected final void gameVoteMe() {
        openUrl(mStoreUrl != null ? mStoreUrl : URL_DEFAULT_VOTE_ME);
    }

    @SuppressWarnings("unused")
    protected final void gameOtherApps() {
        openUrl(mDeveloperUrl != null ? mDeveloperUrl : URL_DEFAULT_OTHER_APPS);
    }

    protected final void setPowerupInfo(final int rewardAmount) {
        getIntent().putExtra(GAME_GP_AMOUNT, Integer.toString(rewardAmount));
        getIntent().putExtra(GAME_GP_EXPIRE, Long.toString(System.currentTimeMillis() + 1000L));
    }

    @SuppressWarnings("unused")
    protected final void clearPowerupData() {
        getIntent().removeExtra(GAME_GP_AMOUNT);
        getIntent().removeExtra(GAME_GP_EXPIRE);
    }
}