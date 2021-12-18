package it.fonztech.breakmycircle;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;

import androidx.annotation.NonNull;

import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.net.URL;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;

import javax.net.ssl.HttpsURLConnection;

public class MainActivity extends EngineActivity implements Thread.UncaughtExceptionHandler, Runnable, DialogInterface.OnClickListener {
    private static final String TAG = EngineActivity.class.getSimpleName();

    protected AlertDialog mDialog;

    protected static final String URL_DEFAULT_VOTE_ME = "https://play.google.com/store/apps/details?id=it.fonztech.breakmycircle";
    protected static final String URL_DEFAULT_OTHER_APPS = "https://play.google.com/store/apps/developer?id=FonzTech";

    protected static final String REWARD_TYPE_POWERUP = "powerup";

    protected File lastExceptionFile;
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

        // Set unhandled exception handler
        lastExceptionFile = new File(getFilesDir(), "last_exception.txt");
        Thread.setDefaultUncaughtExceptionHandler(this);

        Log.d(TAG, "Last exception file is: " + lastExceptionFile.getAbsolutePath() + " - Exists: " + lastExceptionFile.exists());

        // Load settings
        {
            final SharedPreferences prefs = getSharedPreferences(CONFIG_PREFERENCES, Context.MODE_PRIVATE);
            mPlayAdThreshold = prefs.getInt("playAdThreshold", 1);
            mStoreUrl = prefs.getString("storeUrl", URL_DEFAULT_VOTE_ME);
            mDeveloperUrl = prefs.getString("developerUrl", URL_DEFAULT_OTHER_APPS);
        }

        // Fetch configuration from API
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
    public final void uncaughtException(@NonNull final Thread thread, @NonNull final Throwable throwable) {
        final ByteArrayOutputStream out = new ByteArrayOutputStream();
        throwable.printStackTrace(new PrintStream(out));
        final String str = new String(out.toByteArray());

        try {
            final FileOutputStream fos = new FileOutputStream(lastExceptionFile, true);
            fos.write(("--- " + System.currentTimeMillis() + "\n\n").getBytes());
            fos.write(str.getBytes());
            fos.write("\n\n".getBytes());
            fos.close();
        }
        catch (final Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    protected final void setRewardedInfo(final String type, final int amount) {
        setRewardedInfoWithTimeout(type, amount, 1000L);
    }

    @Override
    protected final String getBackendUrl() {
        return Utility.BACKEND_URL + Utility.BACKEND_API;
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

                final StringBuilder sb = Utility.getBasicApiPayload(this);
                sb.append("&configVersion=").append(URLEncoder.encode(configVersion, StandardCharsets.UTF_8.name()));

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

                editor.apply();
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

    /**
     * This method is used to open the "Vote Me" intent. As of time of writing, this method is
     * called directly in-game (so on the C++ side), after a tap on a button by the user.
     */
    @SuppressWarnings("unused")
    protected final void gameVoteMe() {
        openUrl(mStoreUrl != null ? mStoreUrl : URL_DEFAULT_VOTE_ME);
    }

    /**
     * This method is used to open the "Other Apps" page in the Google Play Store app. As of time
     * of writing, this method is called directly in-game (so on the C++ side), after a tap on a
     * button by the user.
     */
    @SuppressWarnings("unused")
    protected final void gameOtherApps() {
        openUrl(mDeveloperUrl != null ? mDeveloperUrl : URL_DEFAULT_OTHER_APPS);
    }

    /**
     * Set reward to be passed/fetched by your game code. This method performs some checks regarding
     * the reward type, debug state and other game logic.
     *
     * @param type string representing the reward type (it can be any string, which makes sense in
     *             your game).
     * @param amount how much reward to give to the player (it can be any number).
     * @param timeout timeout after which the reward is no more valid. The timeout system is ONLY
     *                managed by the game logic and nothing more.
     */
    protected final void setRewardedInfoWithTimeout(final String type, final int amount, final long timeout) {
        if (REWARD_TYPE_POWERUP.equalsIgnoreCase(type)) {
            setPowerupInfo(amount, timeout);
        }
        else if (ADS_NOT_AVAILABLE_TYPE.equalsIgnoreCase(type)) {
            setPowerupInfo(0, timeout);
        }
        else {
            setPowerupInfo(Utility.DEBUG ? 1 : amount, timeout);
        }
    }

    /**
     * Set powerup info parameters in this activity's intent. Then, those parameters will be fetched
     * directly by the game logic. So, it only acts as a "data store".
     *
     * @param rewardAmount reward amount for the given reward.
     * @param timeout for how much time the reward is valid.
     */
    protected final void setPowerupInfo(final int rewardAmount, final long timeout) {
        getIntent().putExtra(GAME_GP_AMOUNT, Integer.toString(rewardAmount));
        getIntent().putExtra(GAME_GP_EXPIRE, Long.toString(System.currentTimeMillis() + timeout));
    }

    /**
     * Clear any game reward-related data. As of time of writing, this method is only called by the
     * game logic (C++ side), hence the "unused" warning suppression.
     */
    @SuppressWarnings("unused")
    protected final void clearPowerupData() {
        getIntent().removeExtra(GAME_GP_AMOUNT);
        getIntent().removeExtra(GAME_GP_EXPIRE);
    }
}